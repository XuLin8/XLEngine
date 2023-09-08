#pragma once
namespace XLEngine
{
	//用于指示编译器将一个函数或类成员导出到动态链接库（DLL）中，以便其他程序可以在运行时访问该函数。
	__declspec(dllexport) void Print();
}