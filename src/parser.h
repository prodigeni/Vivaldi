#ifndef VV_PARSER_H
#define VV_PARSER_H

#include "expression.h"
#include "utils.h"

#include <boost/optional/optional.hpp>

#include <istream>
#include <string>
#include <vector>

namespace vv {

namespace parser {

class val_res {
public:
  val_res(vector_ref<std::string> token) : m_tokens{token} { }
  val_res() { }
  val_res(vector_ref<std::string> where, const std::string& what)
    : m_tokens {where},
      m_error  {what}
  { }

  const std::string& error() const { return *m_error; }

  auto operator*() const { return *m_tokens; }
  auto* operator->() const { return &*m_tokens; }

  operator bool() const { return m_tokens && !m_error; }

  bool invalid() const { return m_error.operator bool(); }
  bool valid() const { return !m_error; }

private:
  boost::optional<vector_ref<std::string>> m_tokens;
  boost::optional<std::string> m_error;
};

using token_string = vector_ref<std::string>;

std::vector<std::string> tokenize(std::istream& input);

val_res is_valid(token_string tokens);

std::vector<std::unique_ptr<ast::expression>> parse(token_string tokens);

}

}

#endif
