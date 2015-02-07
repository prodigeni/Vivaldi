#ifndef IL_VALUE_BUILTIN_TYPE_H
#define IL_VALUE_BUILTIN_TYPE_H

#include "value.h"
#include "vm/call_stack.h"

#include <functional>

namespace il {

namespace value {

struct builtin_type : public basic_type {
public:
  builtin_type(
      const std::function<base*(vm::call_stack&)>& ctr,
      const std::unordered_map<
              il::symbol,
              value::builtin_function> fns);

  void each_key(const std::function<void(il::symbol)>& fn) const override;

  value::base* method(il::symbol name) override;

  std::string value() const override;

private:
  std::function<base*(vm::call_stack&)> m_ctr;
  std::unordered_map<
    il::symbol,
    value::builtin_function> m_methods;
};

}

}

#endif
