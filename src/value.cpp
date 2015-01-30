#include "value.h"

#include "environment.h"

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
  return type()->method(method, this, env)->call(args);
}
