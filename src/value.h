#ifndef VV_VALUE_H
#define VV_VALUE_H

#include "symbol.h"

#include <unordered_map>
#include <vector>

namespace vv {

namespace ast {
class function_definition;
}

namespace value {

struct array;
struct type;
struct boolean;
struct builtin_function;
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
  base(type* type);

  virtual std::string value() const { return "<object>"; }

  virtual ~base() { }

  std::unordered_map<vv::symbol, value::base*> members;
  type* type;

  virtual void mark();
  bool marked() const { return m_marked; }
  void unmark() { m_marked = false; }

private:
  bool m_marked;
};

struct type : public base {
  type(value::base* constructor,
      const std::unordered_map<vv::symbol, value::base*>& methods,
      vv::symbol name);

  std::unordered_map<vv::symbol, value::base*> methods;
  value::base* constructor;
  vv::symbol name;

  std::string value() const override;

  void mark() override;
};

}

}

#endif
