// krun.cpp
// Copyright(C) 2010, Juce, Robbie.
//
// This application is used to launch the game
// in such a way that Kitserver loader DLL (kload)
// get loaded into memory before the main program
// starts executing.
//
// (This is similar to what manager(setup) programs
// were doing in the past, except this way you do 
// not need to modify the game EXE at all.

#define UNICODE

#include <windows.h>
#include <string>
#include "kload_exp.h"
#include "utf8.h"
#include "imageutil.h"
#include "regtools.h"


std::wstring _gameExe;
std::string _kloadDll("kload");

const char _start_proc_template[] = 
    "\xbe\0\0\0\0"        // mov esi, _savedCode
    "\xbf\0\0\0\0"        // mov edi, _entryPoint
    "\xb9\x05\0\0\0"      // mov ecx, 5
    "\xf3\xa4"            // rep mosvb
    "\x68\0\0\0\0"        // push _dllName
    "\xff\x15\0\0\0\0"    // call dword [_loadLibraryThunkVA]
    "\xe9\0\0\0\0";       // jmp _entryPoint

#define SAVED_CODE 1
#define ENTRY_POINT_1 6
#define DLL_NAME 18
#define LOAD_LIBRARY_THUNK 24
#define ENTRY_POINT_2 29

BYTE _start_proc[sizeof(_start_proc_template)];

struct bootstrap_data_t
{
    char kloadDll[16];
    BYTE savedCode[8];
};

static void string_strip(wstring& s)
{
    static const wchar_t* empties = L" \t\n\r";
    int e = s.find_last_not_of(empties);
    s.erase(e + 1);
    int b = s.find_first_not_of(empties);
    s.erase(0,b);
}

static void string_strip_quotes(wstring& s)
{
    if (s.empty())
        return;
    if (s[s.length()-1]=='"')
        s.erase(s.length()-1);
    if (s[0]=='"')
        s.erase(0,1);
}

