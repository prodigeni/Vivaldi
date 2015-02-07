#ifndef IL_AST_LITERAL_H
#define IL_AST_LITERAL_H

#include "expression.h"

namespace il {

namespace ast {

namespace literal {

class boolean : public expression {
public:
  boolean(bool val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  bool m_val;
};

class floating_point : public expression {
public:
  floating_point(double val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  double m_val;
};

class integer : public expression {
public:
  integer(int val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  int m_val;
};

class nil : public expression {
public:
  std::vector<vm::command> generate() const override;
};

class string : public expression {
public:
  string(const std::string& val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  std::string m_val;
};

class symbol : public expression {
public:
  symbol(il::symbol val) : m_val{val} { }
  std::vector<vm::command> generate() const override;
private:
  il::symbol m_val;
};

}

}

}

#endif
