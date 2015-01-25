#ifndef IL_AST_FOR_LOOP_H
#define IL_AST_FOR_LOOP_H

#include "expression.h"

namespace il {

namespace ast {

class for_loop : public expression {
public:
  for_loop(symbol iterator,
           std::unique_ptr<expression>&& range,
           std::unique_ptr<expression>&& body);

  value::base* eval(environment& env) const override;

private:
  symbol m_iterator;
  std::unique_ptr<expression> m_range;
  std::unique_ptr<expression> m_body;
};

}

}

#endif
