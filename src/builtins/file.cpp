#include "builtins.h"

#include "gc.h"
#include "lang_utils.h"
#include "vm.h"
#include "value/builtin_function.h"
#include "value/file.h"
#include "value/string.h"

#include <sstream>

using namespace vv;
using namespace builtin;

namespace {

value::base* fn_file_init(vm::machine& vm)
{
  auto arg = get_arg(vm, 0);
  if (arg->type != &type::string)
    return throw_exception("Files can only be constructed from Strings", vm);
  auto& self = static_cast<value::file&>(*vm.frame->self);
  const auto& filename = static_cast<value::string*>(arg)->val;
  self.val = std::fstream{filename};
  self.name = filename;
  std::getline(self.val, self.cur_line);
  return &self;
}

value::base* fn_file_contents(vm::machine& vm)
{
  auto& self = static_cast<value::file&>(*vm.frame->self);
  std::ostringstream str_stream;
  str_stream << self.cur_line;
  str_stream << self.val.rdbuf();
  self.cur_line.clear();
  return gc::alloc<value::string>( str_stream.str() );
}

value::base* fn_file_start(vm::machine& vm)
{
  return &*vm.frame->self;
}

value::base* fn_file_get(vm::machine& vm)
{
  const auto& self = static_cast<value::file&>(*vm.frame->self);
  return gc::alloc<value::string>( self.cur_line );
}

value::base* fn_file_increment(vm::machine& vm)
{
  auto& self = static_cast<value::file&>(*vm.frame->self);
  if (self.val.peek() == EOF)
    return throw_exception("Cannot read past end of File", vm);
  std::getline(self.val, self.cur_line);
  return &self;
}

value::base* fn_file_at_end(vm::machine& vm)
{
  auto& self = static_cast<value::file&>(*vm.frame->self);
  return gc::alloc<value::boolean>(self.val.peek() == EOF && !self.cur_line.size());
}

value::builtin_function file_init      {fn_file_init,      1};
value::builtin_function file_contents  {fn_file_contents,  0};
value::builtin_function file_start     {fn_file_start,     0};
value::builtin_function file_get       {fn_file_get,       0};
value::builtin_function file_increment {fn_file_increment, 0};
value::builtin_function file_at_end    {fn_file_at_end,    0};

}

vv::value::type vv::builtin::type::file {gc::alloc<value::file>, {
  { {"init"},      &file_init      },
  { {"contents"},  &file_contents  },
  { {"start"},     &file_start     },
  { {"get"},       &file_get       },
  { {"increment"}, &file_increment },
  { {"at_end"},    &file_at_end    }
}, vv::builtin::type::object, {"File"}};
