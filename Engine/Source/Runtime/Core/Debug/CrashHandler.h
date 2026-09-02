#pragma once

namespace XLEngine::CrashHandler
{
	// 安装崩溃处理器：捕获未处理异常(SIGSEGV/ABRT/ILL/FPE 与 Windows SEH 访问违例)，
	// 在终止前把 异常码/故障地址/调用栈 写入引擎日志文件并打印到 stderr。
	// 应在引擎启动最早期（main 开头）调用一次；处理器按需延迟解析日志路径。
	void Install();
}