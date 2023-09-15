#include <CrashRpt.h>

crash_rpt::CrashRpt g_crashRpt(
	"979035E1-28F6-4701-A5EF-E0200F732471", // GUID assigned to this application.
	L"Sample Application", // Application name that will be used in message box.
	L"Idol Software"       // Company name that will be used in message box.
	);

int main(int argc, char* argv[])
{
	*((int*)0) = 0;

	return 0;
}
