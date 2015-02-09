#ifndef VV_SYMBOL_H
#define VV_SYMBOL_H

#include <string>
#include <unordered_set>

namespace vv {

class symbol {
public:
  symbol(const std::string& str);

  friend bool operator==(symbol first, symbol second);
  friend bool operator!=(symbol first, symbol second);

  friend const std::string& to_string(symbol sym);

private:
  const std::string* m_ptr;
  static std::unordered_set<std::string> s_symbol_table;

  friend struct std::hash<vv::symbol>;
};

}

template <>
struct std::hash<vv::symbol> {
  size_t operator()(const vv::symbol& sym) const;
};

#endif
