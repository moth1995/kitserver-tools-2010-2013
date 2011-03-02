// krun.cpp
// Copyright(C) 2011, Juce, Robbie.

#define UNICODE

#include <windows.h>
#include <string>
#include "kload_exp.h"
#include "utf8.h"
#include "imageutil.h"
#include "regtools.h"
#include "configs.h"


std::wstring _gameExe;
const int SECS_TO_WAIT = 2;

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

class file_manager_t
{
public:
    file_manager_t(FILE* f) : _file(f) {}
    ~file_manager_t() 
    {
        if (_file) {
            fclose(_file);
        }
    }

    FILE* _file;
};

void fmLog(file_manager_t& fm, const wchar_t *format, ...);

bool AttachKload(wstring& filename, file_manager_t& log)
{
    HMODULE krnl = GetModuleHandle(L"kernel32.dll");
    DWORD loadLib = (DWORD)GetProcAddress(krnl, "LoadLibraryA");

    DWORD ep, ib;
    DWORD dataOffset, dataVA;
    DWORD codeOffset, codeVA;
    DWORD loadLibAddr, kloadAddr, loadLibAddr1;
    DWORD newEntryPoint;

    file_manager_t fm(_wfopen(filename.c_str(), L"r+b"));
    if (!fm._file) {
        fmLog(log, L"ERROR: unable to modify %s\n", filename.c_str());
        return false;
    }

    loadLibAddr1 = getImportThunkRVA(
        fm._file, "kernel32.dll","LoadLibraryA");
    
    // this used to cause the latest crashes on game start
    if (!loadLibAddr1) {
        fmLog(log, L"ERROR: cannot find LoadLibraryA thunk\n");
        return false;
    }

    if (!SeekEntryPoint(fm._file)) {
        fmLog(log, L"ERROR: unable to localte entry point\n");
        return false;
    }

    fread(&ep, sizeof(DWORD), 1, fm._file);
    fmLog(log, L"Entry point: %08x\n", ep);

    if (!SeekImageBase(fm._file)) {
        fmLog(log, L"ERROR: unable to find image base\n");
        return false;
    }

    fread(&ib, sizeof(DWORD), 1, fm._file);
    fmLog(log, L"Image base: %08x\n", ib);

    IMAGE_SECTION_HEADER dataHeader;
    ZeroMemory(&dataHeader, sizeof(IMAGE_SECTION_HEADER));
    dataOffset = 0;
    dataVA = 0;

    // find empty space at the end of .rdata
    bool rdataFound = SeekSectionHeader(fm._file, ".rdata");
    if (!rdataFound) rdataFound = SeekSectionHeader(fm._file, ".rsrc");
    if (!rdataFound) rdataFound = SeekSectionHeader(fm._file, ".IN");
    if (rdataFound) {
        fread(&dataHeader, sizeof(IMAGE_SECTION_HEADER), 1, fm._file);

        //adjust SizeOfRawData (needed for WE9LEK-nocd)
        int rem = dataHeader.SizeOfRawData % 0x80;
        if (rem > 0) {
            dataHeader.SizeOfRawData += 0x80 - rem;
            fseek(fm._file, -sizeof(IMAGE_SECTION_HEADER), SEEK_CUR);
            fwrite(&dataHeader, sizeof(IMAGE_SECTION_HEADER), 1, fm._file);
        }

        dataVA = dataHeader.VirtualAddress + dataHeader.SizeOfRawData;
        if (dataHeader.PointerToRawData != 0) {
            dataOffset = dataHeader.PointerToRawData 
                + dataHeader.SizeOfRawData;
        }
        else {
            dataOffset = dataHeader.VirtualAddress 
                + dataHeader.SizeOfRawData;
        }

        // shift 32 bytes back
        dataOffset -= 0x20;
        dataVA -= 0x20;
    }
    else {
        fmLog(log, L"ERROR: unable to find space in .rdata\n");
        return false;
    }

    fmLog(log, L"dataOffset = %08x\n", dataOffset);
    fmLog(log, L"dataVA = %08x\n", dataVA);

    // at the found empty place, write the LoadLibrary address, 
    // and the name of kload.dll
    BYTE buf[0x20], zero[0x20];
    ZeroMemory(zero, 0x20);
    ZeroMemory(buf, 0x20);

    fseek(fm._file, dataOffset, SEEK_SET);
    fread(&buf, 0x20, 1, fm._file);
    if (memcmp(buf, zero, 0x20)==0 ||
            strncmp((const char*)buf+8, "kitserver\\kload", 0x20)==0)
    {
        // ok, we found an empty place. Let's live here.
        // (or preattached kitserver - that's ok too)
        fseek(fm._file, -0x20, SEEK_CUR);
        DWORD* p = (DWORD*)buf;
        p[0] = ep; // save old entry pointer for easy uninstall
        p[1] = loadLib;
        memcpy(buf + 8, "kload", strlen("kload")+1);
        fwrite(buf, 0x20, 1, fm._file);
        
        loadLibAddr = ib + dataVA + sizeof(DWORD);
        fmLog(log, L"loadLibAddr = %08x\n", loadLibAddr);
        kloadAddr = loadLibAddr + sizeof(DWORD);
        fmLog(log, L"kloadAddr = %08x\n", kloadAddr);
        
        loadLibAddr = ib + loadLibAddr1;
    }
    else
    {
        fmLog(log, L"ERROR: space already taken by other data.\n");
        return false;
    }

    IMAGE_SECTION_HEADER textHeader;
    ZeroMemory(&textHeader, sizeof(IMAGE_SECTION_HEADER));
    codeOffset = 0;
    codeVA = 0;

    // find empty space at the end of .text
    bool textFound = SeekSectionHeader(fm._file, ".text");
    if (!textFound) textFound = SeekSectionHeader(fm._file,"rr02");
    if (!textFound) textFound = SeekSectionHeader(fm._file,".HEAVEN");
    if (!textFound) textFound = SeekSectionHeader(fm._file,"");
    if (textFound) {
        fread(&textHeader, sizeof(IMAGE_SECTION_HEADER), 1, fm._file);

        //adjust SizeOfRawData (needed for WE9LEK-nocd)
        int rem = textHeader.SizeOfRawData % 0x40;
        if (rem > 0) {
            textHeader.SizeOfRawData += 0x40 - rem;
            fseek(fm._file, -sizeof(IMAGE_SECTION_HEADER), SEEK_CUR);
            fwrite(&textHeader, sizeof(IMAGE_SECTION_HEADER), 1, fm._file);
        }

        codeVA = textHeader.VirtualAddress + textHeader.SizeOfRawData;
        if (textHeader.PointerToRawData != 0) {
            codeOffset = textHeader.PointerToRawData 
                + textHeader.SizeOfRawData;
        }
        else {
            codeOffset = textHeader.VirtualAddress 
                + textHeader.SizeOfRawData;
        }

        // shift 32 bytes back.
        codeOffset -= 0x20;
        codeVA -= 0x20;
    } 
    else {
        fmLog(log, L"section header for '.text' not found.\n");
        return false;
    }

    fmLog(log, L"codeOffset = %08x\n", codeOffset);
    fmLog(log, L"codeVA = %08x\n", codeVA);

    // at the found place, write the new entry point logic
    ZeroMemory(zero, 0x20);
    ZeroMemory(buf, 0x20);

    fseek(fm._file, codeOffset, SEEK_SET);
    fread(&buf, 0x20, 1, fm._file);
    if (memcmp(buf, zero, 0x20)==0)
    {
        // ok, we found an empty place. Let's live here.
        // (or new entry point logic already in place, which is ok too)
        fseek(fm._file, -0x20, SEEK_CUR);
        buf[0] = 0x68;  // push
        DWORD* p = (DWORD*)(buf + 1); p[0] = kloadAddr;
        buf[5] = 0xff; buf[6] = 0x15; // call
        p = (DWORD*)(buf + 7); p[0] = loadLibAddr;
        buf[11] = 0xe9; // jmp
        p = (DWORD*)(buf + 12); p[0] = ib + ep - 5 - (ib + codeVA + 11);
        fwrite(buf, 0x20, 1, fm._file);

        newEntryPoint = codeVA;
    }
    else if (buf[0]==0x68 && buf[5]==0xff && buf[11]==0xe9) {
        fmLog(log, L"WARN: looks like new entry logic already in place\n");
        return true;
    }
    else {
        fmLog(log, L"ERROR: space for new entry logic already "
                L"taken by other data.\n");
        return false;
    }

    if (!SeekEntryPoint(fm._file)) {
        fmLog(log, L"ERROR: unable to find entry point\n");
        return false;
    }

    // write new entry point
    fwrite(&newEntryPoint, sizeof(DWORD), 1, fm._file);
    return true;
}

