#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int main(int argc, char* argv[])
{
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);

	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
