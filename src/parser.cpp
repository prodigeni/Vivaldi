#include "parser.h"

#include "ast/assignment.h"
#include "ast/block.h"
#include "ast/cond_statement.h"
#include "ast/for_loop.h"
#include "ast/function_call.h"
#include "ast/function_definition.h"
#include "ast/literal.h"
#include "ast/variable.h"
#include "ast/variable_declaration.h"
#include "ast/while_loop.h"
#include "value/boolean.h"
#include "value/floating_point.h"
#include "value/integer.h"
#include "value/nil.h"
#include "value/string.h"
#include "value/symbol.h"

#include <boost/utility/string_ref.hpp>
#include <boost/optional/optional.hpp>

using namespace il;

// String helper functions {{{

namespace {

boost::string_ref ltrim(boost::string_ref str)
{
  auto last = std::find_if_not(begin(str), end(str), isspace);
  str.remove_prefix(last - begin(str));
  return str;
}

boost::string_ref rm_prefix(boost::string_ref str, size_t prefix)
{
  str.remove_prefix(prefix);
  return str;
}

}

// }}}

// Tokenizing {{{

/**
 * Available tokens:
 * {
 * }
 * [
 * ]
 * (
 * )
 * .
 * ..
 * ...
 * ,
 * :
 * =
 * ==
 * +
 * -
 * *
 * !
 * ~
 * ^
 * &
 * &&
 * |
 * ||
 * /
 * <
 * >
 * <=
 * >=
 * %
 * '
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
    if (line[1] == 'x') {
      last = std::find_if_not(begin(line) + 2, end(line), ishexnumber);
    }
    if (line[1] == 'b') {
      last = std::find_if(begin(line) + 2, end(line),
                          [](auto c) { return c != '0' && c != '1'; });
    }
    if (isdigit(line[1])) {
      last = std::find_if(begin(line) + 2, end(line),
                          [](auto c)
                            { return !isdigit(c) || c == '8' || c == '9'; });
    }
  }
  std::string num{begin(line), last};
  return { num, ltrim(rm_prefix(line, last - begin(line))) };
}

// }}}
// '1'-'9' {{{

tok_res digit_token(boost::string_ref line)
{
  auto last = std::find_if_not(begin(line) + 2, end(line), isdigit);
  if (last != end(line) && *last == '.')
    last = std::find_if_not(last + 1, end(line), isdigit);
  std::string num{begin(line), last};
  return { num, ltrim(rm_prefix(line, last - begin(line))) };
}

// }}}
// '.' {{{

tok_res dot_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '.')
    return {".", ltrim(rm_prefix(line, 1))};
  if (line.size() == 2 || line[2] != '.')
    return {"..", ltrim(rm_prefix(line, 2))};
  return {"...", ltrim(rm_prefix(line, 3))};
}

// }}}
// '=' {{{

tok_res eq_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '=')
    return {"=", ltrim(rm_prefix(line, 1))};
  return {"==", ltrim(rm_prefix(line, 2))};
}

// }}}
// '&' {{{

tok_res and_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '&')
    return {"&", ltrim(rm_prefix(line, 1))};
  return {"&&", ltrim(rm_prefix(line, 2))};
}

// }}}
// '|' {{{

tok_res or_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '|')
    return {"|", ltrim(rm_prefix(line, 1))};
  return {"||", ltrim(rm_prefix(line, 2))};
}

// }}}
// '<' {{{

tok_res lt_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '<')
    return {"<", ltrim(rm_prefix(line, 1))};
  return {"<=", ltrim(rm_prefix(line, 2))};
}

// }}}
// '>' {{{

tok_res gt_tokens(boost::string_ref line)
{
  if (line.size() == 1 || line[1] != '>')
    return {">", ltrim(rm_prefix(line, 1))};
  return {">=", ltrim(rm_prefix(line, 2))};
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
  return {token, ltrim(rm_prefix(line, 1))};
}

// }}}
// /./ {{{

tok_res name_token(boost::string_ref line)
{
  auto last = std::find_if(begin(line), end(line),
                           [](auto c) { return isspace(c) || ispunct(c); });
  std::string name{begin(line), last};
  return {name, ltrim(rm_prefix(line, last - begin(line)))};
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
  case '*':
  case '~':
  case '^':
  case '/':
  case '%':
  case '\'': return {{line.front()}, ltrim(rm_prefix(line, 1))};
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
  case '.': return dot_tokens(line);
  case '=': return eq_tokens(line);
  case '&': return and_tokens(line);
  case '|': return or_tokens(line);
  case '<': return lt_tokens(line);
  case '>': return gt_tokens(line);
  case '"': return string_token(line);
  default:  return name_token(line);
  }
}

// }}}

}

parser::token_string parser::tokenize(std::istream& input)
{
  token_string tokens{};
  std::string current_line;
  while (input.peek() != EOF) {
    getline(input, current_line);
    boost::string_ref line{current_line};
    while (line.size()) {
      auto res = first_token(line);
      tokens.push_back(res.first);
      line = res.second;
    }
    tokens.push_back("\n");
  }
  return tokens;
}

// }}}
// Parsing {{{

/**
 * expression ::= assignment
 *            ::= block
 *            ::= cond_statement
 *            ::= for_loop
 *            ::= function_call
 *            ::= function_definition
 *            ::= literal
 *            ::= type_definition
 *            ::= name
 *            ::= variable_declaration
 *            ::= while_loop
 *            ::= '(' expression ')'
 *
 * assignment ::= name '=' expression
 *
 * block ::= '{' inner_block '}'
 * inner_block ::=
 *             ::= expression '\\n' inner_block
 *             ::= expression ';' inner_block
 *
 * cond_statement ::= 'cond' '{' inner_cond '}'
 *                ::= 'if' cond_pair
 *                ::= 'if' cond_pair 'else' expression
 * inner_cond ::=
 *            ::= cond_pair
 *            ::= cond_pair, inner_cond
 * cond_pair ::= expression ':' expression
 *
 * for_loop ::= 'for' name 'in' expression ':' expression
 *
 * while_loop ::= 'while' expression ':' expression
 *
 * function_call ::= name '(' expr_list ')'
 *               ::= name '.' name '(' expr_list ')'
 *               ::= '[' expr_list ']'
 *               ::= '{' dict_list '}'
 *               ::= expression '+' expression
 *               ::= expression '-' expression
 *               ::= expression '*' expression
 *               ::= expression '/' expression
 *               ::= expression '%' expression
 *               ::= expression '&' expression
 *               ::= expression '|' expression
 *               ::= expression '^' expression
 *               ::= expression '==' expression
 *               ::= expression '!=' expression
 *               ::= expression '<' expression
 *               ::= expression '>' expression
 *               ::= expression '<=' expression
 *               ::= expression '>=' expression
 *               ::= expression '&&' expression
 *               ::= expression '||' expression
 *               ::= expression '[' expression ']'
 *               ::= '!' expression
 *               ::= '~' expression
 *               ::= '-' expression
 * expr_list ::=
 *           ::= expression
 *           ::= expression ',' expr_list
 * dict_list ::=
 *           ::= dict_pair
 *           ::= dict_pair ',' dict_list
 * dict_pair ::= expression ':' expression
 *
 * function_definition ::= 'fn' name '(' arg_list ')' ':' expression
 *                     ::= 'fn' '(' arg_list ')' ':' expression
 * arg_list ::=
 *          ::= name
 *          ::= name ',' arg_list
 *
 * literal ::= number
 *         ::= string
 *         ::= symbol
 *         ::= 'true'
 *         ::= 'false'
 *         ::= 'nil'
 *
 * type_definition ::= 'type' name type_body
 *                 ::= 'type' name ':' name type_body
 * type_body ::= '{' inner_type_body '}'
 * inner_type_body ::=
 *                 ::= type_member
 *                 ::= type_member, inner_type_body
 * type_member ::= type_status variable_declaration
 *             ::= type_status function_definition
 * type_status ::=
 *             ::= 'public'
 *             ::= 'private'
 *             ::= 'static'
 *
 * variable_declaration ::= 'let' name '=' expression
 *
 */

