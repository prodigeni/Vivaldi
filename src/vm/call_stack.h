#ifndef IL_VM_CALL_STACK_H
#define IL_VM_CALL_STACK_H

#include "value.h"
#include "symbol.h"

#include <boost/optional/optional.hpp>

#include <unordered_map>

namespace il {

class vm_instruction;

class call_stack {
public:
private:
  boost::optional<call_stack&> m_parent;
  boost::optional<call_stack&> m_enclosing;
  std::unordered_map<symbol, value::base* const> m_local;
  std::unordered_map<symbol, value::base* const> m_parameters;
  boost::optional<value::base&> m_self;

  std::vector<value::base* const> m_pushed_args;
  boost::optional<value::base&> m_pushed_self;

  boost::optional<value::base&>  m_retrieved_val;
};

}

#endif
