#ifndef VV_VALUE_H
#define VV_VALUE_H

#include "symbol.h"
#include "vm/instruction.h"

#include <unordered_map>
#include <vector>

namespace vv {

// Pre-declarations for headers dependent on this one
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
  base();

  // Used in REPL, and in 'puts' and 'print' for non-String types
  virtual std::string value() const { return "<object>"; }

  virtual ~base() { }

  std::unordered_map<vv::symbol, value::base*> members;
  type* type;

  virtual size_t hash() const;
  virtual bool equals(const value::base& other) const;

  virtual void mark();
  bool marked() const { return m_marked; }
  void unmark() { m_marked = false; }

private:
  bool m_marked;
};

struct type : public base {
  type(const std::function<value::base*()>& constructor,
       const std::unordered_map<vv::symbol, value::base*>& methods,
       value::base& parent,
       vv::symbol name);

  std::unordered_map<vv::symbol, value::base*> methods;
  std::function<value::base*()> constructor;
  // This shim is necessary because, of course, when you create a new object you
  // want to get that object back. Unfortunately it's not possible to guarantee
  // this in the init function, since someone could do something like
  //   class Foo
  //     fn init(): 5
  //   end
  // When you instantiate Foo, you'd prefer you get your new object back, not 5.
  // So the init shim creates a function that looks something like
  //   fn (<args...): do
  //     self.init(<args...>)
  //     self
  //   end
  // and the constructor calls that fake init function instead of 'init'
  vm::function_t init_shim;

  value::base& parent;
  // Stored in class so value() can be prettier than just <type>
  vv::symbol name;

  std::string value() const override;

  void mark() override;
};

}

}

template <>
struct std::hash<vv::value::base*> {
  size_t operator()(const vv::value::base* b) const;
};

template <>
struct std::equal_to<vv::value::base*> {
  bool operator()(const vv::value::base* left, const vv::value::base* right) const;
};

#endif
