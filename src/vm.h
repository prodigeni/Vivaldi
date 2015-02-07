#ifndef IL_VM_H
#define IL_VM_H

#include "vm/call_stack.h"

namespace il {

namespace vm {

class machine {
public:
  machine(std::shared_ptr<call_stack> base);

  void run();

private:
  void push_int(int val);
  void push_sym(symbol val);
  void push_bool(bool val);
  void push_str(const std::string& val);
  void push_flt(double val);
  void push_fn(vector_ref<command> val);

  void read(symbol sym);
  void write(symbol sym);
  void let(symbol sym);

  void push_self();
  void self();
  void push_arg();
  void pop_arg(symbol sym);
  void member(symbol sym);
  void call(int args);

  void enter();
  void leave();

  void jump_unless(int offset);
  void jump(int offset);

  std::shared_ptr<call_stack> m_stack;
  value::base* m_retval;

  std::shared_ptr<call_stack> m_base;
};

}

}

#endif
