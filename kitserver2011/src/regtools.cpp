// regtools.cpp

#define UNICODE

#include <windows.h>
#include <string>
#include "regtools.h"

// This functions looks up the game in the Registry.
// Returns the installation directory for a specified game key
BOOL GetInstallDirFromReg(
    const std::wstring& gameKey, std::wstring& installDir)
{
	HKEY hKey = NULL;
	BOOL res = false;
	
    std::wstring key(L"SOFTWARE\\KONAMI\\");
    key += gameKey;

	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			key.c_str(), 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
	{
        wchar_t installDirBuf[1024];
        SIZE_T bc = sizeof(installDirBuf);
        memset(installDirBuf, 0, sizeof(installDirBuf));
		res = (RegQueryValueEx(
                hKey, L"installdir", NULL, NULL, 
                (LPBYTE)installDirBuf, &bc) == ERROR_SUCCESS);
		RegCloseKey(hKey);
        installDir = installDirBuf;
	}
	return res;
}

