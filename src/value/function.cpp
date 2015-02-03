#include "function.h"

#include "builtins.h"
#include "gc.h"
#include "lang_utils.h"
#include "value/builtin_type.h"

using namespace il;

value::function::function(const std::vector<il::symbol>& args,
                          std::shared_ptr<ast::expression> body,
                          environment& outer_env)
  : base   {&builtin::type::function, outer_env},
    m_args {args},
    m_body {body}
{ }

std::string value::function::value() const
{
  return "<function>";
}

value::base* value::function::call(const std::vector<base*>& args)
{
  check_size(m_args.size(), args.size());

  environment call_env{*env().parent()};
  for (auto sz = args.size(); sz--;)
    call_env.assign(m_args[sz], args[sz]);

  return m_body->eval(call_env);
}

value::base* value::function::copy() const
{
  return gc::alloc<function>( m_args, m_body, *env().parent() );
}

void value::function::mark()
{
  base::mark();
  env().mark();
}
