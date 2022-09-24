#include <iostream>
#include <Windows.h>

using namespace std;

extern "C" {
	typedef long (WINAPI* RtlSetProcessIsCritical) (
		IN BOOLEAN	bNew,
		OUT BOOLEAN* pbOld,
		IN BOOLEAN	bNeedScb);
}

auto AdjustPrivlage(LPCSTR lpszPriv) -> bool
{
    PVOID hToken;
    LUID luid;
    TOKEN_PRIVILEGES tkprivs;
    ZeroMemory(&tkprivs, sizeof(tkprivs));

    if (!OpenProcessToken(GetCurrentProcess(), (TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY), &hToken))
        return FALSE;

    if (!LookupPrivilegeValue(NULL, lpszPriv, &luid)) {
        CloseHandle(hToken); return FALSE;
    }

    tkprivs.PrivilegeCount = 1;
    tkprivs.Privileges[0].Luid = luid;
    tkprivs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    BOOL bRet = AdjustTokenPrivileges(hToken, FALSE, &tkprivs, sizeof(tkprivs), NULL, NULL);
    CloseHandle(hToken);
    return bRet;
}

auto main() -> void{
	if (AdjustPrivlage(SE_DEBUG_NAME) != TRUE){
		std::cout << "Failed to raise privlage" << std::endl;
		return ;
	}

	HANDLE ntdll = LoadLibrary("ntdll.dll");
	if (ntdll == NULL){
		std::cout << "Failed to load ntdll\n" << std::flush;
		return ;
	}

	RtlSetProcessIsCritical SetCritical;
	SetCritical = reinterpret_cast<RtlSetProcessIsCritical>(GetProcAddress((HINSTANCE)ntdll, "RtlSetProcessIsCritical"));

	if (!SetCritical){
		std::cout << "Failed.\n" << std::flush;
		return ;
	}

	SetCritical(TRUE, NULL, FALSE);
	std::cout << "Success setting critical process\n" << std::flush;
	system("pause");

	SetCritical(FALSE, NULL, FALSE);

	return ;

}