#include "gc.h"

#include "builtins.h"

using namespace il;

namespace {

std::vector<value::base*> g_args;

std::vector<value::base*> g_vals;

void mark()
{
  for (auto* i : g_args)
    i->mark();
  builtin::g_base_env.mark();
}

void sweep()
{
  for (auto*& i : g_vals) {
    if (i->marked()) {
      i->unmark();
    } else {
      delete i;
      i = nullptr;
    }
  }

  g_vals.erase(remove(begin(g_vals), end(g_vals), nullptr), end(g_vals));
}

}

value::base* gc::internal::emplace(value::base* val)
{
  if (g_vals.capacity() == g_vals.size()) {
    mark();
    sweep();
  }
  if (g_vals.capacity() == g_vals.size()) {
    g_vals.reserve(g_vals.capacity() * 2);
  }

  g_vals.push_back(val);
  return val;
}

value::base* gc::push_argument(value::base* arg)
{
  g_args.push_back(arg);
  return arg;
}

void gc::pop_argument()
{
  g_args.pop_back();
}

void gc::init()
{
  g_vals.reserve(512);
}

void gc::empty()
{
  for (auto i : g_vals)
    delete i;
  g_vals.clear();
}
