#ifndef VV_AST_EXCEPT_H
#define VV_AST_EXCEPT_H

#include "expression.h"

namespace vv {

namespace ast {

class except : public expression {
public:
  except(std::unique_ptr<expression>&& value);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<expression> m_value;
};

}

}

#endif