size_t ExecuteProcess(
        std::wstring FullPathToExe, 
        std::wstring Parameters, 
        size_t SecondsToWait) 
{ 
    size_t iMyCounter = 0, iReturnVal = 0, iPos = 0; 
    DWORD dwExitCode = 0; 
    std::wstring sTempStr = L""; 

    /* Add a space to the beginning of the Parameters */ 
    if (Parameters.size() != 0) 
    { 
        if (Parameters[0] != L' ') 
        { 
            Parameters.insert(0,L" "); 
        } 
    } 

    /* The first parameter needs to be the exe itself */ 
    sTempStr = FullPathToExe; 
    iPos = sTempStr.find_last_of(L"\\"); 
    sTempStr.erase(0, iPos +1); 
    Parameters = sTempStr.append(Parameters); 

     /* CreateProcessW can modify Parameters thus we allocate needed memory */ 
    wchar_t * pwszParam = new wchar_t[Parameters.size() + 1]; 
    if (pwszParam == 0) 
    { 
        return 1; 
    } 
    const wchar_t* pchrTemp = Parameters.c_str(); 
    wcscpy_s(pwszParam, Parameters.size() + 1, pchrTemp); 

    /* CreateProcess API initialization */ 
    STARTUPINFOW siStartupInfo; 
    PROCESS_INFORMATION piProcessInfo; 
    memset(&siStartupInfo, 0, sizeof(siStartupInfo)); 
    memset(&piProcessInfo, 0, sizeof(piProcessInfo)); 
    siStartupInfo.cb = sizeof(siStartupInfo); 

    /* need to read certain information from the exe.
     * We count on the fact that LoadLibraryA is imported
     * by the game exe, and therefore we can use its
     * import thunk to call it from the new process. */

    FILE *exef = _wfopen(FullPathToExe.c_str(), L"rb");
    if (!exef) {
        wprintf(L"Unable to open exe file for reading.\n");
        return 0;
    }

    DWORD imageBase = 0;
    if (SeekImageBase(exef)) {
        fread(&imageBase, sizeof(DWORD), 1, exef);
        wprintf(L"imageBase = %p\n", imageBase);
    }

    DWORD entryPointRVA = 0;
    if (SeekEntryPoint(exef)) {
        fread(&entryPointRVA, sizeof(DWORD), 1, exef);
        wprintf(L"entryPointRVA = %p\n", entryPointRVA);
    }

    DWORD loadLibraryRVA = getImportThunkRVA(
        exef, "kernel32.dll", "LoadLibraryA");
    wprintf(L"loadLibraryRVA = %p\n", loadLibraryRVA);

    fclose(exef);

    /* now read to create new process */
    if (CreateProcessW(
            const_cast<LPCWSTR>(FullPathToExe.c_str()), 
            pwszParam, 0, 0, false, 
            CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED, 0, 0, 
            &siStartupInfo, &piProcessInfo) != false) 
    { 
        wprintf(L"hProcess: %p\n", piProcessInfo.hProcess);

        /* Allocate memory in the new process address space
         * for the bootstrap_data block */
        BYTE* data = (BYTE*)VirtualAllocEx(
                piProcessInfo.hProcess, NULL,
                sizeof(struct bootstrap_data_t), MEM_RESERVE | MEM_COMMIT,
                PAGE_READWRITE);
        if (!data) {
            wprintf(L"Unable to allocate memory for bootstrap data\n");
            return 0;
        }
        else {
            wprintf(L"Allocated at %p\n", data);
        }

        DWORD entryPointVA = imageBase + entryPointRVA;
        DWORD loadLibraryThunk = imageBase + loadLibraryRVA;

        SIZE_T bytesRead = 0;
        SIZE_T bytesWritten = 0;

        struct bootstrap_data_t localData;
        strcpy(localData.kloadDll, _kloadDll.c_str());
        if (!ReadProcessMemory(
                piProcessInfo.hProcess,
                (BYTE*)entryPointVA, &localData.savedCode, 5, &bytesRead)) {
            wprintf(L"Unable to read process data\n");
            return 0;
        }

        /* Copy the bootstrap data into the new process address space */
        if (!WriteProcessMemory(
                piProcessInfo.hProcess,
                data, &localData, sizeof(localData), &bytesWritten)) {
            wprintf(L"Unable to write process memory.\n");
            return 0;
        }
        else {
            wprintf(L"Wrote %d bytes to the new process memory\n",
                bytesWritten);
        }

        /* Allocate memory in the the new process address space */
        SIZE_T start_proc_size = sizeof(_start_proc);
        BYTE* mem = (BYTE*)VirtualAllocEx(
                piProcessInfo.hProcess, NULL,
                start_proc_size, MEM_RESERVE | MEM_COMMIT, 
                PAGE_EXECUTE_READWRITE);
        if (!mem) {
            wprintf(L"Unable to allocate memory in the new "
                    L"process address space\n");
            return 0;
        }
        else {
            wprintf(L"Allocated space at %p\n", mem);
        }

        /* fill in addresses for start_proc code */
        memcpy(_start_proc, _start_proc_template, sizeof(_start_proc));

        struct bootstrap_data_t* pRemoteData = 
            (struct bootstrap_data_t*)data;
        DWORD jmpOffset = 
            entryPointVA - ((DWORD)mem + ENTRY_POINT_2 + 4);
        *(DWORD*)(_start_proc + SAVED_CODE) = (DWORD)&pRemoteData->savedCode;
        *(DWORD*)(_start_proc + ENTRY_POINT_1) = entryPointVA;
        *(DWORD*)(_start_proc + DLL_NAME) = (DWORD)&pRemoteData->kloadDll;
        *(DWORD*)(_start_proc + LOAD_LIBRARY_THUNK) = loadLibraryThunk;
        *(DWORD*)(_start_proc + ENTRY_POINT_2) = jmpOffset;

        /* Copy the start_proc function into the address space
         * of the new process */
        if (!WriteProcessMemory(
                piProcessInfo.hProcess,
                mem, _start_proc, start_proc_size, &bytesWritten)) {
            wprintf(L"Unable to write process memory.\n");
            return 0;
        }
        else {
            wprintf(L"Wrote %d bytes to the new process memory\n",
                bytesWritten);
        }

        /* replace code at entry point with a jmp to our code */
        DWORD oldProtection = 0;
        DWORD newProtection = PAGE_EXECUTE_READWRITE;
        if (!VirtualProtectEx(
                piProcessInfo.hProcess,
                (BYTE*)entryPointVA, 8, newProtection, &oldProtection)) {
            wprintf(L"VirtualProtectEx failed.\n");
            return 0;
        }
        else {
            BYTE jmpCmd[5];
            jmpCmd[0] = '\xe9';
            *(DWORD*)(jmpCmd + 1) = (DWORD)mem - (entryPointVA + 5);

            if (!WriteProcessMemory(
                    piProcessInfo.hProcess,
                    (BYTE*)entryPointVA, jmpCmd, 
                    sizeof(jmpCmd), &bytesWritten)) {
                wprintf(L"Unable to write jmp-command.\n");
                return 0;
            }
            else {
                wprintf(L"Wrote jump-command.\n");
            }
        }

        /* Resume the main thread of the process to
         * launch our injected code */
        ResumeThread(piProcessInfo.hThread);

        /* Watch the process. */ 
        dwExitCode = WaitForSingleObject(
                piProcessInfo.hProcess, (SecondsToWait * 1000)); 
    } 
    else 
    { 
        /* CreateProcess failed */ 
        iReturnVal = GetLastError(); 
    } 

    /* Free memory */ 
    delete[]pwszParam; 
    pwszParam = 0; 

    /* Release handles */ 
    CloseHandle(piProcessInfo.hProcess); 
    CloseHandle(piProcessInfo.hThread); 

    return iReturnVal; 
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    wchar_t* exeName = Utf8::utf8ToUnicode((BYTE*)lpCmdLine);
    _gameExe += exeName;
    Utf8::free(exeName);

    if (_gameExe.empty()) {
        // exe not specified in command-line: try to read
        // from a configuration file then.
        wchar_t name[8192];
        GetModuleFileName(NULL, name, sizeof(name)/sizeof(wchar_t));
        std::wstring myname(name);
        std::wstring myshortname(myname.substr(myname.rfind(L"\\")+1));
        std::wstring myconfig(
            myshortname.substr(0, myshortname.rfind(L".")));
        myconfig += L"-config.txt";

        HANDLE handle = CreateFile(
               myconfig.c_str(), 
               GENERIC_READ, FILE_SHARE_READ, NULL,
               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
               NULL);
        if (handle != INVALID_HANDLE_VALUE)
        {
            SIZE_T size = GetFileSize(handle,NULL);
            SIZE_T read = 0;
            BYTE* buf = new BYTE[size+1];
            memset(buf, 0, size+1);
            ReadFile(handle, buf, size, &read, 0);
            exeName = Utf8::utf8ToUnicode(buf);
            _gameExe += exeName;
            Utf8::free(exeName);
            delete buf;
            CloseHandle(handle);
        }
    }

    string_strip(_gameExe);
    string_strip_quotes(_gameExe);

    if (_gameExe.empty()) {
        // one last attempt: try to read it from the registry
        wstring installDir;
        if (GetInstallDirFromReg(L"PES2011", installDir)) {
            _gameExe = installDir;
            _gameExe += L"\\PES2011.exe";
        }
    }

    wprintf(L"Launching the exe: {%s}...\n", _gameExe.c_str());
    ExecuteProcess(_gameExe, L"", 0);
    wprintf(L"Launcher is finished.\n");
    return 0;
}

