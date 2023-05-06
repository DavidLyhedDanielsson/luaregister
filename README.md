## /!\ Work-in-progress

Expose C-functions to lua using a single function call which wraps the given
function and automatically manages the lua stack.

An explanation of the inner workings can be found at
[my webpage](https://www.lyhed.dev/projects/luaregister/)

### Primary goals
- Utilise templates and parameter packs to deduce function signature - the API
  should be able to be used without passing types
- Avoid recursive templates and unnecessary code generation
- Avoid (or enable opt-in) dynamic memory allocation
- Allow out-parameters (pointers)

## Requirements
C++20 for lua_register.hpp. Catch2 and lua 5.3 for the tests

## Features
Currently implemented:
- `const char*` and primitive type parameters.
- `const char*` and primitive type return values.
- Output parameters via pointers (multiple returns).
- C-functions taking an array as a pointer. Max 4 members.
- Variadic string functions. Max 32 parameters and 256 max total length;
  configurable by `MAX_VARIADIC_ARG_COUTN` and `STRING_BUFFER_LENGTH`.
- Passing custom types to/from Lua via template specialisation -
  `LuaRegister::LuaGetFunc`, `LuaRegister::LuaSetFunc`, and
  `LuaRegister::GetDefault`.
- Simple hard-coded default values.

Not implemented:
- Configurable array max length, though upping the limit is trivial.
- Error messages for wrong types general error handling.

## Usage
lua_register.hpp contains documentation. The only function needed from the
outside is one of the Register functions.

An example file which registers most of the
[Dear imgui](https://github.com/ocornut/imgui) widgets is found in the repo as
lua_example.cpp.

## Examples

lua_example.cpp contains examples of the following:
- Custom parameter types `ImVec2`, `ImVec4`
- Simple function calls `ImGui::Begin`
- Overloaded function registration `ImGui::BeginChild`
- Variadic function registration `ImGui::TextX`
- C-function taking an array as a pointer `ImGui::DragFloatX`

Some examples can be found through the tests.