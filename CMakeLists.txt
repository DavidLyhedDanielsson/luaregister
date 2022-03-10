cmake_minimum_required (VERSION 3.11)
project(lua_register_tests VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)

find_package(Catch2 3 REQUIRED)
find_package(Lua 5.3 REQUIRED)

add_executable(lua_register_tests tests/lua_register.cpp)
target_link_libraries(lua_register_tests PRIVATE Catch2::Catch2WithMain ${LUA_LIBRARIES})

include(CTest)
include(Catch)
catch_discover_tests(lua_register_tests)