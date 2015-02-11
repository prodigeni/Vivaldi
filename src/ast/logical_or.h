#ifndef VV_AST_LOGICAL_OR_H
#define VV_AST_LOGICAL_OR_H

#include "expression.h"

namespace vv {

namespace ast {

class logical_or : public expression {
public:
  logical_or(std::unique_ptr<expression>&& left,
             std::unique_ptr<expression>&& right);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<expression> m_left;
  std::unique_ptr<expression> m_right;
};

}

}

#endif
