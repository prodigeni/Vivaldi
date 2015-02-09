#ifndef VV_AST_TRY_CATCH_H
#define VV_AST_TRY_CATCH_H

#include "expression.h"

namespace vv {

namespace ast {

class try_catch : public expression {
public:
  try_catch(std::unique_ptr<expression>&& body,
            std::unique_ptr<expression>&& catcher);

  std::vector<vm::command> generate() const override;

private:
  std::unique_ptr<expression> m_body;
  std::unique_ptr<expression> m_catcher;
};

}

}

#endif
