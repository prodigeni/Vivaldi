#include "require.h"

#include "vm/instruction.h"

using namespace vv;

ast::require::require(const std::string& filename)
  : m_filename {filename}
{ }

std::vector<vm::command> ast::require::generate() const
{
  return std::vector<vm::command>{ {vm::instruction::req, m_filename} };
}
