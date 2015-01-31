#ifndef IL_ENVIRONMENT_H
#define IL_ENVIRONMENT_H

#include "symbol.h"
#include "value.h"

#include <unordered_map>

namespace il {

class environment {
public:
  environment(const std::unordered_map<symbol, value::base*>& local = {});
  environment(environment& parent);

  bool is_defined(symbol name);
  value::base*& at(symbol name);
  value::base* assign(symbol name, value::base* val);

  void mark();

  ~environment();

private:
  std::unordered_map<symbol, value::base*> m_local_env;
  environment* m_parent;
  std::vector<environment*> m_children;
};

}

#endif
