#ifndef IL_VALUE_H
#define IL_VALUE_H

#include "symbol.h"

#include <vector>

namespace il {

class environment;

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
class basic_type;
class custom_type;
class builtin_type;
class custom_object;

class base {
public:

  virtual basic_type* type() const = 0;
  virtual std::string value() const = 0;

  virtual base* call(const std::vector<value::base*>& args);
  virtual base* call_method(il::symbol method,
                            const std::vector<value::base*>& args);

  virtual base* copy() const = 0;

  virtual ~base() { }
};

class basic_type : public base {
public:
  virtual value::base* method(il::symbol name,
                              value::base* self,
                              environment& env) const = 0;
};

}

}

#endif
