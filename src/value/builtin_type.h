#ifndef IL_VALUE_BUILTIN_TYPE_H
#define IL_VALUE_BUILTIN_TYPE_H

#include "value.h"
#include "expression.h"

namespace il {

namespace value {

class builtin_type : public basic_type {
public:
  builtin_type(
      const std::function<base*(const std::vector<base*>&)>& ctr,
      const std::unordered_map<
              il::symbol,
              std::function<base*(base*, const std::vector<base*>&)>>& methods,
      environment& env);

  void each_key(const std::function<void(il::symbol)>& fn) const override;
  base* method(il::symbol name, environment& env) const override;

  std::string value() const override;
  base* call(const std::vector<base*>& args) override;
  base* copy() const override;

private:
  std::function<base*(const std::vector<base*>&)> m_ctr;
  std::unordered_map<
    il::symbol,
    std::function<base*(base*, const std::vector<base*>&)>> m_methods;
};

}

}

#endif
