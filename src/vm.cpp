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

vm::machine::machine(std::shared_ptr<call_stack> frame,
                     const std::function<void(vm::machine&)>& exception_handler)
  : stack               {frame},
    retval              {nullptr},
    m_base              {frame},
    m_exception_handler {exception_handler}
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
    stack->instr_ptr = stack->instr_ptr.subvec(1);

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
    case instruction::argc:     argc(get<int>(arg));       break;
    case instruction::readm:    readm(get<symbol>(arg));   break;
    case instruction::writem:   writem(get<symbol>(arg));  break;
    case instruction::call:     call(get<int>(arg));       break;

    case instruction::eblk: eblk(); break;
    case instruction::lblk: lblk(); break;
    case instruction::ret:  ret();  break;

    case instruction::push: push(); break;
    case instruction::pop:  pop();  break;

    case instruction::jmp:       jmp(get<int>(arg));       break;
    case instruction::jmp_false: jmp_false(get<int>(arg)); break;
    case instruction::jmp_true:  jmp_true(get<int>(arg));  break;

    case instruction::push_catch: push_catch();            break;
    case instruction::pop_catch:  pop_catch();              break;
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
  push_str("no such variable: " + to_string(sym));
  except();
}

void vm::machine::write(symbol sym)
{
  auto cur_stack = stack;
  while (cur_stack) {
    auto holder = find_if(rbegin(cur_stack->local), rend(cur_stack->local),
                          [&](const auto& vars) { return vars.count(sym); });
    if (holder != rend(cur_stack->local)) {
      holder->at(sym) = retval;
      return;
    }
    cur_stack = cur_stack->enclosing;
  }
  push_str("no such variable: " + to_string(sym));
  except();
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
  stack->pushed.push_back(retval);
}

void vm::machine::pop_arg(symbol sym)
{
  stack->local.back()[sym] = stack->parent->pushed.back();
  stack->parent->pushed.pop_back();
}

void vm::machine::argc(int count)
{
  if (stack->args != static_cast<size_t>(count)) {
    push_str("Wrong number of arguments--- expected "
            + std::to_string(stack->args) + ", got "
            + std::to_string(count));
    except();
  }
}

void vm::machine::readm(symbol sym)
{
  stack->pushed_self = *retval;
  if (retval->members.count(sym)) {
    retval = retval->members[sym];
    return;
  }

  auto member = find_method(retval->type, sym);
  if (member) {
    retval = member;
    return;
  }
  push_str("no such member: " + to_string(sym));
  except();
}

void vm::machine::writem(symbol sym)
{
  auto value = stack->pushed.back();
  stack->pushed.pop_back();

  retval->members[sym] = value;
  retval = value;
}

// TODO: make suck less.
void vm::machine::call(int argc)
{
  std::vector<value::base*> args;

  auto function = retval;
  if (auto type = dynamic_cast<value::type*>(function)) {
    auto init = find_method(type, {"init"});
    stack = std::make_shared<call_stack>( stack,
                                          m_base,
                                          init ? 0 : argc, // save args for init
                                          stack->instr_ptr );
    stack->caller = *function;

    auto except_flag = stack.get();
    gc::set_current_frame(stack);
    retval = type->constructor(*this);

    if (except_flag == stack.get()) {
      ret();
      if (init) {
        // HACK--- since we need to return self, not whatever init returns,
        // during construction, evaluate it in a separate VM
        stack->pushed_self = *retval;
        auto real_instr_ptr = stack->instr_ptr;

        std::vector<command> instruction{ {instruction::call, argc} };
        vm::machine initializer{stack, m_exception_handler};
        initializer.retval = init;
        initializer.stack->instr_ptr = instruction;
        initializer.run();

        stack->instr_ptr = real_instr_ptr;
      }
    }

  } else if (auto fn = dynamic_cast<value::function*>(function)) {
    stack = std::make_shared<call_stack>(stack, fn->enclosure, argc, fn->body);
    stack->caller = *function;

    gc::set_current_frame(stack);

  } else if (auto fn = dynamic_cast<value::builtin_function*>(function)) {
    stack = std::make_shared<call_stack>(stack, m_base, argc, stack->instr_ptr);
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
  if (stack->parent) {
    stack = stack->parent;
    gc::set_current_frame(stack);
  } else {
    push_str("The top-level environment can't be returned from");
    except();
  }
}

void vm::machine::push()
{
  stack->pushed.push_back(retval);
}

void vm::machine::pop()
{
  retval = stack->pushed.back();
  stack->pushed.pop_back();
}

void vm::machine::jmp(int offset)
{
  stack->instr_ptr = stack->instr_ptr.shifted_by(offset - 1);
}

void vm::machine::jmp_false(int offset)
{
  if (!truthy(retval))
    jmp(offset);
}

void vm::machine::jmp_true(int offset)
{
  if (truthy(retval))
    jmp(offset);
}

void vm::machine::push_catch()
{
  stack->catcher = *retval;
}

void vm::machine::pop_catch()
{
  stack->catcher = {};
}

void vm::machine::except()
{
  while (stack->parent && !stack->catcher)
    stack = stack->parent;

  if (!stack->catcher) {
    m_exception_handler(*this);
  } else {
    push_arg();
    retval = &*stack->catcher;
    call(1);
    pop_catch();
  }
}

// }}}
