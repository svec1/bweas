#include <gtest/gtest.h>
#include <bs/tools/bwlua.hpp>

#define TEST1_SOURCE_LUA \
"\
str_g = \"\"\n\
function test_func1_welcome() \
    return \"Hello, World!\" \
end \
function test_func2(num, str, num2, str2)\
    str_g = str .. str2 \
    return num+num2 \
end \
function test_func3(nums) \
    local sum = 0 \
    for _, num in ipairs(nums) do \
        sum = sum + num \
    end \
    return sum \
end \
test_func1_welcome() \
"

#define call_test_func1 ltest.call_function__nmutex<std::string, bwlua::lua::nil>("test_func1_welcome", bwlua::lua::nil{})
#define call_test_func2(num1, str1, num2, str2) ltest.call_function__nmutex<int, int, std::string, int, std::string>("test_func2", num1, str1, num2, str2)
#define call_test_func3(list) ltest.call_function__nmutex<int, std::vector<int>>("test_func3", list)

TEST(BWWRAP_LUA, WorkClassBwLua){
    bwlua::lua ltest;
    ASSERT_NO_THROW(ltest.create__nmutex(TEST1_SOURCE_LUA));
    ASSERT_NO_THROW(ltest.close__nmutex());
}

TEST(BWWRAP_LUA, NoThrowClassBwLuaCallFuncion){
    bwlua::lua ltest(TEST1_SOURCE_LUA);
    std::vector<int> nums = {1, 2, 3, 4, 5};
    std::string str_tmp;
    int num_tmp;

    // test_func1_welcome()
    ASSERT_NO_THROW(str_tmp = call_test_func1);
    ASSERT_EQ(str_tmp, "Hello, World!");

    // test_func2()
    ASSERT_NO_THROW(num_tmp = call_test_func2(4, "Hello, ", 5, "World!"));
    ASSERT_EQ(num_tmp, 9);
    ASSERT_EQ(ltest.get_var__nmutex<std::string>("str_g"), "Hello, World!");

    ASSERT_NO_THROW(num_tmp = call_test_func3(nums));
    ASSERT_EQ(num_tmp, 15);
}