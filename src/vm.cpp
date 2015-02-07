#include "vm.h"

#include "lang_utils.h"
#include "value.h"
#include "vm/instruction.h"

#include <boost/variant/get.hpp>

using namespace il;

void vm::machine::run()
{
  using boost::get;

  while (m_base->instr_ptr.size()) {

    auto instr = m_base->instr_ptr.front().instr;
    const auto& arg = m_base->instr_ptr.front().arg;
    m_base->instr_ptr.remove_prefix(1);

    switch (instr) {
    case instruction::push_int:  push_int(get<int>(arg));                 break;
    case instruction::push_sym:  push_sym(get<symbol>(arg));              break;
    case instruction::push_bool: push_bool(get<bool>(arg));               break;
    case instruction::push_str:  push_str(get<std::string>(arg));         break;
    case instruction::push_flt:  push_flt(get<double>(arg));              break;
    case instruction::push_fn:   push_fn(get<std::vector<command>>(arg)); break;

    case instruction::read:  read(get<symbol>(arg));  break;
    case instruction::write: write(get<symbol>(arg)); break;
    case instruction::let:   let(get<symbol>(arg));   break;

    case instruction::push_self: push_self();                 break;
    case instruction::self:      self();                      break;
    case instruction::push_arg:  push_arg();                  break;
    case instruction::pop_arg:   pop_arg(get<symbol>(arg)); break;
    case instruction::member:    member(get<symbol>(arg));  break;
    case instruction::call:      call();                      break;
    case instruction::call_nat:  call_nat();                  break;

    case instruction::enter: enter(); break;
    case instruction::leave: leave(); break;

    case instruction::jump_unless: jump_unless(get<int>(arg)); break;
    case instruction::jump:        jump(get<int>(arg));        break;
    }
  }
}

void vm::machine::push_int(int val)
{
  static_assert(false, "implement!");
}

void vm::machine::push_sym(symbol val)
{
  static_assert(false, "implement!");
}

void vm::machine::push_bool(bool val)
{
  static_assert(false, "implement!");
}

void vm::machine::push_str(const std::string& val)
{
  static_assert(false, "implement!");
}

void vm::machine::push_flt(double val)
{
  static_assert(false, "implement!");
}

void vm::machine::push_fn(vector_ref<command> val)
{
  static_assert(false, "implement!");
}

void vm::machine::read(symbol sym)
{
  auto stack = m_base;
  while (!stack->local.count(sym))
    stack = stack->enclosing;
  m_retval = stack->local[sym];
}

void vm::machine::write(symbol sym)
{
  auto stack = m_base;
  while (!stack->local.count(sym))
    stack = stack->enclosing;
  stack->local[sym] = m_retval;
}

void vm::machine::let(symbol sym)
{
  m_base->local[sym] = m_retval;
}

void vm::machine::push_self()
{
  m_base->pushed_self = *m_retval;
}

void vm::machine::self()
{
  m_retval = &*m_base->self;
}

void vm::machine::push_arg()
{
  m_base->pushed_args.push_back(m_retval);
}

void vm::machine::pop_arg(symbol sym)
{
  m_base->local[sym] = m_base->parent->pushed_args.back();
  m_base->parent->pushed_args.pop_back();
}

void vm::machine::member(symbol sym)
{
  static_assert(false, "implement!");
}

void vm::machine::call()
{
  static_assert(false, "implement!");
}

void vm::machine::call_nat()
{
  static_assert(false, "implement!");
}


void vm::machine::enter()
{
  m_base = std::make_shared<call_stack>( m_base, m_base, m_base->instr_ptr );
}

void vm::machine::leave()
{
  if (m_base->parent && m_base->parent == m_base->enclosing)
    m_base->parent->instr_ptr = m_base->instr_ptr;
  m_base = m_base->parent;
  if (!m_base)
    exit(0);
}


void vm::machine::jump_unless(int offset)
{
  if (!truthy(m_retval))
    m_base->instr_ptr.remove_prefix(offset);
}

void vm::machine::jump(int offset)
{
  m_base->instr_ptr.remove_prefix(offset);
}
