#include <catch2/catch_test_macros.hpp>
#include <limits>
#include <cstring>
#include <string_view>

#include "../src/lua_register.hpp"

#define QuickRegister(func) LuaRegister::Register(lua, funcName, func);
#define CallFunc(ret)             \
    lua_getglobal(lua, funcName); \
    REQUIRE(lua_pcall(lua, 0, ret, 0) == LUA_OK);
#define QuickRun(code) REQUIRE(luaL_dostring(lua, code) == LUA_OK);

#define REQUIRE_SINGLE_RETURN_VALUE REQUIRE(lua_gettop(lua) == 1);

#define REQUIRE_NUMBER REQUIRE(lua_isnumber(lua, -1));
#define REQUIRE_STRING REQUIRE(lua_isstring(lua, -1));
#define REQUIRE_TABLE REQUIRE(lua_istable(lua, -1));

#define REQUIRE_NUMBER_VALUE(val) REQUIRE(lua_tonumber(lua, -1) == val);
#define REQUIRE_STRING_VALUE(val) REQUIRE(std::string_view(lua_tostring(lua, -1)) == val);

const char* funcName = "funcName";

TEST_CASE("Register arguments", "[Function registration]")
{
    lua_State* lua = luaL_newstate();

    SECTION("void (*)(void)")
    {
        static bool called = false;
        QuickRegister(+[]() { called = true; });
        QuickRun(R"###(funcName())###");
        REQUIRE(called);
    }

    SECTION("void (*)(const char*)")
    {
        static bool called = false;
        QuickRegister(+[](const char* arg) {
            called = true;
            REQUIRE(std::string_view(arg) == "It works!");
        });

        QuickRun(R"###(funcName("It works!"))###");
        REQUIRE(called);
    }

#define PrimitiveTest(type, expected, val) \
    SECTION("void (*)(" #type ")")         \
    {                                      \
        static bool called = false;        \
        QuickRegister(+[](type arg) {      \
            called = true;                 \
            REQUIRE(arg == expected);      \
        });                                \
        QuickRun("funcName(" #val ")");    \
        REQUIRE(called);                   \
    }

    PrimitiveTest(int, -123, -123);
    PrimitiveTest(unsigned int, 123, 123);
    PrimitiveTest(long, -12345, -12345);
    PrimitiveTest(unsigned long, 12345, 12345);
    PrimitiveTest(long long, -1234567, -1234567);
    PrimitiveTest(float, 123.4f, 123.4); // Lua floats don't use f
    PrimitiveTest(double, 123.456, 123.456);
    PrimitiveTest(bool, true, true);
    PrimitiveTest(bool, false, 1 == 2);

    SECTION("Multiple arguments")
    {
        static bool called = false;
        QuickRegister(+[](const char* arg, int i, float f, bool b) {
            called = true;
            REQUIRE(std::string_view(arg) == "It works!");
            REQUIRE(i == 123);
            REQUIRE(f == 1.23f);
            REQUIRE(b == true);
        });

        QuickRun(R"###(funcName("It works!", 123, 1.23, true))###");
        REQUIRE(called);
    }
}

TEST_CASE("Register return values", "[Function registration]")
{
    lua_State* lua = luaL_newstate();

#undef PrimitiveTest
#define PrimitiveTest(type, expected, val, func) \
    SECTION(#type " (*)())")                     \
    {                                            \
        static bool called = false;              \
        QuickRegister(+[]() {                    \
            called = true;                       \
            return val;                          \
        });                                      \
        CallFunc(1);                             \
        REQUIRE(called);                         \
        REQUIRE_SINGLE_RETURN_VALUE              \
        auto ret = func(lua, -1);                \
        REQUIRE(ret == expected);                \
    }

    PrimitiveTest(int, -123, -123, lua_tointeger);
    PrimitiveTest(unsigned int, 123, 123, lua_tointeger);
    PrimitiveTest(long, -12345, -12345, lua_tointeger);
    PrimitiveTest(unsigned long, 12345, 12345, lua_tointeger);
    PrimitiveTest(long long, -1234567, -1234567, lua_tointeger);
    PrimitiveTest(float, 123.4f, 123.4f, lua_tonumber);
    PrimitiveTest(double, 123.4, 123.4, lua_tonumber);
    PrimitiveTest(bool, true, true, lua_toboolean);
    PrimitiveTest(bool, false, false, lua_toboolean);

    SECTION("const char* (*)(void)")
    {
        static bool called = false;
        QuickRegister(+[]() {
            called = true;
            return "It works!";
        });
        CallFunc(1);

        REQUIRE(called);
        REQUIRE_SINGLE_RETURN_VALUE;
        REQUIRE_STRING;
        REQUIRE_STRING_VALUE("It works!");
    }
}

TEST_CASE("Register pointer arguments", "[Function registration]")
{
    lua_State* lua = luaL_newstate();

    SECTION("void (*)(float*) with primitive lua variable")
    {
        static bool called = false;
        QuickRegister(+[](float* ptr) {
            REQUIRE(*ptr == 1.0f);
            *ptr *= -1;
            called = true;
        });

        QuickRun("var = 1.0");
        lua_getglobal(lua, funcName);
        lua_getglobal(lua, "var");
        REQUIRE(lua_pcall(lua, 1, 1, 0) == LUA_OK);

        REQUIRE(called);
        REQUIRE_SINGLE_RETURN_VALUE;
        REQUIRE_NUMBER;
        REQUIRE_NUMBER_VALUE(-1.0f);
    }

    SECTION("void (*)(float*) with lua table")
    {
        static bool called = false;
        QuickRegister(+[](float* arr) {
            REQUIRE(arr[0] == 0.5f);
            REQUIRE(arr[1] == 0.25f);
            REQUIRE(arr[2] == 0.125f);
            REQUIRE(arr[3] == 0.0625f);

            arr[0] *= -1;
            arr[1] *= -1;
            arr[2] *= -1;
            arr[3] *= -1;

            called = true;
        });

        QuickRun("table = { 0.5, 0.25, 0.125, 0.0625 }");
        lua_getglobal(lua, funcName);
        lua_getglobal(lua, "table");
        REQUIRE(lua_pcall(lua, 1, 1, 0) == LUA_OK);

        REQUIRE(called);
        REQUIRE_SINGLE_RETURN_VALUE;
        REQUIRE_TABLE;

        lua_geti(lua, -1, 1);
        REQUIRE(lua_tonumber(lua, -1) == -0.5f);
        lua_geti(lua, -2, 2);
        REQUIRE(lua_tonumber(lua, -1) == -0.25f);
        lua_geti(lua, -3, 3);
        REQUIRE(lua_tonumber(lua, -1) == -0.125f);
        lua_geti(lua, -4, 4);
        REQUIRE(lua_tonumber(lua, -1) == -0.0625f);
    }
}