namespace {

// Helper vector_ref class {{{

template <typename T>
class vector_ref {
public:
  using iterator = const T*;

  vector_ref(const std::vector<T>& vec)
    : m_data {vec.data()},
      m_sz   {vec.size()}
  { }

  template <typename I>
  vector_ref(I first, I last) : m_data{&*first}, m_sz( last - first ) { }

  vector_ref remove_prefix(size_t prefix) const
  {
    return {m_data + prefix, m_data + m_sz};
  }

  const T& front() const { return *m_data; }
  const T& back() const { return m_data[m_sz - 1]; }

  size_t size() const { return m_sz; }

  const T& operator[](int idx) { return m_data[idx]; }

  iterator begin() const { return m_data; }
  iterator end() const { return m_data + m_sz; }

private:
  const T* m_data;
  size_t m_sz;
};

// }}}

template <typename T = std::unique_ptr<ast::expression>>
using parse_res = boost::optional<std::pair<T, vector_ref<std::string>>>;

parse_res<> parse_expression(vector_ref<std::string> tokens);
parse_res<> parse_assignment(vector_ref<std::string> tokens);
parse_res<> parse_block(vector_ref<std::string> tokens);
parse_res<> parse_cond_statement(vector_ref<std::string> tokens);
parse_res<> parse_for_loop(vector_ref<std::string> tokens);
parse_res<> parse_while_loop(vector_ref<std::string> tokens);
parse_res<> parse_function_call(vector_ref<std::string> tokens);
parse_res<std::vector<std::unique_ptr<ast::expression>>>
  parse_expr_list(vector_ref<std::string> tokens);
parse_res<> parse_function_definition(vector_ref<std::string> tokens);
parse_res<> parse_literal(vector_ref<std::string> tokens);
//parse_res<> parse_type_definition(vector_ref<std::string> tokens);
parse_res<> parse_variable_declaration(vector_ref<std::string> tokens);
parse_res<> parse_name(vector_ref<std::string> tokens);

// Individual parsing functions {{{

// expression {{{

parse_res<> parse_expression(vector_ref<std::string> tokens)
{
  if (tokens.front() == "(") {
    auto res = parse_expression(tokens.remove_prefix(1));
    return {{ move(res->first), res->second.remove_prefix(1) }};
  }

  parse_res<> res;
  if ((res = parse_assignment(tokens)))
    return res;
  else if ((res = parse_block(tokens)))
    return res;
  else if ((res = parse_cond_statement(tokens)))
    return res;
  else if ((res = parse_for_loop(tokens)))
    return res;
  else if ((res = parse_while_loop(tokens)))
    return res;
  else if ((res = parse_function_call(tokens)))
    return res;
  else if ((res = parse_function_definition(tokens)))
    return res;
  else if ((res = parse_literal(tokens)))
    return res;
  //else if ((res = parse_type_definition(tokens)))
    //return res;
  else if ((res = parse_variable_declaration(tokens)))
    return res;
  return parse_name(tokens);
}

// }}}
// assignment !!! {{{

parse_res<> parse_assignment(vector_ref<std::string> tokens)
{
  if (tokens.size() == 1 || tokens[1] != "=")
    return {};
  symbol name{tokens.front()};
  auto value_res = parse_expression(tokens.remove_prefix(2));
  auto value = move(value_res->first);
  tokens = move(value_res->second);

  return {{ std::make_unique<ast::assignment>(name, move(value)), tokens }};
}

// }}}
// block {{{

parse_res<> parse_block(vector_ref<std::string> tokens)
{
  if (tokens.front() != "{")
    return {};

  std::vector<std::unique_ptr<ast::expression>> subexprs;
  do {
    do tokens = tokens.remove_prefix(1); while (tokens.front() == "\n");
    auto expr_res = parse_expression(tokens);
    subexprs.push_back(move(expr_res->first));
    tokens = expr_res->second;
  } while (tokens.front() == "\n" || tokens.front() == ";");

  return {{ std::make_unique<ast::block>(move(subexprs)),
            tokens.remove_prefix(1) }}; // '}'
}

// }}}
// cond_statement {{{

parse_res<std::pair<std::unique_ptr<ast::expression>,
                    std::unique_ptr<ast::expression>>>
parse_cond_pair(vector_ref<std::string> tokens)
{
  if (tokens.size() < 3 || tokens[0] == "}")
    return {};

  auto test_expr_res = parse_expression(tokens);
  auto test_expr = move(test_expr_res->first);
  tokens = test_expr_res->second;

  auto then_expr_res = parse_expression(tokens);
  auto then_expr = move(then_expr_res->first);
  tokens = then_expr_res->second;

  auto pair = make_pair(move(test_expr), move(then_expr));
  return {{ move(pair), tokens }};
}

parse_res<> parse_inner_cond(vector_ref<std::string> tokens)
{
  std::vector<std::pair<std::unique_ptr<ast::expression>,
                        std::unique_ptr<ast::expression>>> pairs{};

  while (auto cur_pair = parse_cond_pair(tokens)) {
    pairs.push_back(move(cur_pair->first));
    tokens = cur_pair->second;
  };
  return {{ std::make_unique<ast::cond_statement>( move(pairs) ), tokens }};
}

parse_res<> parse_if_statement(vector_ref<std::string> tokens)
{
  auto test_res = parse_expression(tokens.remove_prefix(1));
  auto test = move(test_res->first);
  tokens = test_res->second.remove_prefix(1); // ':'
  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second.remove_prefix(1);

  auto pair = std::make_pair(move(test), move(body));

  // this is all a bit weird
  std::vector<decltype(pair)> single_pair{};
  single_pair.push_back(move(pair));
  return {{ std::make_unique<ast::cond_statement>(move(single_pair)), tokens }};
}

parse_res<> parse_cond_statement(vector_ref<std::string> tokens)
{
  if (tokens[0] == "if")
    return parse_if_statement(tokens);
  if (tokens[0] != "cond")
    return {};
   auto res = parse_inner_cond(tokens.remove_prefix(2)); // 'cond' '{'
   return {{ move(res->first), res->second.remove_prefix(1) }};
}

// }}}
// for_loop {{{

parse_res<> parse_for_loop(vector_ref<std::string> tokens)
{
  if (tokens.front() != "for")
    return {};
  symbol iterator{tokens[1]};
  auto range_res = parse_expression(tokens.remove_prefix(3)); // 'for' name 'in'
  auto range = move(range_res->first);
  tokens = range_res->second.remove_prefix(1); // ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;
  return {{ std::make_unique<ast::for_loop>(iterator, move(range), move(body)),
            tokens }};
}

// }}}
// while_loop {{{

parse_res<> parse_while_loop(vector_ref<std::string> tokens)
{
  if (tokens.front() != "for")
    return {};
  auto test_res = parse_expression(tokens.remove_prefix(1)); // 'for' name 'in'
  auto test = move(test_res->first);
  tokens = test_res->second.remove_prefix(1); // ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;
  return {{ std::make_unique<ast::while_loop>(move(test), move(body)),
            tokens }};
}

// }}}
// function_call {{{

//parse_res<> parse_dict_pair(vector_ref<std::string> tokens);
//parse_res<> parse_dict_list(vector_ref<std::string> tokens);

parse_res<> parse_function_call(vector_ref<std::string> tokens)
{
  if (tokens.front() == "[") {
    symbol function_name{"make_array"};
    auto list_res = parse_expr_list(tokens.remove_prefix(1)); // '['
    auto list = move(list_res->first);
    tokens = list_res->second.remove_prefix(1); // ']'

    return {{ std::make_unique<ast::function_call>(function_name, move(list)),
              tokens }};
  }

  if (tokens.size() == 1 || tokens[1] != "(")
    return {};
  symbol function_name{tokens.front()};
  auto list_res = parse_expr_list(tokens.remove_prefix(2));
  auto list = move(list_res->first);
  tokens = list_res->second.remove_prefix(1); // ')'

  return {{ std::make_unique<ast::function_call>(function_name, move(list)),
            tokens }};
}

// }}}
// expr_list {{{

parse_res<std::vector<std::unique_ptr<ast::expression>>>
  parse_expr_list(vector_ref<std::string> tokens)
{
  if (tokens.front() == ")" || tokens.front() == "]")
    return {};
  std::vector<std::unique_ptr<ast::expression>> exprs;
  do {
    tokens.remove_prefix(1);
    auto res = parse_expression(tokens);
    exprs.push_back(move(res->first));
    tokens = res->second;
  } while (tokens.front() == ",");
  return {{ move(exprs), tokens }};
}

// }}}
// function_definition {{{

parse_res<std::vector<symbol>> parse_arg_list(vector_ref<std::string> tokens)
{
  if (tokens.front() == ")")
    return {{ {}, tokens.remove_prefix(1) }};

  std::vector<symbol> syms{};
  for (;;) {
    syms.push_back(tokens.front());
    tokens = tokens.remove_prefix(1);
    if (tokens.front() == ",")
      tokens = tokens.remove_prefix(1);
    else
      return {{ syms, tokens }};
  }
}

parse_res<> parse_function_definition(vector_ref<std::string> tokens)
{
  if (tokens.front() != "fn")
    return {};
  symbol name{""};
  if (tokens[1] == "(") {
    tokens = tokens.remove_prefix(2);
  } else {
    name = tokens[1];
    tokens = tokens.remove_prefix(3);
  }
  auto arg_res = parse_arg_list(tokens);
  auto args = arg_res->first;
  tokens = arg_res->second.remove_prefix(1); // ':'
  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_unique<ast::function_definition>(name, move(body), args),
            tokens }};
}

