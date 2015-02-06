#include "environment.h"

#include "value.h"

il::environment::environment(
    const std::unordered_map<symbol, value::base*>& local)
  : m_local_env {local},
    m_parent    {nullptr}
{ }

il::environment::environment(environment& prev)
  : m_parent {&prev}
{
  m_parent->m_children.push_back(this);
}

il::environment il::environment::close_on(environment& parent)
{
  environment env{{}};
  env.m_parent = &parent;
  return env;
}

bool il::environment::is_defined(symbol name)
{
  if (m_local_env.count(name))
    return true;
  if (!m_parent)
    return false;
  return m_parent->is_defined(name);
}

il::value::base* il::environment::at(symbol name)
{
  if (m_local_env.count(name))
    return m_local_env[name];
  if (m_parent)
    return m_parent->at(name);
  throw std::runtime_error{"symbol '" + to_string(name) + " undefined"};
}

il::value::base* il::environment::assign(symbol name, value::base* val)
{
  if (m_local_env.count(name)) {
    m_local_env.erase(name);
    return m_local_env[name] = val;
  }
  if (m_parent)
    return m_parent->assign(name, val);
  throw std::runtime_error{"symbol '" + to_string(name) + " undefined"};
}

il::value::base* il::environment::create(symbol name, value::base* val)
{
  return m_local_env[name] = val;
}

void il::environment::mark()
{
  for (const auto& i : m_local_env)
    if (!i.second->marked())
      i.second->mark();
  for (auto* i : m_children)
    i->mark();
}

il::environment::~environment()
{
  if (m_parent) {
    auto& children = m_parent->m_children;
    auto parent_ptr = find(begin(children), end(children), this);
    if (parent_ptr != end(children))
      children.erase(parent_ptr);
  }
}
