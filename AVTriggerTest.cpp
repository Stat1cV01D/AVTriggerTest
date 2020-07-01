// AVTriggerTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// Uncomment for additional AV triggers
#define AV_USE_INET

// Comment this for even more AV triggers
// #define USE_EXCEPTIONS

#define EMPTY_MAIN

#include <iostream>
#include <string>

#ifdef AV_USE_INET
#include <WinSock2.h>
#include <WinInet.h>
#include <iphlpapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "iphlpapi.lib")
#endif // AV_USE_INET

// Include Windows.h after WinSock2.h
#include <Windows.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#ifdef _UNICODE
#error "We need to compile in ANSI for this case"
#endif // _UNICODE


#ifdef AV_USE_INET
struct NetworkInfo
{
    std::string ipAddress;
    std::string macAddress;
};

NetworkInfo getNetworkInfo()
{
    NetworkInfo result;
    result.macAddress = "error";
    result.ipAddress = "error";

    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    ULONG family = AF_INET;

    DWORD bufferLength = 0;

    DWORD resultCode = GetAdaptersAddresses(family, flags, NULL, nullptr, &bufferLength);

    if (resultCode != ERROR_BUFFER_OVERFLOW)
    {
        return result;
    }

    auto adapterAddresses = reinterpret_cast<PIP_ADAPTER_ADDRESSES>(HeapAlloc(GetProcessHeap(), 0, bufferLength));

    resultCode = GetAdaptersAddresses(family, flags, NULL, adapterAddresses, &bufferLength);

    if (resultCode != NO_ERROR)
    {
        return result;
    }

    return result;
}
#endif // AV_USE_INET

const int DEFAULT_ERROR_CODE = 1;

class Exception : public std::exception
{
public:
    Exception(const std::string& str, int)
        : std::exception(str.c_str()) 
    {}
};

bool runExecutable(const std::string& exePath)
{
    SHELLEXECUTEINFO shellExecuteInfo = { 0 };
    shellExecuteInfo.cbSize = sizeof(shellExecuteInfo);
    shellExecuteInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shellExecuteInfo.hwnd = 0;
    shellExecuteInfo.lpVerb = "runas";
    shellExecuteInfo.lpFile = exePath.c_str();
    shellExecuteInfo.lpDirectory = 0;
    shellExecuteInfo.nShow = SW_SHOW;
    shellExecuteInfo.hInstApp = 0;
    shellExecuteInfo.lpParameters = "/app parameter";

    SHFILEINFO fileInfo = { 0 };

    if (FALSE == ::PathFileExists(exePath.c_str()))
    {
        throw Exception("File not found at path: " + exePath, DEFAULT_ERROR_CODE);
    }

    if (FALSE == ::SHGetFileInfo(exePath.c_str(),
        NULL,
        &fileInfo,
        sizeof(fileInfo),
        SHGFI_EXETYPE))
    {
        throw Exception("Not executable file at: " + exePath, DEFAULT_ERROR_CODE);
    }

    if (FALSE == ::ShellExecuteEx(&shellExecuteInfo))
    {
        auto errCode = ::GetLastError();
        std::string errorMessage = "Error while launch file: ";

        throw Exception(errorMessage, errCode);
    }

    const int errorCode = reinterpret_cast<int>(shellExecuteInfo.hInstApp);
    if (errorCode <= 32)
    {
        switch (errorCode)
        {
        case ERROR_FILE_NOT_FOUND:
            throw Exception("File not found at path: " + exePath, errorCode);
        case ERROR_PATH_NOT_FOUND:
            throw Exception("Path not found at path: " + exePath, errorCode);
        case SE_ERR_ACCESSDENIED:
            throw Exception("Access denied: " + exePath, errorCode);
        case SE_ERR_OOM:
            throw Exception("Not enough memory or resources to run at", errorCode);
        case SE_ERR_SHARE:
            throw Exception("Cannot share an open file :" + exePath, errorCode);
        default:
            throw Exception("Error while launch file", errorCode);
        }
    }

    //return shellExecuteInfo.hProcess;

    return true;
}

int main()
{
#if (defined EMPTY_MAIN)
#elif (defined USE_EXCEPTIONS)
    try
    {
        runExecutable("test.exe");
#ifdef AV_USE_INET
        auto info = getNetworkInfo();
#endif
    }
    catch (std::exception e)
    {
        std::cout << e.what();
        return 1;
    }
#else
    runExecutable("test.exe");
#ifdef AV_USE_INET
    auto info = getNetworkInfo();
#endif
#endif // USE_EXCEPTIONS
    return 0;
}