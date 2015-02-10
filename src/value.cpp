#include "value.h"

#include "builtins.h"
#include "gc.h"
#include "ast/function_definition.h"
#include "value/function.h"

using namespace vv;

value::base::base(struct type* new_type)
  : members  {},
    type     {new_type},
    m_marked {false}
{ }

void value::base::mark()
{
  m_marked = true;
  if (type && !type->marked())
    type->mark();
  for (auto& i : members)
    if (!i.second->marked())
      i.second->mark();
}

value::type::type(
    const std::function<value::base*(vm::machine&)>& new_constructor,
    const std::unordered_map<vv::symbol, value::base*>& new_methods,
    value::base& new_parent,
    vv::symbol new_name)
  : base        {&builtin::type::custom_type},
    methods     {new_methods},
    constructor {new_constructor},
    parent      {new_parent},
    name        {new_name}
{ }

std::string value::type::value() const { return to_string(name); }

void value::type::mark()
{
  base::mark();
  for (const auto& i : methods)
    if (!i.second->marked())
      i.second->mark();
  if (!parent.marked())
    parent.mark();
}
