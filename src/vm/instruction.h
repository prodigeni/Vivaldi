#ifndef IL_VM_INSTRUCTIONS_H
#define IL_VM_INSTRUCTIONS_H

#include "expression.h"
#include "symbol.h"
#include "value.h"

#include <boost/variant/variant.hpp>

namespace il {

namespace vm {

struct nil_t { };

enum class instruction {
  push_int,
  push_sym,
  push_bool,
  push_str,
  push_flt,
  push_fn,

  read,
  write,
  let,

  push_self,
  self,
  push_arg,
  pop_arg,
  member,
  call,
  call_nat,

  enter,
  leave,

  jump_unless,
  jump
};

struct command {
public:
  command(instruction instr, int arg);
  command(instruction instr, symbol arg);
  command(instruction instr, bool arg);
  command(instruction instr, const std::string& arg);
  command(instruction instr, double arg);
  command(instruction instr, const std::vector<command>& arg);
  command(instruction instr);

  instruction instr;
  boost::variant<int,
                symbol,
                bool,
                std::string,
                double,
                std::vector<command>,
                nil_t>
    arg;
};

}

}

#endif
