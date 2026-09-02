#include "xlpch.h"
#include "Test/EngineTest.h"
#include "Runtime/Core/Log/Log.h"

// 轻量测试运行器：遍历所有已注册 TEST，逐条执行并汇总失败数。
// 返回非零退出码供 CTest / 冒烟脚本判定失败（含 --help 快速列出用例）。
int main(int argc, char** argv)
{
    // 序列化路径内的 XL_CORE_TRACE 依赖日志单例，头less 测试不搭引擎/窗口，
    // 必须在跑用例前手动初始化 spdlog 全局 logger，否则 Deserialize 崩溃。
    XLEngine::Log::Init();

    using namespace XLEngine::Test;

    if (argc > 1 && std::string(argv[1]) == "--help")
    {
        std::printf("XLEngine ECS core tests: %zu registered cases.\n", All().size());
        for (const auto& t : All())
            std::printf("  - %s\n", t.Name.c_str());
        return 0;
    }

    int failedSuites = 0;
    for (const auto& t : All())
    {
        const int before = g_FailureCount;
        std::printf("[ RUN ] %s\n", t.Name.c_str());
        t.Body();
        if (g_FailureCount == before)
        {
            std::printf("[  OK ] %s\n", t.Name.c_str());
        }
        else
        {
            std::printf("[FAIL ] %s\n", t.Name.c_str());
            ++failedSuites;
        }
        fflush(stdout);
    }

    std::printf("---- %zu tests, %d suites failed, %d assertions failed ----\n",
        All().size(), failedSuites, g_FailureCount);
    return failedSuites > 0 ? 1 : 0;
}