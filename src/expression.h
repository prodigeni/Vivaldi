#ifndef VV_EXPRESSION_H
#define VV_EXPRESSION_H

#include "symbol.h"

#include <vector>

namespace vv {

namespace vm {

struct command;

}

namespace ast {

class expression {
public:
  virtual std::vector<vm::command> generate() const = 0;
  virtual ~expression() { }
};

class assignment;
class block;
class cond_statement;
class except;
class for_loop;
class function_call;
class function_definition;
class logical_and;
class logical_or;
class member;
class member_assignment;
class return_statement;
class try_catch;
class type_definition;
class while_loop;
class variable;
class variable_declaration;

namespace literal {

class boolean;
class floating_point;
class integer;
class nil;
class string;
class symbol;

}

}

}

#endif
