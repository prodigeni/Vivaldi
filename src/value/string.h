#ifndef IL_VALUE_STRING_H
#define IL_VALUE_STRING_H

#include "value.h"

#include <string>

namespace il {

namespace value {

struct string : public base {
public:
  string(const std::string& val);

  std::string value() const override;

  std::string val;
};

}

}

#endif
