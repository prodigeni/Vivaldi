cmake_minimum_required (VERSION 3.0)
project (vivaldi)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wold-style-cast -Wconversion -Wsign-conversion -O3 -std=c++1y -lc++abi -ggdb")

include_directories("src" SYSTEM "/usr/local/Cellar/boost/1.57.0/include")

add_executable(vivaldi
  src/main.cpp

  src/gc.cpp
  src/lang_utils.cpp
  src/symbol.cpp
  src/value.cpp
  src/vm.cpp

  src/builtins.cpp

  src/tokenizer.cpp
  src/validator.cpp
  src/parser.cpp

  src/ast/assignment.cpp
  src/ast/block.cpp
  src/ast/cond_statement.cpp
  src/ast/for_loop.cpp
  src/ast/function_call.cpp
  src/ast/function_definition.cpp
  src/ast/literal.cpp
  src/ast/member.cpp
  src/ast/member_assignment.cpp
  src/ast/try_catch.cpp
  src/ast/type_definition.cpp
  src/ast/variable.cpp
  src/ast/variable_declaration.cpp
  src/ast/while_loop.cpp

  src/value/array.cpp
  src/value/boolean.cpp
  src/value/builtin_function.cpp
  src/value/floating_point.cpp
  src/value/function.cpp
  src/value/integer.cpp
  src/value/nil.cpp
  src/value/string.cpp
  src/value/symbol.cpp

  src/vm/call_stack.cpp
  src/vm/instruction.cpp)