// }}}
// literal {{{

parse_res<> parse_number(vector_ref<std::string> tokens)
{
  const auto& num = tokens.front();
  if (!isdigit(num.front()))
    return {};

  if (num.find('.') != num.npos) {

    auto value = stod(num);
    std::unique_ptr<value::base> flt{new value::floating_point{value}};
    std::unique_ptr<ast::expression> literal{new ast::literal{move(flt)}};
    return {{ move(literal), tokens.remove_prefix(1) }};
  }

  int value;

  if (num.front() == '0' && num[1] == 'x') {
    value = std::strtol(num.c_str() + 2, nullptr, 2);
  } else {
    value = std::stoi(num);
  }

  std::unique_ptr<value::base> integer{new value::integer{value}};
  std::unique_ptr<ast::expression> literal{new ast::literal{move(integer)}};
  return {{ move(literal), tokens.remove_prefix(1) }};
}

parse_res<> parse_string(vector_ref<std::string> tokens)
{
  if (tokens.front().front() != '"')
    return {};
  std::string str{begin(tokens.front()) + 1, end(tokens.front()) - 1};

  std::unique_ptr<value::base> string{new value::string{str}};
  std::unique_ptr<ast::expression> literal{new ast::literal{move(string)}};
  return {{ move(literal), tokens.remove_prefix(1) }};
}

