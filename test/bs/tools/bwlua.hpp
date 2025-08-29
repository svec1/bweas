#ifndef TEST_BWLUA__H_
#define TEST_BWLUA__H_
#include <bs/utils/lua_wrapper.hpp>
#include <gtest/gtest.h>

#include <ctype.h>

using namespace bweas::utils;

#define TEST1_SOURCE_LUA                                                                                               \
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
function test_func4(struct) \
    str_g = struct[2]['test2'] \
    return struct[1]['test'] \
end \
test_func1_welcome() \
"

TEST(BWWRAP_LUA, NoThrowInitClassBwLua) {
    lua ltest;
    ASSERT_NO_THROW(ltest.create(TEST1_SOURCE_LUA));
    ASSERT_NO_THROW(ltest.close());
}

TEST(BWWRAP_LUA, NoThrowClassBwLuaVariablePerformance) {
    lua ltest(TEST1_SOURCE_LUA);
    lua::fast_table<std::string_view, std::any> test_var_vec{{"test", std::string("HELLO WORLD!")},
                                                             {"test2", (lua::integer)2}};
    lua::array<lua::integer> nums = {1, 2, 3, 4, 5};
    std::string str_tmp;

    ASSERT_NO_THROW({
        ltest["test"]  = test_var_vec;
        ltest["test2"] = nums;
    });
    ASSERT_EQ(std::any_cast<lua::number>(ltest["test"].getval<lua::fast_table<std::string_view, std::any>>()[0].second),
              2);
    ASSERT_EQ(ltest["test2"].getval<lua::array<lua::integer>>()[2], 3);
}

TEST(BWWRAP_LUA, NoThrowClassBwLuaCallFunction) {
    lua ltest(TEST1_SOURCE_LUA);
    lua::fast_table<std::string_view, std::any> test_var_vec{{"test", "HELLO WORLD!"}, {"test2", "HELLO WORLD!"}};
    lua::array<lua::integer> nums = {1, 2, 3, 4, 5};
    std::string str_tmp;
    int num_tmp;

    ltest.include_libs();

    // test_func1_welcome()
    ASSERT_NO_THROW({ str_tmp = ltest.call_function<std::string>("test_func1_welcome", lua::nil{}); });
    ASSERT_EQ(str_tmp, "Hello, World!");

    // test_func2()
    ASSERT_NO_THROW({
        num_tmp =
            ltest.call_function<lua::integer>("test_func2", (lua::integer)4, "Hello, ", (lua::integer)5, "World!");
    });
    ASSERT_EQ(num_tmp, 9);

    // test_func3()
    ASSERT_NO_THROW({ num_tmp = ltest.call_function<lua::integer>("test_func3", nums); });
    ASSERT_EQ(num_tmp, 15);

    // test_func4()
    ASSERT_NO_THROW({ str_tmp = ltest.call_function<std::string>("test_func4", test_var_vec); });
    ASSERT_EQ(ltest.get_var<std::string>("str_g"), "HELLO WORLD!");
    ASSERT_EQ(str_tmp, "HELLO WORLD!");
}

TEST(BWWRAP_LUA, NoThrowClassBwLuaVariableAStackNBad) {
    lua ltest(TEST1_SOURCE_LUA);
    int num_tmp;

    for (lua::integer i = 0; i < 1000; ++i) {
        ASSERT_NO_THROW({ num_tmp = ltest.call_function<lua::integer>("test_func2", i, "Hello, ", i * 10, "World!"); });
        ASSERT_EQ(num_tmp, i + i * 10);
        ASSERT_EQ(ltest.get_var<std::string>("str_g"), "Hello, World!");
    }
}

#endif
