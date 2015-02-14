#include "type_definition.h"

#include "gc.h"
#include "vm/instruction.h"

#include <boost/variant/get.hpp>

using namespace vv;

ast::type_definition::type_definition(
    symbol name,
    symbol parent,
    std::unordered_map<vv::symbol, ast::function_definition>&& methods)

  : m_name    {name},
    m_parent  {parent},
    m_methods {move(methods)}
{ }

std::vector<vm::command> ast::type_definition::generate() const
{
  std::unordered_map<symbol, vm::function_t> methods;
  for (const auto& i : m_methods) {
    auto arg = i.second.generate().front().arg;
    methods[i.first] = boost::get<vm::function_t>(arg);
  }

  std::vector<vm::command> vec;
  vec.emplace_back(vm::instruction::push_type,
                   vm::type_t{m_name, m_parent, methods});
  return vec;
}
