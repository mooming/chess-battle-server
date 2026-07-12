#pragma once

// Minimal header-only test framework. No external dependencies.

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <functional>

namespace test
{

struct TestCase
{
    std::string name;
    std::function<bool()> func;
};

inline std::vector<TestCase>& registry()
{
    static std::vector<TestCase> cases;
    return cases;
}

inline int registerTest(const std::string& name, std::function<bool()> func)
{
    registry().push_back({name, func});
    return 0;
}

#define TEST(name) \
    static bool test_##name(); \
    static int _reg_##name = test::registerTest(#name, test_##name); \
    static bool test_##name()

#define ASSERT_TRUE(expr) \
    do { if (!(expr)) { std::cerr << "  FAIL: " #expr " at " << __FILE__ << ":" << __LINE__ << std::endl; return false; } } while(0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b) \
    do { if ((a) != (b)) { std::cerr << "  FAIL: " #a " == " #b " at " << __FILE__ << ":" << __LINE__ << std::endl; return false; } } while(0)

#define ASSERT_NE(a, b) \
    do { if ((a) == (b)) { std::cerr << "  FAIL: " #a " != " #b " at " << __FILE__ << ":" << __LINE__ << std::endl; return false; } } while(0)

inline int runAll()
{
    int passed = 0, failed = 0;
    for (const auto& tc : registry())
    {
        std::cout << "[ RUN      ] " << tc.name << std::endl;
        try
        {
            if (tc.func())
            {
                std::cout << "[       OK ] " << tc.name << std::endl;
                ++passed;
            }
            else
            {
                std::cout << "[  FAILED  ] " << tc.name << std::endl;
                ++failed;
            }
        }
        catch (const std::exception& e)
        {
            std::cout << "[  FAILED  ] " << tc.name << " (exception: " << e.what() << ")" << std::endl;
            ++failed;
        }
    }
    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << passed << " passed, " << failed << " failed, "
              << (passed + failed) << " total" << std::endl;
    return failed > 0 ? 1 : 0;
}

} // namespace test
