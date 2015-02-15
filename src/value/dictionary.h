#ifndef VV_VALUE_DICTIONARY_H
#define VV_VALUE_DICTIONARY_H

#include "value.h"

namespace vv {

namespace value {

struct dictionary : public base {
public:
  dictionary(const std::unordered_map<base*, base*>& mems = {});

  std::string value() const override;
  void mark() override;

  std::unordered_map<base*, base*> val;
};

}

}

#endif
