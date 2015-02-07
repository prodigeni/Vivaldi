#ifndef IL_VALUE_H
#define IL_VALUE_H

#include "symbol.h"

#include <unordered_map>
#include <vector>

namespace il {

namespace ast {
class function_definition;
}

namespace value {

struct array;
struct basic_type;
struct boolean;
struct builtin_function;
struct builtin_type;
struct custom_object;
struct custom_type;
struct dictionary;
struct floating_point;
struct function;
struct integer;
struct iterator;
struct nil;
struct range;
struct string;
struct symbol;

struct base {
  base(basic_type* type);

  virtual std::string value() const = 0;

  virtual ~base() { }

  std::unordered_map<il::symbol, value::base*> members;
  basic_type* type;

  virtual void mark();
  bool marked() const { return m_marked; }
  void unmark() { m_marked = false; }

private:
  bool m_marked;
};

struct basic_type : public base {
  basic_type();

  virtual void each_key(const std::function<void(il::symbol)>& fn) const = 0;
  virtual base* method(il::symbol name) const = 0;
};

}

}

#endif