// Universal logger
void fmLog(file_manager_t& fm, const wchar_t *format, ...)
{
    if (!fm._file)
        return;

    va_list params;
    va_start(params, format);
    vfwprintf(fm._file, format, params);
    va_end(params);
    fflush(fm._file);
}

int ExecuteProcess(wstring& filename, file_manager_t& log)
{
    int start = filename.rfind(L"\\") + 1;
    int end = filename.rfind(L".");
    wstring dest(L".\\tmp\\");
    dest += filename.substr(start, end-start);
    dest += L".tmp";

    if (!CopyFile(filename.c_str(), dest.c_str(), FALSE)) {
        fmLog(log, L"ERROR: problem with accessing EXE: check your path\n");
        return 1;
    }
    if (!AttachKload(dest, log)) {
        return 1;
    }

    size_t iMyCounter = 0, iReturnVal = 0, iPos = 0; 
    DWORD dwExitCode = 0; 
    std::wstring sTempStr = L""; 
    wstring Parameters(L" ");

    /* Add a space to the beginning of the Parameters */ 
    if (Parameters.size() != 0) 
    { 
        if (Parameters[0] != L' ') 
        { 
            Parameters.insert(0,L" "); 
        } 
    } 

    /* The first parameter needs to be the exe itself */ 
    sTempStr = dest; 
    iPos = sTempStr.find_last_of(L"\\"); 
    sTempStr.erase(0, iPos +1); 
    Parameters = sTempStr;

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

    if (CreateProcessW(
            const_cast<LPCWSTR>(dest.c_str()), 
            pwszParam, 0, 0, false, 
            CREATE_DEFAULT_ERROR_MODE, 0, 0, 
            &siStartupInfo, &piProcessInfo) != false) 
    { 
        fmLog(log, L"Launched successfully\n");

        /* Watch the process. */ 
        dwExitCode = WaitForSingleObject(
                piProcessInfo.hProcess, (SECS_TO_WAIT * 1000)); 
        //DeleteFile(dest.c_str());
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

void krunConfig(char* pName, const void* pValue, DWORD a)
{
	switch (a) {
		case 1:	//exe
            _gameExe = (wchar_t*)pValue;
			break;
    }
}

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
    wchar_t* exeName = Utf8::utf8ToUnicode((BYTE*)lpCmdLine);
    _gameExe += exeName;
    Utf8::free(exeName);

    wchar_t name[4096];
    GetModuleFileName(NULL, name, sizeof(name)/sizeof(wchar_t));
    std::wstring myname(name);
    int start = myname.rfind(L"\\") + 1;
    int end = myname.rfind(L".");
    std::wstring myshortname(myname.substr(start, end-start));

    if (_gameExe.empty()) {
        // exe not specified in command-line: try to read
        // from a configuration file then.
        std::wstring myconfig(myshortname);
        myconfig += L"-config.txt";

        if (readConfig(myconfig.c_str())) {
            _getConfig("krun", "exe", DT_STRING, 1, krunConfig);
        }
    }

    string_strip(_gameExe);
    string_strip_quotes(_gameExe);

    if (_gameExe.empty()) {
        // one last attempt: try to read it from the registry
        wstring installDir;
        if (GetInstallDirFromReg(L"PES2011", installDir)) {
            _gameExe = installDir;
            if (!_gameExe.empty() && _gameExe[_gameExe.size()-1] != L'\\') {
                _gameExe += L"\\";
            }
            _gameExe += L"PES2011.exe";
        }
    }

    wstring logname(myshortname);
    logname += L".log";
    file_manager_t log(_wfopen(logname.c_str(), L"wt"));

    fmLog(log, L"EXE: {%s}\n", _gameExe.c_str());
    ExecuteProcess(_gameExe, log);
    fmLog(log, L"Launcher is finished.\n");
    return 0;
}

