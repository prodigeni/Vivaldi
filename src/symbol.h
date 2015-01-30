#ifndef IL_SYMBOL_H
#define IL_SYMBOL_H

#include <string>
#include <unordered_set>

namespace il {

class symbol {
public:
  symbol(std::string str);

  friend bool operator==(symbol first, symbol second);
  friend bool operator!=(symbol first, symbol second);

  friend const std::string& to_string(symbol sym);

private:
  const std::string* m_ptr;
  static std::unordered_set<std::string> s_symbol_table;

  friend struct std::hash<il::symbol>;
};

}

template <>
struct std::hash<il::symbol> {
  size_t operator()(const il::symbol& sym) const;
};

#endif
