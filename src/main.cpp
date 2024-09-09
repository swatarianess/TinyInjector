//
// Created by Swat on 08/09/2024.
//

#include <windows.h>
#include <cwchar>

/**
 * \brief Our "main" function
 * \return Exit code
 */
unsigned int injector();

/**
 * \brief Parses the commandline arguments
 * \param[out] out_windowName The name of the window to inject
 * \param[out] out_dllPath The path of the dll to inject into the aforementioned window
 */
void parseCmdLineArguments(const wchar_t **out_windowName, const wchar_t **out_dllPath);

/**
 * \brief Prints help text
 */
void printHelpText();

/**
 * \brief Usage help text
 */
constexpr char USAGE_HELP_TEXT[] = "USAGE: tinyinjector.exe <Window Name> <Path to Dll>";

/**
 * \brief Retrieves the process ID using the window name.
 * \param windowName The window name
 * \return The process ID of the provided windowName, or 0 if not found
 */
DWORD getProcessIDByWindowName(const wchar_t *windowName);

/**
 * \brief Grabs the LoadLibraryW Process address
 * \return The process address for LoadLibraryW, or NULL if not found
 */
void* getLoadLibraryWAddress();

/**
 * \brief Application entry point
 * \return Exit code
 */
extern "C" unsigned int entry_point() {
    return injector();
}

unsigned int injector() {
    const wchar_t *windowName, *dllPath;
    parseCmdLineArguments(&windowName, &dllPath);

    if (!windowName || !dllPath) {
        printHelpText();
        return 0;
    }

    DWORD processID = getProcessIDByWindowName(windowName);
    if (!processID) {
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processID);
    if (!hProcess) {
        return 2;
    }

    void* allocation = VirtualAllocEx(hProcess, nullptr, 1024, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!allocation) {
        CloseHandle(hProcess);
        return 3;
    }

    BOOL res = WriteProcessMemory(hProcess, allocation, reinterpret_cast<const void*>(dllPath), (wcslen(dllPath) + 1)  * sizeof(wchar_t), nullptr);
    if (res == FALSE) {
        VirtualFreeEx(hProcess, allocation, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 4;
    }

    void* loadLibraryWAddress = getLoadLibraryWAddress();
    if (!loadLibraryWAddress) {
        VirtualFreeEx(hProcess, allocation, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 5;
    }

    HANDLE hThread = CreateRemoteThread(hProcess, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(loadLibraryWAddress), allocation, 0, nullptr);
    if (!hThread) {
        VirtualFreeEx(hProcess, allocation, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return 6;
    }

    // ReSharper disable once CppStringLiteralToCharPointerConversion
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), L"Success!\r\n", wcslen(L"Success!\r\n") * sizeof(wchar_t), nullptr,
              nullptr);

    CloseHandle(hThread);

    return 0;
}

void parseCmdLineArguments(const wchar_t **out_windowName, const wchar_t **out_dllPath) {
    int argc;
    const LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argc != 3) {
        *out_windowName = nullptr;
        *out_dllPath = nullptr;

        return;
    }

    *out_windowName = argv[1];
    *out_dllPath = argv[2];
}

void printHelpText() {
    constexpr size_t text_size = sizeof(USAGE_HELP_TEXT);
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), USAGE_HELP_TEXT, text_size, nullptr, nullptr);
}

DWORD getProcessIDByWindowName(const wchar_t *windowName) {
    HWND window = FindWindowW(nullptr, windowName);
    if (window == nullptr) {
        return 0;
    }

    DWORD pid;
    DWORD res = GetWindowThreadProcessId(window, &pid);

    if (res == 0) {
        return 0;
    }

    return pid;
}

void* getLoadLibraryWAddress() {
    const HMODULE hModule = GetModuleHandle("kernel32.dll");
    if (!hModule) {
        return nullptr;
    }

    return reinterpret_cast<void*>(GetProcAddress(hModule, "LoadLibraryW"));
}

int main() {
    return entry_point();
}