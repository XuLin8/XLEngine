#include "xlpch.h"
#include "CrashHandler.h"

#include <csignal>
#include <cstdio>
#include <cstring>
#include <io.h>
#include <sstream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <dbghelp.h>

namespace XLEngine::CrashHandler
{
	namespace
	{
		// 日志路径：与 Log 一致，取可执行文件旁 logs\XLEngine.log（崩溃时按需解析，
		// 此时 Log::Init 已完成日志轮转，XLEngine.log 即当前启动实例的日志文件）。
		std::filesystem::path LogPathByExe()
		{
			wchar_t buf[MAX_PATH];
			const DWORD n = GetModuleFileNameW(nullptr, buf, MAX_PATH);
			std::filesystem::path exe(n ? std::wstring(buf, n) : std::wstring(L"XLEngineEditor.exe"));
			const std::filesystem::path p = exe.parent_path() / "logs" / "XLEngine.log";
			std::error_code ec;
			std::filesystem::create_directories(p.parent_path(), ec);
			return p;
		}

		// 追加写入崩溃记录。刻意不用 spdlog（崩溃态下调用日志器有重入/加锁风险），
		// 直接 fopen 追加同一日志文件，保证与常规日志串在同一条时间线。
		void AppendCrashRecord(const std::string& text)
		{
			const std::string path = LogPathByExe().string();
			FILE* f = nullptr;
			if (fopen_s(&f, path.c_str(), "a") == 0 && f)
			{
				fprintf(f, "[CRASH] %s\n", text.c_str());
				fflush(f);
				fclose(f);
			}
			fprintf(stderr, "[CRASH] %s\n", text.c_str());
			fflush(stderr);
		}

		// 交互式终端崩溃时保持控制台打开，等用户回车再退出（否则窗口一闪即关，看不清报告）。
		// 仅当 stdin 是真实终端时阻塞；自动化(冒烟/管道)调用时 _isatty 为假，直接返回不阻塞。
		void HoldForRead()
		{
			if (_isatty(_fileno(stdin)) == 0)
				return;
			fprintf(stderr, "\n[XLEngine][CRASH] 引擎已崩溃，详情见 logs/XLEngine.log。按回车键退出...\n");
			fflush(stderr);
			int c;
			while ((c = getchar()) != EOF && c != '\n') {}
			int d = getchar();
			(void)d;
		}

		void PrintResolvedAddr(std::ostringstream& oss, uint64_t addr,
		                       bool* symOk, SYMBOL_INFO& sym, HMODULE dbghelp)
		{
			using FnSymAddr = BOOL(WINAPI*)(HANDLE, DWORD64, PDWORD64, SYMBOL_INFO*);
			const auto symAddr = dbghelp ? (FnSymAddr)GetProcAddress(dbghelp, "SymFromAddr") : (FnSymAddr)nullptr;

			DWORD64 disp = 0;
			if (*symOk && symAddr && symAddr(GetCurrentProcess(), (DWORD64)addr, &disp, &sym))
				oss << "  " << sym.Name << " +0x" << std::hex << (unsigned long long)disp << "\n";
			else
				oss << "  0x" << std::hex << (unsigned long long)addr << "\n";
			sym.Name[0] = '\0'; // 复位名称，供下一帧复用同一缓冲区
		}

		// 用 dbghelp 动态解析调用栈返回地址到 {函数+偏移}；无符号/加载失败则降级为地址。
		std::string BacktraceText(void* const* frames, WORD count, uint64_t programCounter)
		{
			std::ostringstream oss;
			oss << std::hex;

			HMODULE dbghelp = LoadLibraryW(L"dbghelp.dll");
			using FnSymInit = BOOL(WINAPI*)(HANDLE, const char*, BOOL);
			const auto symInit = dbghelp ? (FnSymInit)GetProcAddress(dbghelp, "SymInitialize") : (FnSymInit)nullptr;
			bool symOk = false;
			if (symInit)
				symOk = symInit(GetCurrentProcess(), nullptr, TRUE) != 0;

			alignas(SYMBOL_INFO) unsigned char symBytes[sizeof(SYMBOL_INFO) + 256];
			SYMBOL_INFO& sym = *reinterpret_cast<SYMBOL_INFO*>(symBytes);
			std::memset(&sym, 0, sizeof(SYMBOL_INFO) + 256);
			sym.MaxNameLen = 255;
			sym.SizeOfStruct = sizeof(SYMBOL_INFO);

			constexpr int kMaxFrames = 32;
			if (programCounter) PrintResolvedAddr(oss, programCounter, &symOk, sym, dbghelp);
			for (WORD i = 0; i < count && i < kMaxFrames; ++i)
				PrintResolvedAddr(oss, reinterpret_cast<uint64_t>(frames[i]), &symOk, sym, dbghelp);

			if (dbghelp) FreeLibrary(dbghelp);
			return oss.str();
		}

		void Report(const char* kind, unsigned long code, unsigned long long faultAddr,
		            void* const* frames, WORD nFrames, uint64_t pc)
		{
			std::ostringstream oss;
			oss << "==== " << kind << " exception=0x" << std::hex << code
			    << " fault=0x" << faultAddr << " rip=0x" << pc << " ====\n";
			oss << BacktraceText(frames, nFrames, pc);
			AppendCrashRecord(oss.str());
		}

		// Windows SEH 未处理异常过滤器：能拿到访问违例的类型与故障读写地址。
		LONG WINAPI SehFilter(EXCEPTION_POINTERS* ep)
		{
			if (!ep) return EXCEPTION_CONTINUE_SEARCH;
			const EXCEPTION_RECORD* er = ep->ExceptionRecord;
			const unsigned long code = er->ExceptionCode;
			unsigned long long fault = er->ExceptionAddress ? (unsigned long long)er->ExceptionAddress : 0ull;
			if (code == EXCEPTION_ACCESS_VIOLATION && er->NumberParameters >= 2)
				fault = (unsigned long long)er->ExceptionInformation[1]; // 访问违例目标地址

			void* frames[32];
			const WORD n = CaptureStackBackTrace(2, 32, frames, nullptr);
			uint64_t pc = 0;
#if defined(_M_X64) || defined(__x86_64__)
			if (ep->ContextRecord) pc = (uint64_t)ep->ContextRecord->Rip;
#endif
			Report("UNHANDLED EXCEPTION", code, fault, frames, n, pc);
			HoldForRead();                    // 终端运行时不立即关闭控制台，等用户回车再看清日志
			return EXCEPTION_CONTINUE_SEARCH; // 放行给系统默认处理（WER 崩溃转储/终止）
		}

		void SignalHandler(int sig)
		{
			void* frames[32];
			const WORD n = CaptureStackBackTrace(2, 32, frames, nullptr);
			Report("SIGNAL", (unsigned long)sig, 0ull, frames, n, 0);
			HoldForRead();   // 终端运行时不立即关闭控制台，等用户回车再看清日志
			std::raise(sig); // 交由默认终止（同时触发 WER 崩溃转储）
		}
	}

	void Install()
	{
		SetUnhandledExceptionFilter(SehFilter);
		std::signal(SIGSEGV, SignalHandler);
		std::signal(SIGABRT, SignalHandler);
		std::signal(SIGILL, SignalHandler);
		std::signal(SIGFPE, SignalHandler);
	}
}