parse_res<> parse_bool(vector_ref<std::string> tokens)
{
  bool value;
  if (tokens.front() == "true")
    value = true;
  else if (tokens.front() == "false")
    value = false;
  else
    return {};
  std::unique_ptr<value::base> boolean{new value::boolean{value}};
  std::unique_ptr<ast::expression> literal{new ast::literal{move(boolean)}};
  return {{ move(literal), tokens.remove_prefix(1) }};
}

parse_res<> parse_nil(vector_ref<std::string> tokens)
{
  if (tokens.front() != "nil")
    return {};

  std::unique_ptr<value::base> nil{new value::nil{}};
  std::unique_ptr<ast::expression> literal{new ast::literal{move(nil)}};
  return {{ move(literal), tokens.remove_prefix(1) }};
}

parse_res<> parse_symbol(vector_ref<std::string> tokens)
{
  if (tokens.front() != "'")
    return {};
  auto sym = symbol{tokens[1]};

  std::unique_ptr<value::base> vsym{new value::symbol{sym}};
  std::unique_ptr<ast::expression> literal{new ast::literal{move(vsym)}};
  return {{ move(literal), tokens.remove_prefix(1) }};
}

parse_res<> parse_literal(vector_ref<std::string> tokens)
{
  parse_res<> res{};
  if ((res = parse_number(tokens)))
    return res;
  if ((res = parse_string(tokens)))
    return res;
  if ((res = parse_bool(tokens)))
    return res;
  if ((res = parse_nil(tokens)))
    return res;
  if ((res = parse_symbol(tokens)))
    return res;
  return {};
}

