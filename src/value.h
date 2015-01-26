#ifndef IL_VALUE_H
#define IL_VALUE_H

#include <vector>

namespace il {

namespace value {

class base {
public:
  virtual base* type() const = 0;
  virtual std::string value() const = 0;

  virtual ~base() { }

private:
  //std::vector<base*> m_public_members;
  //std::vector<base*> m_private_members;
};

class string;
class integer;
class floating_point;
class boolean;
class array;
class dictionary;
class function;
class builtin_function;
class type;
class symbol;
class iterator;
class range;
class nil;
class custom_type;

}

}

#endif
