#ifndef IL_VALUE_H
#define IL_VALUE_H

#include "symbol.h"

#include <vector>

namespace il {

namespace value {

class string;
class integer;
class floating_point;
class boolean;
class array;
class dictionary;
class function;
class builtin_function;
class symbol;
class iterator;
class range;
class nil;
class custom_type;
class custom_object;

class base {
public:

  virtual custom_type* type() const = 0;
  virtual std::string value() const = 0;

  virtual base* call(const std::vector<value::base*>& args);
  virtual base* call_method(il::symbol method,
                            const std::vector<value::base*>& args);

  virtual base* copy() const = 0;

  virtual ~base() { }

private:
  //std::vector<base*> m_public_members;
  //std::vector<base*> m_private_members;
};

}

}

#endif
