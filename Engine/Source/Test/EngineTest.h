#pragma once
// 轻量单元测试头：无第三方依赖（MinGW/离线环境可直接编译）。
// 用法：
//   #include "Test/EngineTest.h"
//   TEST(GroupName) { EXPECT_EQ(a, b); EXPECT_TRUE(c); }
// 提供 TEST / EXPECT_TRUE / EXPECT_EQ / EXPECT_NEAR 宏，自动注册并由 TestMain.cpp 统一驱动。
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include <functional>

namespace XLEngine::Test
{
    struct TestEntry
    {
        std::string Name;
        std::function<void()> Body;
    };

    // 测试表（函数局部静态，跨翻译单元共享）
    inline std::vector<TestEntry>& All()
    {
        static std::vector<TestEntry> s_Tests;
        return s_Tests;
    }

    inline int g_FailureCount = 0;

    struct Registrar
    {
        Registrar(const char* name, void (*body)())
        {
            All().push_back({ name, body });
        }
    };

    inline void ReportFailure(const char* file, int line, const std::string& msg)
    {
        ++g_FailureCount;
        std::fprintf(stderr, "      [FAIL] %s:%d  %s\n", file, line, msg.c_str());
    }

    // 值打印重载（用于 EXPECT_EQ 失败时的读值辅助）
    inline std::string ToString(bool v) { return v ? "true" : "false"; }
    inline std::string ToString(int v) { return std::to_string(v); }
    inline std::string ToString(unsigned v) { return std::to_string(v); }
    inline std::string ToString(long v) { return std::to_string(v); }
    inline std::string ToString(unsigned long v) { return std::to_string(v); }
    inline std::string ToString(long long v) { return std::to_string(v); }
    inline std::string ToString(unsigned long long v) { return std::to_string(v); }
    inline std::string ToString(float v) { return std::to_string(v); }
    inline std::string ToString(double v) { return std::to_string(v); }
    inline std::string ToString(const std::string& v) { return v; }
    template<typename T> std::string ToString(const T&) { return "<value>"; }
}

#define TEST(name) \
    static void name(); \
    static ::XLEngine::Test::Registrar name##_reg(#name, &name); \
    static void name()

#define EXPECT_TRUE(cond) \
    do { if (!(cond)) ::XLEngine::Test::ReportFailure(__FILE__, __LINE__, "EXPECT_TRUE(" #cond ") failed"); } while (0)

#define EXPECT_FALSE(cond) \
    do { if (cond) ::XLEngine::Test::ReportFailure(__FILE__, __LINE__, "EXPECT_FALSE(" #cond ") failed"); } while (0)

#define EXPECT_EQ(lhs, rhs) \
    do { \
        const auto& _xl_l = (lhs); \
        const auto& _xl_r = (rhs); \
        if (!(_xl_l == _xl_r)) \
            ::XLEngine::Test::ReportFailure(__FILE__, __LINE__, \
                std::string("EXPECT_EQ(" #lhs ", " #rhs ")  left=[" + ::XLEngine::Test::ToString(_xl_l) \
                + "]  right=[" + ::XLEngine::Test::ToString(_xl_r) + "]")); \
    } while (0)

#define EXPECT_NEAR(lhs, rhs, eps) \
    do { \
        const auto _xl_l = (lhs); \
        const auto _xl_r = (rhs); \
        if (std::abs((double)(_xl_l) - (double)(_xl_r)) > (eps)) \
            ::XLEngine::Test::ReportFailure(__FILE__, __LINE__, \
                std::string("EXPECT_NEAR(" #lhs ", " #rhs ")  diff too large")); \
    } while (0)