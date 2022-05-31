#ifndef RAM_LISTENER_H
#define RAM_LISTENER_H

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>

class RamListener {
	HANDLE hProc;
	DWORD pID;

public:
	bool attachProc(const char* procName);
	template<typename T> void wpm(T value, DWORD addressToWrite);
	template<typename T> T rpm(DWORD addressToRead);
};

bool RamListener::attachProc(const char* procName)
{
	PROCESSENTRY32 procEntry32;

	// Definig the size so we can populate it
	procEntry32.dwSize = sizeof(PROCESSENTRY32);

	// Taking a snaphot of all processes running
	auto hProcSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcSnap == INVALID_HANDLE_VALUE) {
		std::cout << "Failed to take snapshot of processes!" << std::endl;
		return false;
	}

	while (Process32Next(hProcSnap, &procEntry32)) {
		std::cout << procEntry32.szExeFile << std::endl;

		// If the process we're looking ..
		if (!strcmp(procName, procEntry32.szExeFile))
		{
			std::cout << "Found process: " << procEntry32.szExeFile << " with process ID " << procEntry32.th32ProcessID << std::endl;
			hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procEntry32.th32ProcessID);
			pID = procEntry32.th32ProcessID;

			if (hProc == NULL)
			{
				std::cout << "Failed getting handle to process." << std::endl;
			}

			CloseHandle(hProcSnap);
			return true;
		}
	}
	std::cout << "Couldn't find " << procName << " in the process snapshot" << std::endl;
	CloseHandle(hProcSnap);
	return false;
}

template <typename T> void RamListener::wpm(T value, DWORD addressToWrite)
{
	WriteProcessMemory(hProc, (PVOID)addressToWrite, &value, sizeof(T), 0);
}

template<typename T> T RamListener::rpm(DWORD addressToRead)
{
	T rpmBuffer;
	ReadProcessMemory(hProc, (PVOID)addressToRead, &rpmBuffer, sizeof(T), 0);
	return rpmBuffer;
}
#endif
