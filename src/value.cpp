#include "value.h"

#include "environment.h"
#include "gc.h"

using namespace il;

value::base* value::base::call(const std::vector<value::base*>& args)
{
  static il::symbol call{"call"};
  return call_method(call, args);
}

value::base* value::base::call_method(il::symbol method,
                                      const std::vector<value::base*>& args)
{
  environment env;
  auto method_val = gc::push_argument(type()->method(method, this, env));
  auto res = method_val->call(args);
  gc::pop_argument();
  return res;
}

void value::base::mark()
{
  m_marked = true;
}

bool value::base::marked() const
{
  return m_marked;
}

void value::base::unmark()
{
  m_marked = false;
}
