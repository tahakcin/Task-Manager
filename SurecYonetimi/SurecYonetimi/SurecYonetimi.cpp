// TaskManager.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _WIN32_DCOM
#include <comdef.h>
#include <WbemIdl.h>
#pragma comment(lib, "wbemuuid.lib")
#include <windows.h>
#include <tlhelp32.h>
#include <iostream> 
#include <locale>
#include <iomanip>
#include <processthreadsapi.h>
#include <string>
#include <psapi.h>
#include <signal.h>
#include <Shlwapi.h>


using namespace std;

HANDLE isleminSnapiniAl() {
	HANDLE Snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	return Snap;
}

void giris() {
	wstring girisMetni = L" 1) Çalýþan Uygulamalarý Listele\n 2) Çalýþan Bir Uygulamayý Kapat \n 3) Uygulamanýn CPU ve Memory Kullanýmýný Görüntüle \n 4) Uygulamayý Sonlandýr \n Lütfen yapmak istediðiniz iþlemin numarasýný giriniz: ";
	wcout << girisMetni;
}

ULONGLONG difference(FILETIME& prev_kernel, FILETIME& prev_user, FILETIME& cur_kernel, FILETIME& cur_user) {
	LARGE_INTEGER a1, a2;
	a1.LowPart = prev_kernel.dwLowDateTime;
	a1.HighPart = prev_kernel.dwHighDateTime;
	a2.LowPart = prev_user.dwLowDateTime;
	a2.HighPart = prev_user.dwHighDateTime;

	LARGE_INTEGER b1, b2;
	b1.LowPart = cur_kernel.dwLowDateTime;
	b1.HighPart = cur_kernel.dwHighDateTime;
	b2.LowPart = cur_user.dwLowDateTime;
	b2.HighPart = cur_user.dwHighDateTime;

	//a1 and b1 - contains kernel times
	//a2 and b2 - contains user times
	return (b1.QuadPart - a1.QuadPart) + (b2.QuadPart - a2.QuadPart);
}

