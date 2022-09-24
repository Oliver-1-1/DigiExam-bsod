#include <stdio.h>
#include <Windows.h>
#include <winternl.h>
#pragma comment(lib,"ntdll.lib") // Need to link with ntdll.lib import library. You can find the ntdll.lib from the Windows DDK.;

typedef struct _SYSTEM_PROCESS_INFO
{
    ULONG                   NextEntryOffset;
    ULONG                   NumberOfThreads;
    LARGE_INTEGER           Reserved[3];
    LARGE_INTEGER           CreateTime;
    LARGE_INTEGER           UserTime;
    LARGE_INTEGER           KernelTime;
    UNICODE_STRING          ImageName;
    ULONG                   BasePriority;
    HANDLE                  ProcessId;
    HANDLE                  InheritedFromProcessId;
}SYSTEM_PROCESS_INFO, * PSYSTEM_PROCESS_INFO;

auto get_process_id(LPCWSTR str) -> int{
    NTSTATUS status;
    PVOID buffer;
    PSYSTEM_PROCESS_INFO spi;

    buffer = VirtualAlloc(NULL, 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!buffer)  return 0;

    spi = (PSYSTEM_PROCESS_INFO)buffer;

    if (!NT_SUCCESS(status = NtQuerySystemInformation(SystemProcessInformation, spi, 1024 * 1024, NULL))) {

        VirtualFree(buffer, 0, MEM_RELEASE);
        return 0;
    }

    while (spi->NextEntryOffset) {
        if (!lstrcmpW(spi->ImageName.Buffer, str)) {
            int result = (int)spi->ProcessId;
            VirtualFree(buffer, 0, MEM_RELEASE);
            return result;
        }

        spi = (PSYSTEM_PROCESS_INFO)((LPBYTE)spi + spi->NextEntryOffset);
    }

    VirtualFree(buffer, 0, MEM_RELEASE);
    return 0;
}
auto main() -> void {
    auto handle = OpenProcess(PROCESS_ALL_ACCESS, TRUE, get_process_id(L"DigiExam-CriticalProcess-Bypass.exe"));
    TerminateProcess(handle, -1);
}