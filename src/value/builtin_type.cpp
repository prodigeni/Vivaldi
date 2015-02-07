#include "builtin_type.h"

#include "builtins.h"
#include "gc.h"
#include "value/builtin_function.h"

using namespace il;
using namespace value;

namespace {

}

builtin_type::builtin_type(
    const std::function<base*(const std::vector<base*>&)>& ctr,
    const std::unordered_map<
            il::symbol,
            std::function<base*(base*, const std::vector<base*>&)>>& fns)
  : basic_type {},
    m_ctr      {ctr},
    m_methods  {fns}
{ }

void builtin_type::each_key(
    const std::function<void(il::symbol)>& fn) const
{
  for (const auto& i : m_methods)
    fn(i.first);
}

std::string builtin_type::value() const
{
  return "<builtin type>";
}