int main() {
	bool sürekli = true;
	locale::global(locale("en_US.utf8"));

	while (sürekli) {
		giris();
		string secilenÝslem;
		cin >> secilenÝslem;
		HANDLE hProcessSnap;
		PROCESSENTRY32 pe32;
		bool processlerSýralandý = false;

		if (secilenÝslem == "1") {
			// Ýlk seçimde iþlemleri listele
			hProcessSnap = isleminSnapiniAl();
			if (hProcessSnap == INVALID_HANDLE_VALUE) {
				wcout << L"\n Ýþlemin anlýk görüntüsü alýnýrken bir hata oluþtu." << endl;
				return 1;
			}

			pe32.dwSize = sizeof(PROCESSENTRY32);

			if (!Process32First(hProcessSnap, &pe32)) {
				CloseHandle(hProcessSnap);
				wcout << L"\n Ýlk iþlem alýnýrken bir hata oluþtu.";
				return 1;
			}

			do {
				wcout << pe32.th32ProcessID << " - " << pe32.szExeFile << endl;
				processlerSýralandý = true;
			} while (Process32Next(hProcessSnap, &pe32));

			if (processlerSýralandý)
			{
				wcout << L"\n Ýþlemler baþarýlý bir þekilde sýralandý.\n" << endl;
			}

			CloseHandle(hProcessSnap);
		}
		else if (secilenÝslem == "2") {
			// Ýkinci seçimde bir iþlemi kapat
			wstring processAdiIste = L" Durdurmak istediðiniz iþlemin adýný '.exe' uzantýsýyla birlikte yazýnýz: ";
			wcout << processAdiIste;
			wstring stopProcessName;
			wcin >> stopProcessName;

			hProcessSnap = isleminSnapiniAl();
			PROCESSENTRY32 pe32;
			bool ProcessDurduruldu = false;
			pe32.dwSize = sizeof(PROCESSENTRY32);

			if (Process32First(hProcessSnap, &pe32)) {
				do {
					if (wcscmp(pe32.szExeFile, stopProcessName.c_str()) == 0) {
						HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pe32.th32ProcessID);
						if (hProcess != NULL) {
							TerminateProcess(hProcess, 0);
							CloseHandle(hProcess);
							wcout << L"\n Ýþlem baþarýyla sonlandýrýldý.\n" << endl;
							ProcessDurduruldu = true;
						}
						else {
							wcout << L"\n Kapatma iþlemi sonlandýrýlamadý.\n" << endl;
						}
						break; // Döngüden çýk
					}
				} while (Process32Next(hProcessSnap, &pe32));
				if (!ProcessDurduruldu)
				{
					wcout << L"\n Belirtilen isimde çalýþan bir uygulama bulunamadý.\n" << endl;
				}
			}

			if (hProcessSnap != INVALID_HANDLE_VALUE) {
				CloseHandle(hProcessSnap);
			}
		}

		else if (secilenÝslem == "3")
		{
			DWORD processAdý;
			PROCESS_MEMORY_COUNTERS_EX pmcex;

			wcout << L"\n Uygulamanýn ID'sini giriniz:  ";
			wcin >> processAdý;

			FILETIME zaman1, zaman2;
			// ilk zaman
			FILETIME oncekiIslemSuresi_Kernel, oncekiSistemSuresi_Kernel;
			FILETIME oncekiIslemSuresi_Kullanýcý, oncekiSistemSuresi_Kullanýcý;
			// sonraki zaman.
			FILETIME islemSuresi_Kernel, sistemSuresi_Kernel;
			FILETIME islemSuresi_Kullanýcý, sistemSuresi_Kullanýcý;
			HANDLE hProcces = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processAdý);

			if (hProcces != NULL) {
				wchar_t yol[MAX_PATH];
				DWORD yolBoyutu = sizeof(yol) / sizeof(wchar_t);

				if (QueryFullProcessImageName(hProcces, 0, yol, &yolBoyutu)) {
					wcout << L"\n Uygulama Adý: " << PathFindFileName(yol) << endl;
				}
				for (int i = 0; i <= 5; i++) {
					if (GetProcessTimes(hProcces, &zaman1, &zaman2, &islemSuresi_Kernel, &islemSuresi_Kullanýcý) == NULL) {
						wcout << L"Ýþlem zamanýna eriþilirken sorun oluþtu. Çýkýþ yapýlýyor...\n";
						break;
					}
					if (GetSystemTimes(0, &sistemSuresi_Kernel, &sistemSuresi_Kullanýcý) == NULL) {
						wcout << L"Ýþlem zamanýna eriþilirken sorun oluþtu. Çýkýþ yapýlýyor...\n";
						break;
					}
					ULONGLONG proc = difference(oncekiIslemSuresi_Kernel, oncekiIslemSuresi_Kullanýcý,
						islemSuresi_Kernel, islemSuresi_Kullanýcý);
					ULONGLONG system = difference(oncekiSistemSuresi_Kernel, oncekiSistemSuresi_Kullanýcý,
						sistemSuresi_Kernel, sistemSuresi_Kullanýcý);
					double usage = 0.0;
					if (system != 0) usage = 1000.0 * (proc / (double)system);
					if (i != 0) wcout << L"\n CPU Kullanýmý: " << usage << "%" << endl;

					oncekiIslemSuresi_Kernel = islemSuresi_Kernel;
					oncekiIslemSuresi_Kullanýcý = islemSuresi_Kullanýcý;
					oncekiSistemSuresi_Kernel = sistemSuresi_Kernel;
					oncekiSistemSuresi_Kullanýcý = sistemSuresi_Kullanýcý;

					ZeroMemory(&pmcex, sizeof(PROCESS_MEMORY_COUNTERS_EX));
					if (GetProcessMemoryInfo(hProcces, (PROCESS_MEMORY_COUNTERS*)&pmcex, sizeof(pmcex)) != 0) {
						wcout << L"\n Toplam Bellek Kullanýmý: " << (double)pmcex.WorkingSetSize / (1024 * 1024) << " MB" << endl;
					}

					Sleep(1000);
				}
				CloseHandle(hProcces);
			}
			else {
				wcout << L"\n Ýþlem Açýlýmý Baþarýsýz Oldu\n" << endl;
			}
		}

		else if (secilenÝslem == "4") {
			sürekli = false;
		}




	}
	return 0;
}