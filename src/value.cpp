#include "value.h"

#include "builtins.h"
#include "gc.h"
#include "ast/function_definition.h"
#include "value/function.h"

using namespace il;

value::base::base(struct type* new_type)
  : type     {new_type},
    m_marked {false}
{
  if (type)
    members = type->methods;
}

void value::base::mark()
{
  m_marked = true;
  for (auto& i : members)
    if (!i.second->marked())
      i.second->mark();
}

value::type::type(
    value::base* new_ctr,
    const std::unordered_map<il::symbol, value::base*>& new_methods)
  : base        {nullptr},
    methods     {new_methods},
    constructor {new_ctr}
{ }

std::string value::type::value() const { return "<type>"; }

void value::type::mark()
{
  base::mark();
  if (constructor && !constructor->marked())
    constructor->mark();
}
