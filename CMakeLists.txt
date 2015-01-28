cmake_minimum_required (VERSION 3.0)
project (il)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -std=c++1y -lc++abi -ggdb")

include_directories("/usr/local/Cellar/boost/1.57.0/include" "src")

add_executable(il
  src/main.cpp
  src/repl.cpp

  src/environment.cpp
  src/gc.cpp
  src/lang_utils.cpp
  src/symbol.cpp
  src/value.cpp

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
  src/ast/type_definition.cpp
  src/ast/variable.cpp
  src/ast/variable_declaration.cpp
  src/ast/while_loop.cpp

  src/value/boolean.cpp
  src/value/custom_object.cpp
  src/value/custom_type.cpp
  src/value/floating_point.cpp
  src/value/function.cpp
  src/value/integer.cpp
  src/value/nil.cpp
  src/value/string.cpp
  src/value/symbol.cpp)
