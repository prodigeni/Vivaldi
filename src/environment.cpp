#include "environment.h"

il::environment::environment(
    const std::unordered_map<symbol, value::base*>& local)
  : m_local_env {local},
    m_parent    {nullptr}
{ }

il::environment::environment(environment& prev) : m_parent{&prev} { }

il::value::base*& il::environment::at(symbol name)
{
  if (m_local_env.count(name))
    return m_local_env[name];
  if (m_parent)
    return m_parent->at(name);
  throw std::runtime_error{"symbol '" + to_string(name) + " undefined"};
}

il::value::base* il::environment::assign(symbol name, value::base* val)
{
  return m_local_env[name] = val;
}
