#include "custom_object.h"

using namespace il;

value::custom_object::custom_object(custom_type* type,
                                    const std::vector<il::symbol>& args,
                                    ast::expression* body,
                                    environment& outer_env)
{ }

value::custom_type* value::custom_object::type() const
{
  throw std::runtime_error{"not yet implemented"};
}

std::string value::custom_object::value() const
{
  throw std::runtime_error{"not yet implemented"};
}

value::base* value::custom_object::call(const std::vector<base*>& args)
{
  throw std::runtime_error{"not yet implemented"};
}
