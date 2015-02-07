#include "vm.h"

#include "builtins.h"
#include "gc.h"
#include "lang_utils.h"

#include "value.h"
#include "value/builtin_function.h"
#include "value/boolean.h"
#include "value/integer.h"
#include "value/floating_point.h"
#include "value/function.h"
#include "value/nil.h"
#include "value/string.h"
#include "value/symbol.h"

#include "vm/instruction.h"

#include <boost/variant/get.hpp>

using namespace il;

void vm::machine::run()
{
  using boost::get;

  while (m_stack->instr_ptr.size()) {

    auto instr = m_stack->instr_ptr.front().instr;
    const auto& arg = m_stack->instr_ptr.front().arg;
    m_stack->instr_ptr.remove_prefix(1);

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

    case instruction::push_self: push_self();               break;
    case instruction::self:      self();                    break;
    case instruction::push_arg:  push_arg();                break;
    case instruction::pop_arg:   pop_arg(get<symbol>(arg)); break;
    case instruction::member:    member(get<symbol>(arg));  break;
    case instruction::call:      call(get<int>(arg));       break;

    case instruction::enter: enter(); break;
    case instruction::leave: leave(); break;

    case instruction::jump_unless: jump_unless(get<int>(arg)); break;
    case instruction::jump:        jump(get<int>(arg));        break;
    }
  }
}

void vm::machine::push_int(int val)
{
  m_retval = gc::alloc<value::integer>( val );
}

void vm::machine::push_sym(symbol val)
{
  m_retval = gc::alloc<value::symbol>( val );
}

void vm::machine::push_bool(bool val)
{
  m_retval = gc::alloc<value::boolean>( val );
}

void vm::machine::push_str(const std::string& val)
{
  m_retval = gc::alloc<value::string>( val );
}

void vm::machine::push_flt(double val)
{
  m_retval = gc::alloc<value::floating_point>( val );
}

void vm::machine::push_fn(vector_ref<command> val)
{
  m_retval = gc::alloc<value::function>( val, m_stack );
}

void vm::machine::read(symbol sym)
{
  auto stack = m_stack;
  while (!stack->local.count(sym))
    stack = stack->enclosing;
  m_retval = stack->local[sym];
}

void vm::machine::write(symbol sym)
{
  auto stack = m_stack;
  while (!stack->local.count(sym))
    stack = stack->enclosing;
  stack->local[sym] = m_retval;
}

void vm::machine::let(symbol sym)
{
  m_stack->local[sym] = m_retval;
}

void vm::machine::push_self()
{
  m_stack->pushed_self = *m_retval;
}

void vm::machine::self()
{
  m_retval = &*m_stack->self;
}

void vm::machine::push_arg()
{
  m_stack->pushed_args.push_back(m_retval);
}

void vm::machine::pop_arg(symbol sym)
{
  m_stack->local[sym] = m_stack->parent->pushed_args.back();
  m_stack->parent->pushed_args.pop_back();
}

void vm::machine::member(symbol sym)
{
  m_retval = m_retval->members[sym];
}

void vm::machine::call(int args)
{
  if (auto fn = dynamic_cast<value::function*>(m_retval)) {
    m_stack = std::make_shared<call_stack>( m_stack, fn->enclosure, fn->body );

  } else if (auto fn = dynamic_cast<value::builtin_function*>(m_retval)) {
    auto stack = std::make_shared<call_stack>( m_stack,
                                               m_base,
                                               m_stack->instr_ptr );
    m_retval = fn->body(stack);
  }
}

void vm::machine::enter()
{
  m_stack = std::make_shared<call_stack>( m_stack, m_stack, m_stack->instr_ptr );
}

void vm::machine::leave()
{
  if (m_stack->parent && m_stack->parent == m_stack->enclosing)
    m_stack->parent->instr_ptr = m_stack->instr_ptr;
  m_stack = m_stack->parent;
  if (!m_stack)
    exit(0);
}


void vm::machine::jump_unless(int offset)
{
  if (!truthy(m_retval))
    m_stack->instr_ptr.remove_prefix(offset);
}

void vm::machine::jump(int offset)
{
  m_stack->instr_ptr.remove_prefix(offset);
}
