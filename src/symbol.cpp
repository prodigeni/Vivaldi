#include "symbol.h"

std::unordered_set<std::string> vv::symbol::s_symbol_table{ };

vv::symbol::symbol(const std::string& str)
  : m_ptr {&*s_symbol_table.insert(str).first}
{ }

bool vv::operator==(vv::symbol first, vv::symbol second)
{
  return first.m_ptr == second.m_ptr;
}

bool vv::operator!=(vv::symbol first, vv::symbol second)
{
  return first.m_ptr != second.m_ptr;
}

const std::string& vv::to_string(symbol sym)
{
  return *sym.m_ptr;
}

size_t std::hash<vv::symbol>::operator()(const vv::symbol& sym) const
{
  // Use *simplest possible hash*
  return reinterpret_cast<size_t>(sym.m_ptr);
}
