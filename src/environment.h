#ifndef IL_ENVIRONMENT_H
#define IL_ENVIRONMENT_H

#include "symbol.h"

#include <vector>
#include <unordered_map>

namespace il {

namespace value {

class base;

}

class environment {
public:
  environment(const std::unordered_map<symbol, value::base*>& local = {});
  environment(environment& parent);
  environment(environment&& other) = default;

  static environment close_on(environment& parent);

  bool is_defined(symbol name);
  value::base* at(symbol name);
  value::base* assign(symbol name, value::base* val);
  value::base* create(symbol name, value::base* val);

  environment* parent() const { return m_parent; }

  void mark();

  ~environment();

private:
  std::unordered_map<symbol, value::base*> m_local_env;
  environment* m_parent;
  std::vector<environment*> m_children;
};

}

#endif
