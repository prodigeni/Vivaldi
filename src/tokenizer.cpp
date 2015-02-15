#include "parser.h"

using namespace vv;

/**
 * Available tokens:
 * '{', '}'
 * '[', ']'
 * '(', ')'
 * '.'
 * ','
 * ':'
 * '='
 * '=='
 * '+', '-'
 * '*', '/', '%'
 * '!', '~'
 * '^', '&', '|'
 * '&&', '||'
 * '<', '>', '<=', '>='
 * '''
 * (strings)
 * (names)
 * (numbers)
 * (hexadecimal numbers)
 * (binary numbers)
 * (octal numbers)
 * (newline)
 */

namespace {

using tok_res = std::pair<std::string, boost::string_ref>;

// Individual tokenizing functions {{{

// '0' {{{

tok_res zero_token(boost::string_ref line)
{
  auto last = begin(line) + 1;
  if (line.size() > 1) {
    if (line[1] == '.') {
      auto post_dot = std::find_if_not(begin(line) + 2, end(line), isdigit);
      if (post_dot != begin(line) + 2)
        last = post_dot;

    } else if (line[1] == 'x') {
      last = std::find_if(begin(line) + 2, end(line),
                         [](auto c) { return !isdigit(c) || c < 'a' || c > 'f'; });

    } else if (line[1] == 'b') {
      last = std::find_if(begin(line) + 2, end(line),
                          [](auto c) { return c != '0' && c != '1'; });

    } else if (isdigit(line[1])) {
      last = std::find_if(begin(line) + 2, end(line),
                          [](auto c)
                            { return !isdigit(c) || c == '8' || c == '9'; });
    }
  }
  std::string num{begin(line), last};
  return { num, ltrim(line.substr(last - begin(line))) };
}

// }}}
// '1'-'9' {{{

tok_res digit_token(boost::string_ref line)
{
  auto nondigit = std::find_if_not(begin(line), end(line), isdigit);
  if (nondigit != end(line) && *nondigit == '.') {
    auto nonfloat = std::find_if_not(nondigit + 1, end(line), isdigit);
    if (nonfloat != nondigit + 1)
      nondigit = nonfloat;
  }

  return { {begin(line), nondigit}, ltrim(line.substr(nondigit - begin(line)))};
}

// }}}
// '=' {{{

tok_res eq_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '=')
    return {"=", ltrim(line.substr(1))};
  return {"==", ltrim(line.substr(2))};
}

// }}}
// '!' {{{

tok_res bang_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '=')
    return {"!", ltrim(line.substr(1))};
  return {"!=", ltrim(line.substr(2))};
}

// }}}
// '*' {{{

tok_res star_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '*')
    return {"*", ltrim(line.substr(1))};
  return {"**", ltrim(line.substr(2))};
}

// }}}
// '&' {{{

tok_res and_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '&')
    return {"&", ltrim(line.substr(1))};
  return {"&&", ltrim(line.substr(2))};
}

// }}}
// '|' {{{

tok_res or_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '|')
    return {"|", ltrim(line.substr(1))};
  return {"||", ltrim(line.substr(2))};
}

// }}}
// '<' {{{

tok_res lt_tokens(boost::string_ref line)
{
  if (line.size() > 1 && line[1] == '=')
    return {"<=", ltrim(line.substr(2))};
  if (line.size() > 1 && line[1] == '<')
    return {"<<", ltrim(line.substr(2))};
  return {"<", ltrim(line.substr(1))};
}

// }}}
// '>' {{{

tok_res gt_tokens(boost::string_ref line)
{
  if (line.size() > 1 && line[1] == '=')
    return {">=", ltrim(line.substr(2))};
  if (line.size() > 1 && line[1] == '>')
    return {">>", ltrim(line.substr(2))};
  return {">", ltrim(line.substr(1))};
}

// }}}
// '"' {{{

char escaped(char nonescaped) {
  switch (nonescaped) {
  case 'a':  return '\a';
  case 'b':  return '\b';
  case 'n':  return '\n';
  case 'f':  return '\f';
  case 'r':  return '\r';
  case 't':  return '\t';
  case 'v':  return '\v';
  case '"':  return '"';
  case '\\': return '\\';
  case '0':  return '\0';
  default:   return nonescaped;
  }
}

tok_res string_token(boost::string_ref line)
{
  std::string token{'"'};
  line.remove_prefix(1);
  while (line.front() != '"') {
    if (line.front() == '\\') {
      line.remove_prefix(1);
      token += escaped(line.front());
    } else {
      token += line.front();
    }
    line.remove_prefix(1);
  }
  return {token += '"', ltrim(line.substr(1))};
}

// }}}
// '/' {{{

tok_res slash_token(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '/')
    return {"/", ltrim(line.substr(1))};
  line.remove_prefix(1 + std::find(begin(line), end(line), '\n') - begin(line));
  return {"\n", line};
}

// }}}
// /./ {{{

tok_res name_token(boost::string_ref line)
{
  auto last = std::find_if(begin(line), end(line),
                           [](auto c) { return isspace(c) || !isnamechar(c); });
  std::string name{begin(line), last};
  return {name, ltrim(line.substr(last - begin(line)))};
}

// }}}

tok_res first_token(boost::string_ref line)
{
  tok_res token;
  switch (line.front()) {
  case '{':
  case '}':
  case '[':
  case ']':
  case '(':
  case ')':
  case ',':
  case ':':
  case ';':
  case '+':
  case '-':
  case '~':
  case '^':
  case '%':
  case '#':
  case '.':
  case '\'': return {{line.front()}, ltrim(line.substr(1))};
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9': return digit_token(line);
  case '0': return zero_token(line);
  case '=': return eq_tokens(line);
  case '!': return bang_tokens(line);
  case '*': return star_tokens(line);
  case '&': return and_tokens(line);
  case '|': return or_tokens(line);
  case '<': return lt_tokens(line);
  case '>': return gt_tokens(line);
  case '"': return string_token(line);
  case '/': return slash_token(line);
  default:  return name_token(line);
  }
}

// }}}

}
#include <iostream>

std::vector<std::string> parser::tokenize(std::istream& input)
{
  std::vector<std::string> tokens{};
  std::string current_line;
  while (input.peek() != EOF) {
    getline(input, current_line);
    boost::string_ref line{current_line};
    line = ltrim(line); // remove leading whitespace from line
    while (line.size()) {
      auto res = first_token(line);
      tokens.push_back(res.first);
      line = res.second;
    }
    tokens.push_back("\n");
  }
  return tokens;
}
