#include "pch.h"
#include "PingTest.h"

CPingTest::CPingTest()
{
	AllocConsole();
	ShowWindow(GetConsoleWindow(), SW_HIDE);		// pys check
}


CPingTest::~CPingTest()
{
	FreeConsole();
}

bool CPingTest::PingTest(char* in_ip)
{
	char strCommand[MAX_PATH] = "ping ";
	strcat(strCommand, in_ip);

	int ret = system(strCommand);
	if (ret == 0)
		return true;
	else
		return false;
}