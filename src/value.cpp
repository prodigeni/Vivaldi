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
    value::base* new_ctr,
    const std::unordered_map<vv::symbol, value::base*>& new_methods)
  : base        {&builtin::type::custom_type},
    methods     {new_methods},
    constructor {new_ctr}
{ }

std::string value::type::value() const { return "<type>"; }

void value::type::mark()
{
  base::mark();
  if (constructor && !constructor->marked())
    constructor->mark();
  for (const auto& i : methods)
    if (!i.second->marked())
      i.second->mark();
}
