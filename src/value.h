#ifndef VV_VALUE_H
#define VV_VALUE_H

#include "symbol.h"

#include <unordered_map>
#include <vector>

namespace vv {

namespace ast {
class function_definition;
}

namespace vm {
class machine;
}

namespace value {

struct array;
struct array_iterator;
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
struct string_iterator;
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
  type(const std::function<value::base*(vm::machine&)>& constructor,
       const std::unordered_map<vv::symbol, value::base*>& methods,
       value::base& parent,
       vv::symbol name);

  std::unordered_map<vv::symbol, value::base*> methods;
  std::function<value::base*(vm::machine&)> constructor;

  value::base& parent;
  vv::symbol name;

  std::string value() const override;

  void mark() override;
};

}

}

#endif
