#include "value.h"

#include "builtins.h"
#include "gc.h"
#include "ast/function_definition.h"
#include "value/builtin_type.h"
#include "value/function.h"

using namespace il;

value::base::base(basic_type* type, environment& env)
  : m_type {type},
    m_env  {env}
{
  m_env.assign({"self"}, this);
  if (m_type)
    m_type->each_key([&](auto sym)
                        { m_members[sym] = m_type->method(sym, m_env); });
}

value::base* value::base::call(const std::vector<value::base*>& args)
{
  static il::symbol call{"call"};
  return member(call)->call(args);
}

value::base*& value::base::member(il::symbol name)
{
  return m_members[name];
}

void value::base::mark()
{
  m_marked = true;
  m_env.mark();
  if (m_type && !m_type->marked())
    m_type->mark();
  for (auto& i : m_members)
    if (!i.second->marked())
      i.second->mark();
}

bool value::base::marked() const
{
  return m_marked;
}

void value::base::unmark()
{
  m_marked = false;
}

value::basic_type::basic_type(environment& env)
  //: base {&builtin::type::custom_type, env}
  : base {nullptr, env}
{ }
