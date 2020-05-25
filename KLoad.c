#include <Windows.h>
#include <winternl.h>
#include <shlwapi.h>
#include <stdio.h>

#pragma comment(lib, "ntdll.lib")

BOOL IsNTAdmin (DWORD reserved, LPDWORD pReserved);

BOOLEAN GetDriverPrivilege()
{
  TOKEN_PRIVILEGES  tokenPrivs;
  HANDLE            hToken;
  LUID              luid;

  if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken))
  {
    if (LookupPrivilegeValueA(NULL, "SeLoadDriverPrivilege", &luid))
    {
      tokenPrivs.PrivilegeCount = 1;
      tokenPrivs.Privileges[0].Luid = luid;
      tokenPrivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
      if (AdjustTokenPrivileges(hToken, FALSE, &tokenPrivs, sizeof(tokenPrivs), NULL, NULL))
      {
        return(TRUE);
      }
    }
  }
  return(FALSE);
}


int main(int argc, void* argv[])
{
  WIN32_FIND_DATAA  findData;
  UNICODE_STRING    usDriverEntry;
  ANSI_STRING       asDriverEntry;
  NTSTATUS          ntRet;
  HANDLE            hFiles;
  HANDLE            hKey;
  LPVOID            lpRegKey;
  LPVOID            lpRegInstance;
  LPVOID            lpFilePath;
  LPVOID            lpLoadPath;
  DWORD             nFilePath;
  DWORD             dwData;

  if (argc > 3 || argc == 1)
  {
    printf("Usage:\tKLoad.exe [Driver Name] [Altitude]\n");
    printf("\tAltitude is optional, but needed if loading a filter driver\n");
    printf("\te.g. KLoad.exe C:\\Windows\\System32\\Drivers\\Random.sys\n");
    printf("\tIf the [Driver Name] is equal to 'unload' then the [Altitude] is the driver name (service key name) to unload\n");
    printf("\te.g. Kload.exe unload Random.sys");
    return(-1);
  }

  if (GetDriverPrivilege())
	{
    if (!strncmp(argv[1], "unload", strlen("unload")))
    {
      if (argc != 3)
      {
        printf("[!] Bad argument for service name of driver to unload\n");
      }
      lpRegKey = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * 2);
      _snprintf_s(lpRegKey, MAX_PATH * 2, MAX_PATH * 2, "\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s", argv[2]);
      RtlInitAnsiString(&asDriverEntry, lpRegKey);
      RtlAnsiStringToUnicodeString(&usDriverEntry, &asDriverEntry, TRUE);
      ntRet = NtUnloadDriver(&usDriverEntry);
      printf("[i] Looked in registry at: %s\n", lpRegKey);
      printf("[i] NtUnloadDriver returned: %X\n", ntRet);
      return(0);
    }
    else 
    {
      hFiles = FindFirstFileA(argv[1], &findData);
      if (hFiles != INVALID_HANDLE_VALUE)
      {
        CloseHandle(hFiles);
        lpRegKey = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * 2);
        _snprintf_s(lpRegKey, MAX_PATH * 2, MAX_PATH * 2, "SYSTEM\\CurrentControlSet\\Services\\%s", findData.cFileName);
        lpFilePath = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * 2);
        CloseHandle(hFiles);
        hFiles = CreateFileA(argv[1], FILE_READ_ACCESS, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, 0);
        GetFinalPathNameByHandleA(hFiles, lpFilePath, MAX_PATH * 2, FILE_NAME_NORMALIZED | VOLUME_NAME_NT);
        CloseHandle(hFiles);
        RegCreateKeyA(HKEY_LOCAL_MACHINE, lpRegKey, &hKey);
        RegSetValueExA(hKey, "ImagePath", 0, REG_SZ, lpFilePath, strlen(lpFilePath));
        dwData = 0x1;
        RegSetValueExA(hKey, "Type", 0, REG_DWORD, &dwData, sizeof(dwData));
        lpLoadPath = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * 2);
        _snprintf_s(lpLoadPath, MAX_PATH * 2, MAX_PATH * 2, "\\REGISTRY\\MACHINE\\SYSTEM\\CurrentControlSet\\Services\\%s", findData.cFileName);
        RtlInitAnsiString(&asDriverEntry, lpLoadPath);
        RtlAnsiStringToUnicodeString(&usDriverEntry, &asDriverEntry, TRUE);
        ntRet = NtLoadDriver(&usDriverEntry);
        if (argc == 3)
        {
          lpRegInstance = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, MAX_PATH * 2);
          _snprintf_s(lpRegInstance, MAX_PATH * 2, MAX_PATH * 2, "SYSTEM\\CurrentControlSet\\Services\\%s\\Instances", findData.cFileName);
          RegCreateKeyA(HKEY_LOCAL_MACHINE, lpRegInstance, &hKey);
          RegSetValueExA(hKey, "DefaultInstance", 0, REG_SZ, "Instance1", strlen("Instance1"));
          RegCreateKeyA(hKey, "Instance1", &hKey);
          RegSetValueExA(hKey, "Altitude", 0, REG_SZ, argv[2], strlen(argv[2]));
          dwData = 0x0;
          RegSetValueExA(hKey, "Flags", 0, REG_DWORD, &dwData, sizeof(dwData));
          printf("[i] Altitude set at: %s\n", argv[2]);
        }
        printf("[i] Registry write occured at: %s\n", lpLoadPath);
        printf("[i] NtLoadDriver returned NTSTATUS: 0x%X\n", ntRet); 
        return(0);
      }
    }
  }
  else 
  {
    printf("[!] This program needs to be run as an administrator\n");
    return(-1);
  }
  return(-1);
}
