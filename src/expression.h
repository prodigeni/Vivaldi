#ifndef IL_EXPRESSION_H
#define IL_EXPRESSION_H

#include "symbol.h"

#include <vector>

namespace il {

namespace vm {

struct command;

}

namespace ast {

class expression {
public:
  virtual std::vector<vm::command> generate() const = 0;
  virtual ~expression() { }
};

class function_definition;
class function_call;
class member;
class variable;
class literal;
class type_definition;
class cond_statement;
class while_loop;
class for_loop;
class assignment;
class variable_declaration;
class block;

}

}

#endif