// }}}
// type_definition !!! {{{

//parse_res<> parse_type_definition(vector_ref<std::string> tokens);

// }}}
// variable_declaration {{{

parse_res<> parse_variable_declaration(vector_ref<std::string> tokens)
{
  if (tokens.front() != "let")
    return {};
  symbol name{tokens[1]};
  tokens = tokens.remove_prefix(3);
  auto value_res = parse_expression(tokens);
  auto value = move(value_res->first);
  tokens = value_res->second;
  return {{ std::make_unique<ast::variable_declaration>(name, value), tokens }};
}

// }}}
// name {{{

parse_res<> parse_name(vector_ref<std::string> tokens)
{
  symbol sym{tokens[0]};
  return {{ std::make_unique<ast::variable>( sym ), tokens.remove_prefix(1) }};
}

// }}}

// }}}

}

std::vector<std::unique_ptr<ast::expression>>
  parser::parse(const token_string& tokens)
{
  std::vector<std::unique_ptr<ast::expression>> expressions;
  vector_ref<std::string> tok_ref{tokens};
  auto first_nonline = std::find_if(begin(tok_ref), end(tok_ref),
                                    [](const auto& s) { return s != "\n"; });
  tok_ref = tok_ref.remove_prefix(first_nonline - begin(tok_ref));

  while (tok_ref.size()) {
    auto res = parse_expression(tok_ref);
    expressions.push_back(move(res->first));
    tok_ref = res->second;
    auto first_nonline = std::find_if(begin(tok_ref), end(tok_ref),
                                      [](const auto& s) { return s != "\n"; });
    tok_ref = tok_ref.remove_prefix(first_nonline - begin(tok_ref));
  }
  return expressions;
}

// }}}
