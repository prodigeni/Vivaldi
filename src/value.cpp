#include "value.h"

#include "builtins.h"
#include "gc.h"
#include "ast/function_definition.h"
#include "value/builtin_type.h"
#include "value/function.h"
#include "environment.h"

using namespace il;

value::base::base(basic_type* type, environment& env)
  : m_marked {false},
    m_type   {type},
    m_env    {std::move(environment::close_on(env))}
{
  m_env.assign(builtin::sym::self, this);
  if (m_type) {
    m_type->each_key([&](auto sym)
    {
      m_members[sym] = gc::push_argument(m_type->method(sym, m_env));
      m_members[sym]->set_owner(this);
    });
    m_type->each_key([&](auto) { gc::pop_argument(); });
  }
}

value::base* value::base::call(const std::vector<value::base*>& args)
{
  return member(builtin::sym::call)->call(args);
}

value::base*& value::base::member(il::symbol name)
{
  return m_members.at(name);
}

void value::base::mark()
{
  m_marked = true;
  m_env.mark();
  if (m_type && !m_type->marked())
    m_type->mark();
  for (auto& i : m_members) {
    if (!i.second->marked())
      i.second->mark();
  }
  if (m_owner && !m_owner->marked())
    m_owner->mark();
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
