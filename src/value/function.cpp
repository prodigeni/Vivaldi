#include "function.h"

#include "builtins.h"
#include "gc.h"
#include "lang_utils.h"
#include "value/builtin_type.h"

using namespace il;

value::function::function(const std::vector<il::symbol>& args,
                          std::shared_ptr<ast::expression> body)
  : base   {&builtin::type::function},
    m_args {args},
    m_body {body}
{ }

std::string value::function::value() const
{
  return "<function>";
}
