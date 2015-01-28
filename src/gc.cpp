#include "gc.h"

using namespace il;

namespace {

std::vector<value::base*> g_vals;

}

value::base* gc::internal::emplace(value::base* val)
{
  g_vals.push_back(val);
  return val;
}
