#ifndef VV_VM_INSTRUCTIONS_H
#define VV_VM_INSTRUCTIONS_H

#include "expression.h"
#include "symbol.h"
#include "value.h"

#include <boost/variant/variant.hpp>

namespace vv {

namespace vm {

struct nil_t { };

enum class instruction {
  push_bool,
  push_flt,
  push_fn,
  push_int,
  push_nil,
  push_str,
  push_sym,

  read,
  write,
  let,

  self,
  push_arg,
  pop_arg,
  readm,
  writem,
  call,

  eblk,
  lblk,
  ret,

  jmp_false,
  jmp
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
