#include "gc.h"

#include "builtins.h"

using namespace il;

namespace {

std::shared_ptr<vm::call_stack> g_frame{nullptr};
std::vector<value::base*> g_vals;

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
    mark(*g_frame);
    sweep();
  }
  if (g_vals.capacity() == g_vals.size()) {
    g_vals.reserve(g_vals.capacity() * 2);
  }

  val->unmark();
  g_vals.push_back(val);
  return val;
}

void gc::set_current_frame(std::shared_ptr<vm::call_stack> frame)
{
  g_frame = frame;
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
