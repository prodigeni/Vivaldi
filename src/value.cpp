#include "value.h"

#include "builtins.h"
#include "gc.h"
#include "lang_utils.h"
#include "ast/function_definition.h"
#include "value/builtin_function.h"
#include "value/function.h"

using namespace vv;

value::base::base(struct type* new_type)
  : members  {},
    type     {new_type},
    m_marked {false}
{ }

value::base::base()
  : members  {},
    type     {&builtin::type::object},
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
    const std::function<value::base*()>& new_constructor,
    const std::unordered_map<vv::symbol, value::base*>& new_methods,
    value::base& new_parent,
    vv::symbol new_name)
  : base        {&builtin::type::custom_type},
    methods     {new_methods},
    constructor {new_constructor},
    parent      {new_parent},
    name        {new_name}
{
  if (auto init = find_method(this, {"init"})) {
    // TODO: make function and builtin_function both inherit from a
    // basic_function class, so I can quit it with all these dynamic_casts.
    if (auto fn = dynamic_cast<builtin_function*>(init))
      init_shim.argc = fn->argc;
    else
      init_shim.argc = static_cast<function*>(init)->argc;

    for (auto i = 0; i != init_shim.argc; ++i) {
      init_shim.body.emplace_back(vm::instruction::arg, i);
      init_shim.body.emplace_back(vm::instruction::push);
    }

    init_shim.body.emplace_back( vm::instruction::self );
    init_shim.body.emplace_back( vm::instruction::readm, vv::symbol{"init"} );
    init_shim.body.emplace_back( vm::instruction::call, init_shim.argc );
    init_shim.body.emplace_back( vm::instruction::self );
    init_shim.body.emplace_back( vm::instruction::ret );

  } else {
    init_shim.argc = 0;
    init_shim.body = {
      { vm::instruction::self },
      { vm::instruction::ret }
    };
  }
}

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
