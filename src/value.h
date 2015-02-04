#ifndef IL_VALUE_H
#define IL_VALUE_H

#include "symbol.h"
#include "environment.h"

#include <unordered_map>
#include <vector>

namespace il {

namespace ast {
class function_definition;
}

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
  base(basic_type* type, environment& env);

  basic_type* type() const { return m_type; }
  environment& env() { return m_env; }
  const environment& env() const { return m_env; }

  virtual std::string value() const = 0;
  virtual base* call(const std::vector<value::base*>& args);
  value::base*& member(il::symbol name);

  virtual base* copy() const = 0;

  void set_owner(base* owner) { m_owner = owner; }

  virtual void mark();
  bool marked() const;
  void unmark();

  virtual ~base() { }

private:
  std::unordered_map<il::symbol, value::base*> m_members;
  bool m_marked;
  basic_type* m_type;
  environment m_env;

  base* m_owner;
};

class basic_type : public base {
public:
  basic_type(environment& env);

  virtual void each_key(const std::function<void(il::symbol)>& fn) const = 0;
  virtual base* method(il::symbol name, environment& env) const = 0;
};

}

}

#endif
