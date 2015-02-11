#ifndef VV_AST_LOGICAL_AND_H
#define VV_AST_LOGICAL_AND_H

#include "expression.h"

namespace vv {

namespace ast {

class logical_and : public expression {
public:
  logical_and(std::unique_ptr<expression>&& left,
              std::unique_ptr<expression>&& right);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<expression> m_left;
  std::unique_ptr<expression> m_right;
};

}

}

#endif
