#include "symbol.h"

std::unordered_set<std::string> il::symbol::s_symbol_table{};

il::symbol::symbol(const std::string& str)
{
  if (!s_symbol_table.count(str))
    s_symbol_table.insert(str);
  m_ptr = &*s_symbol_table.find(str);
}

bool il::operator==(il::symbol first, il::symbol second)
{
  return first.m_ptr == second.m_ptr;
}

bool il::operator!=(il::symbol first, il::symbol second)
{
  return first.m_ptr != second.m_ptr;
}

const std::string& il::to_string(symbol sym)
{
  return *sym.m_ptr;
}

size_t std::hash<il::symbol>::operator()(const il::symbol& sym) const
{
  const static std::hash<const std::string*> str_hash{};
  return str_hash(sym.m_ptr);
}
