#ifndef VV_AST_REQUIRE_H
#define VV_AST_REQUIRE_H

#include "expression.h"

namespace vv {

namespace ast {

class require : public expression {
public:
  require(const std::string& filename);

  std::vector<vm::command> generate() const override;

private:
  std::string m_filename;
};

}

}

#endif
