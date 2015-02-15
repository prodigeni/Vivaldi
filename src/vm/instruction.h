#ifndef VV_VM_INSTRUCTIONS_H
#define VV_VM_INSTRUCTIONS_H

#include "symbol.h"

#include <boost/variant/variant.hpp>

#include <unordered_map>
#include <vector>

namespace vv {

namespace vm {

struct command;

struct function_t {
  int argc;
  std::vector<command> body;
};

struct type_t {
  symbol name;
  symbol parent;
  std::unordered_map<symbol, function_t> methods;
};

struct nil_t { };

enum class instruction {
  /// pushes the provided Bool literal into retval
  push_bool,
  /// pushes the provided Float literal into retval
  push_flt,
  /// pushes the provided Function literal into retval
  push_fn,
  /// pushes the provided Integer literal into retval
  push_int,
  /// pushes a Nil literal into retval
  push_nil,
  /// pushes the provided String literal into retval
  push_str,
  /// pushes the provided Symbol literal into retval
  push_sym,
  /// Pushes the provided Type literal into retval
  push_type,

  /// sets retval to an Array made out of the provided number of pushed args
  make_arr,
  /// sets retval to a Dictionary made out of the provided number of pushed args
  make_dict,

  /// reads a variable into retval
  read,
  /// writes retval to a variable
  write,
  /// creates a new variable with value retval
  let,

  /// reads self into retval
  self,
  /// pushes retval onto arg stack
  push_arg,
  /// retrieves the nth passed argument, where n is the provided integer
  arg,
  /// reads a member into retval
  readm,
  /// sets a member to retval
  writem,
  /// calls retval, using the provided number of pushed arguments
  call,
  /// creates new object of the type in retval
  new_obj,

  /// enters a new block
  eblk,
  /// leaves current block
  lblk,
  /// returns from a function
  ret,

  /// pushes retval onto the arg/temporary stack
  push,
  /// pops a temporary off the stack into retval
  pop,

  /// loads and run a file with the provided name
  req,

  /// unconditionally jumps the provided number of commands
  jmp,
  /// jump the provided number of commands if retval is falsy
  jmp_false,
  /// jump the provided number of commands if retval is trutyh
  jmp_true,
  /// pushes retval as a new function for catching exceptions
  push_catch,
  /// pops an exception catcher and discards it, leaving retval unchanged
  pop_catch,
  /// throws retval as an exception
  except
};

struct command {
public:
  command(instruction instr, int arg);
  command(instruction instr, symbol arg);
  command(instruction instr, bool arg);
  command(instruction instr, const std::string& arg);
  command(instruction instr, double arg);
  command(instruction instr, const function_t& arg);
  command(instruction instr, const type_t& arg);
  command(instruction instr);

  instruction instr;
  boost::variant<int,
                symbol,
                bool,
                std::string,
                double,
                function_t,
                type_t,
                nil_t>
    arg;
};

}

}

#endif
