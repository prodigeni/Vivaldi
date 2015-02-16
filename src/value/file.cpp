#include "value/file.h"

#include "builtins.h"
#include "gc.h"

using namespace vv;

value::file::file(const std::string& filename)
  : base {&builtin::type::file},
    name {filename},
    val  {filename}
{
  std::getline(val, cur_line);
}

value::file::file()
  : base {&builtin::type::file},
    name {""}
{ }

std::string value::file::value() const { return "File: " + name; }
