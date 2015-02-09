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

using namespace vv;

vm::machine::machine(std::shared_ptr<call_stack> frame)
  : m_stack  {frame},
    m_retval {nullptr},
    m_base   {frame}
{
  gc::set_current_frame(frame);
  gc::set_current_retval(m_retval);
}

void vm::machine::run()
{
  using boost::get;

  while (m_stack->instr_ptr.size()) {
    auto instr = m_stack->instr_ptr.front().instr;
    const auto& arg = m_stack->instr_ptr.front().arg;
    m_stack->instr_ptr = m_stack->instr_ptr.remove_prefix(1);

    if (instr != instruction::call)
      m_stack->pushed_self = {};

    switch (instr) {
    case instruction::push_bool: push_bool(get<bool>(arg));               break;
    case instruction::push_flt:  push_flt(get<double>(arg));              break;
    case instruction::push_fn:   push_fn(get<std::vector<command>>(arg)); break;
    case instruction::push_int:  push_int(get<int>(arg));                 break;
    case instruction::push_nil:  push_nil();                              break;
    case instruction::push_str:  push_str(get<std::string>(arg));         break;
    case instruction::push_sym:  push_sym(get<symbol>(arg));              break;

    case instruction::read:  read(get<symbol>(arg));  break;
    case instruction::write: write(get<symbol>(arg)); break;
    case instruction::let:   let(get<symbol>(arg));   break;

    case instruction::self:     self();                    break;
    case instruction::push_arg: push_arg();                break;
    case instruction::pop_arg:  pop_arg(get<symbol>(arg)); break;
    case instruction::readm:    readm(get<symbol>(arg));   break;
    case instruction::writem:   writem(get<symbol>(arg));  break;
    case instruction::call:     call(get<int>(arg));       break;

    case instruction::eblk: eblk(); break;
    case instruction::lblk: lblk(); break;
    case instruction::ret:  ret(); break;

    case instruction::jmp_false: jmp_false(get<int>(arg)); break;
    case instruction::jmp:       jmp(get<int>(arg));       break;
    }
  }
}

void vm::machine::push_bool(bool val)
{
  m_retval = gc::alloc<value::boolean>( val );
}

void vm::machine::push_flt(double val)
{
  m_retval = gc::alloc<value::floating_point>( val );
}

void vm::machine::push_fn(const std::vector<command>& val)
{
  m_retval = gc::alloc<value::function>( val, m_stack );
}

void vm::machine::push_int(int val)
{
  m_retval = gc::alloc<value::integer>( val );
}

void vm::machine::push_nil()
{
  m_retval = gc::alloc<value::nil>( );
}

void vm::machine::push_str(const std::string& val)
{
  m_retval = gc::alloc<value::string>( val );
}

void vm::machine::push_sym(symbol val)
{
  m_retval = gc::alloc<value::symbol>( val );
}

void vm::machine::read(symbol sym)
{
  auto stack = m_stack;
  while (stack) {
    auto holder = find_if(rbegin(stack->local), rend(stack->local),
                          [&](const auto& vars) { return vars.count(sym); });
    if (holder != rend(stack->local)) {
      m_retval = holder->at(sym);
      return;
    }
    stack = stack->enclosing;
  }
}

void vm::machine::write(symbol sym)
{
  auto stack = m_stack;
  for (;;) {
    auto holder = find_if(rbegin(stack->local), rend(stack->local),
                          [&](const auto& vars) { return vars.count(sym); });
    if (holder != rend(stack->local)) {
      holder->at(sym) = m_retval;
      return;
    }
    stack = stack->enclosing;
  }
}

void vm::machine::let(symbol sym)
{
  m_stack->local.back()[sym] = m_retval;
}

void vm::machine::self()
{
  auto stack = m_stack;
  while (!m_stack->self)
    stack = m_stack->enclosing;
  m_retval = &*stack->self;
}

void vm::machine::push_arg()
{
  m_stack->pushed_args.push_back(m_retval);
}

void vm::machine::pop_arg(symbol sym)
{
  m_stack->local.back()[sym] = m_stack->args.back();
  m_stack->args.pop_back();
}

void vm::machine::readm(symbol sym)
{
  m_stack->pushed_self = *m_retval;
  if (m_retval->members.count(sym))
    m_retval = m_retval->members[sym];
  else
    m_retval = m_retval->type->methods[sym];
}

void vm::machine::writem(symbol sym)
{
  auto value = m_stack->pushed_args.back();
  m_stack->pushed_args.pop_back();

  m_retval->members[sym] = value;
  m_retval = value;
}

void vm::machine::call(int argc)
{
  std::vector<value::base*> args;
  copy(end(m_stack->pushed_args) - argc, end(m_stack->pushed_args),
       back_inserter(args));
  m_stack->pushed_args.erase(end(m_stack->pushed_args) - argc,
                             end(m_stack->pushed_args));

  auto function = m_retval;
  if (auto type = dynamic_cast<value::type*>(function))
    function = type->constructor;

  if (auto fn = dynamic_cast<value::function*>(function)) {
    m_stack = std::make_shared<call_stack>( m_stack,
                                            fn->enclosure,
                                            move(args),
                                            fn->body );

    gc::set_current_frame(m_stack);

  } else if (auto fn = dynamic_cast<value::builtin_function*>(function)) {
    auto stack = std::make_shared<call_stack>( m_stack,
                                               m_base,
                                               move(args),
                                               m_stack->instr_ptr );
    gc::set_current_frame(stack);
    m_retval = fn->body(*stack);
    gc::set_current_frame(m_stack);
  }
}

void vm::machine::eblk()
{
  m_stack->local.emplace_back();
}

void vm::machine::lblk()
{
  m_stack->local.pop_back();
}

void vm::machine::ret()
{
  m_stack = m_stack->parent;
  if (!m_stack)
    exit(0);
  gc::set_current_frame(m_stack);
}

void vm::machine::jmp_false(int offset)
{
  if (!truthy(m_retval))
    m_stack->instr_ptr = m_stack->instr_ptr.remove_prefix(offset - 1);
}

void vm::machine::jmp(int offset)
{
  m_stack->instr_ptr = m_stack->instr_ptr.remove_prefix(offset - 1);
}
