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

#include <iostream>

using namespace vv;

vm::machine::machine(std::shared_ptr<call_stack> frame)
  : stack  {frame},
    retval {nullptr},
    m_base {frame}
{
  gc::set_current_frame(frame);
  gc::set_current_retval(retval);
}

void vm::machine::run()
{
  using boost::get;

  while (stack->instr_ptr.size()) {
    // Get next instruction (and argument, if it exists), and increment the
    // instruction pointer
    auto instr = stack->instr_ptr.front().instr;
    const auto& arg = stack->instr_ptr.front().arg;
    stack->instr_ptr = stack->instr_ptr.remove_prefix(1);

    // HACK--- avoid weirdness like the following:
    //   let i = 1
    //   let add = i.add // pushed_self is now i
    //   add(2)          // => 3
    //   5 + 1           // pushed self is now 5
    //   add(2)          // => 7
    if (instr != instruction::call)
      stack->pushed_self = {};

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

    case instruction::push_catch: push_catch();            break;
    case instruction::pop_catch: pop_catch();              break;
    case instruction::except:     except();                break;
    }
  }
}

// Instruction implementations {{{

void vm::machine::push_bool(bool val)
{
  retval = gc::alloc<value::boolean>( val );
}

void vm::machine::push_flt(double val)
{
  retval = gc::alloc<value::floating_point>( val );
}

void vm::machine::push_fn(const std::vector<command>& val)
{
  retval = gc::alloc<value::function>( val, stack );
}

void vm::machine::push_int(int val)
{
  retval = gc::alloc<value::integer>( val );
}

void vm::machine::push_nil()
{
  retval = gc::alloc<value::nil>( );
}

void vm::machine::push_str(const std::string& val)
{
  retval = gc::alloc<value::string>( val );
}

void vm::machine::push_sym(symbol val)
{
  retval = gc::alloc<value::symbol>( val );
}

void vm::machine::read(symbol sym)
{
  auto cur_stack = stack;
  while (cur_stack) {
    auto holder = find_if(rbegin(cur_stack->local), rend(cur_stack->local),
                          [&](const auto& vars) { return vars.count(sym); });
    if (holder != rend(cur_stack->local)) {
      retval = holder->at(sym);
      return;
    }
    cur_stack = cur_stack->enclosing;
  }
  push_str("no such variable");
  except();
}

void vm::machine::write(symbol sym)
{
  auto cur_stack = stack;
  for (;;) {
    auto holder = find_if(rbegin(cur_stack->local), rend(cur_stack->local),
                          [&](const auto& vars) { return vars.count(sym); });
    if (holder != rend(cur_stack->local)) {
      holder->at(sym) = retval;
      return;
    }
    stack = stack->enclosing;
  }
}

void vm::machine::let(symbol sym)
{
  stack->local.back()[sym] = retval;
}

void vm::machine::self()
{
  auto cur_stack = stack;
  while (cur_stack && !cur_stack->self)
    cur_stack = cur_stack->enclosing;
  if (cur_stack) {
    retval = &*cur_stack->self;
  } else {
    push_str("self does not exist outside of objects");
    except();
  }
}

void vm::machine::push_arg()
{
  stack->pushed_args.push_back(retval);
}

void vm::machine::pop_arg(symbol sym)
{
  stack->local.back()[sym] = stack->args.back();
  stack->args.pop_back();
}

void vm::machine::readm(symbol sym)
{
  stack->pushed_self = *retval;
  if (retval->members.count(sym))
    retval = retval->members[sym];
  else if (retval->type->methods.count(sym))
    retval = retval->type->methods[sym];
  push_str("no such member");
  except();
}

void vm::machine::writem(symbol sym)
{
  auto value = stack->pushed_args.back();
  stack->pushed_args.pop_back();

  retval->members[sym] = value;
  retval = value;
}

// TODO: make suck less.
void vm::machine::call(int argc)
{
  std::vector<value::base*> args;
  copy(end(stack->pushed_args) - argc, end(stack->pushed_args),
       back_inserter(args));
  stack->pushed_args.erase(end(stack->pushed_args) - argc,
                           end(stack->pushed_args));

  auto function = retval;
  if (auto type = dynamic_cast<value::type*>(function))
    function = type->constructor;

  if (auto fn = dynamic_cast<value::function*>(function)) {
    stack = std::make_shared<call_stack>( stack,
                                          fn->enclosure,
                                          move(args),
                                          fn->body );
    stack->caller = *function;

    gc::set_current_frame(stack);

  } else if (auto fn = dynamic_cast<value::builtin_function*>(function)) {
    stack = std::make_shared<call_stack>( stack,
                                          m_base,
                                          move(args),
                                          stack->instr_ptr );
    stack->caller = *function;

    auto except_flag = stack.get();
    gc::set_current_frame(stack);
    retval = fn->body(*this);
    if (except_flag == stack.get())
      ret();
  } else {
    push_str("Only functions and types can be called");
    except();
  }
}

void vm::machine::eblk()
{
  stack->local.emplace_back();
}

void vm::machine::lblk()
{
  stack->local.pop_back();
}

void vm::machine::ret()
{
  stack = stack->parent;
  if (!stack)
    exit(0);
  gc::set_current_frame(stack);
}

void vm::machine::jmp_false(int offset)
{
  if (!truthy(retval))
    stack->instr_ptr = stack->instr_ptr.remove_prefix(offset - 1);
}

void vm::machine::jmp(int offset)
{
  stack->instr_ptr = stack->instr_ptr.remove_prefix(offset - 1);
}

void vm::machine::push_catch()
{
  stack->catchers.push_back(retval);
}

void vm::machine::pop_catch()
{
  stack->catchers.pop_back();
}

void vm::machine::except()
{
  while (stack && !stack->catchers.size()) {
    stack = stack->parent;
  }
  if (!stack) {
    std::cerr << "caught exception: " << retval->value() << '\n';
    gc::empty();
    exit(0);
  }
  push_arg();
  retval = stack->catchers.back();
  call(1);
  pop_catch();
}

// }}}
