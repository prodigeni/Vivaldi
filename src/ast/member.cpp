#include "member.h"

#include "gc.h"

using namespace il;

ast::member::member(std::unique_ptr<ast::expression>&& object, il::symbol name)
  : m_object {move(object)},
    m_name   {name}
{ }

value::base* ast::member::eval(environment& env) const
{
  auto object = gc::push_argument(m_object->eval(env));
  auto member = object->member(m_name);
  gc::pop_argument();
  return member;
}
