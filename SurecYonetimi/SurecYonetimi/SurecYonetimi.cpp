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
	wstring girisMetni = L" 1) �al��an Uygulamalar� Listele\n 2) �al��an Bir Uygulamay� Kapat \n 3) Uygulaman�n CPU ve Memory Kullan�m�n� G�r�nt�le \n 4) Uygulamay� Sonland�r \n L�tfen yapmak istedi�iniz i�lemin numaras�n� giriniz: ";
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
	bool s�rekli = true;
	locale::global(locale("en_US.utf8"));

	while (s�rekli) {
		giris();
		string secilen�slem;
		cin >> secilen�slem;
		HANDLE hProcessSnap;
		PROCESSENTRY32 pe32;
		bool processlerS�raland� = false;

		if (secilen�slem == "1") {
			// �lk se�imde i�lemleri listele
			hProcessSnap = isleminSnapiniAl();
			if (hProcessSnap == INVALID_HANDLE_VALUE) {
				wcout << L"\n ��lemin anl�k g�r�nt�s� al�n�rken bir hata olu�tu." << endl;
				return 1;
			}

			pe32.dwSize = sizeof(PROCESSENTRY32);

			if (!Process32First(hProcessSnap, &pe32)) {
				CloseHandle(hProcessSnap);
				wcout << L"\n �lk i�lem al�n�rken bir hata olu�tu.";
				return 1;
			}

			do {
				wcout << pe32.th32ProcessID << " - " << pe32.szExeFile << endl;
				processlerS�raland� = true;
			} while (Process32Next(hProcessSnap, &pe32));

			if (processlerS�raland�)
			{
				wcout << L"\n ��lemler ba�ar�l� bir �ekilde s�raland�.\n" << endl;
			}

			CloseHandle(hProcessSnap);
		}
		else if (secilen�slem == "2") {
			// �kinci se�imde bir i�lemi kapat
			wstring processAdiIste = L" Durdurmak istedi�iniz i�lemin ad�n� '.exe' uzant�s�yla birlikte yaz�n�z: ";
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
							wcout << L"\n ��lem ba�ar�yla sonland�r�ld�.\n" << endl;
							ProcessDurduruldu = true;
						}
						else {
							wcout << L"\n Kapatma i�lemi sonland�r�lamad�.\n" << endl;
						}
						break; // D�ng�den ��k
					}
				} while (Process32Next(hProcessSnap, &pe32));
				if (!ProcessDurduruldu)
				{
					wcout << L"\n Belirtilen isimde �al��an bir uygulama bulunamad�.\n" << endl;
				}
			}

			if (hProcessSnap != INVALID_HANDLE_VALUE) {
				CloseHandle(hProcessSnap);
			}
		}

		else if (secilen�slem == "3")
		{
			DWORD processAd�;
			PROCESS_MEMORY_COUNTERS_EX pmcex;

			wcout << L"\n Uygulaman�n ID'sini giriniz:  ";
			wcin >> processAd�;

			FILETIME zaman1, zaman2;
			// ilk zaman
			FILETIME oncekiIslemSuresi_Kernel, oncekiSistemSuresi_Kernel;
			FILETIME oncekiIslemSuresi_Kullan�c�, oncekiSistemSuresi_Kullan�c�;
			// sonraki zaman.
			FILETIME islemSuresi_Kernel, sistemSuresi_Kernel;
			FILETIME islemSuresi_Kullan�c�, sistemSuresi_Kullan�c�;
			HANDLE hProcces = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processAd�);

			if (hProcces != NULL) {
				wchar_t yol[MAX_PATH];
				DWORD yolBoyutu = sizeof(yol) / sizeof(wchar_t);

				if (QueryFullProcessImageName(hProcces, 0, yol, &yolBoyutu)) {
					wcout << L"\n Uygulama Ad�: " << PathFindFileName(yol) << endl;
				}
				for (int i = 0; i <= 5; i++) {
					if (GetProcessTimes(hProcces, &zaman1, &zaman2, &islemSuresi_Kernel, &islemSuresi_Kullan�c�) == NULL) {
						wcout << L"��lem zaman�na eri�ilirken sorun olu�tu. ��k�� yap�l�yor...\n";
						break;
					}
					if (GetSystemTimes(0, &sistemSuresi_Kernel, &sistemSuresi_Kullan�c�) == NULL) {
						wcout << L"��lem zaman�na eri�ilirken sorun olu�tu. ��k�� yap�l�yor...\n";
						break;
					}
					ULONGLONG proc = difference(oncekiIslemSuresi_Kernel, oncekiIslemSuresi_Kullan�c�,
						islemSuresi_Kernel, islemSuresi_Kullan�c�);
					ULONGLONG system = difference(oncekiSistemSuresi_Kernel, oncekiSistemSuresi_Kullan�c�,
						sistemSuresi_Kernel, sistemSuresi_Kullan�c�);
					double usage = 0.0;
					if (system != 0) usage = 1000.0 * (proc / (double)system);
					if (i != 0) wcout << L"\n CPU Kullan�m�: " << usage << "%" << endl;

					oncekiIslemSuresi_Kernel = islemSuresi_Kernel;
					oncekiIslemSuresi_Kullan�c� = islemSuresi_Kullan�c�;
					oncekiSistemSuresi_Kernel = sistemSuresi_Kernel;
					oncekiSistemSuresi_Kullan�c� = sistemSuresi_Kullan�c�;

					ZeroMemory(&pmcex, sizeof(PROCESS_MEMORY_COUNTERS_EX));
					if (GetProcessMemoryInfo(hProcces, (PROCESS_MEMORY_COUNTERS*)&pmcex, sizeof(pmcex)) != 0) {
						wcout << L"\n Toplam Bellek Kullan�m�: " << (double)pmcex.WorkingSetSize / (1024 * 1024) << " MB" << endl;
					}

					Sleep(1000);
				}
				CloseHandle(hProcces);
			}
			else {
				wcout << L"\n ��lem A��l�m� Ba�ar�s�z Oldu\n" << endl;
			}
		}

		else if (secilen�slem == "4") {
			s�rekli = false;
		}




	}
	return 0;
}