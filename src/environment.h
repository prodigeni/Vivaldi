#ifndef IL_ENVIRONMENT_H
#define IL_ENVIRONMENT_H

#include "symbol.h"
#include "value.h"

#include <unordered_map>

namespace il {

class environment {
public:
  environment();
  environment(environment& parent);

  bool is_defined(symbol name);
  value::base*& at(symbol name);

private:
  std::unordered_map<symbol, value::base*> m_local_env;
  environment* m_parent;
};

}

#endif
