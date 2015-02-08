#include "parser.h"

#include "builtins.h"
#include "utils.h"
#include "ast/assignment.h"
#include "ast/block.h"
#include "ast/cond_statement.h"
#include "ast/for_loop.h"
#include "ast/function_call.h"
#include "ast/function_definition.h"
#include "ast/literal.h"
#include "ast/member.h"
#include "ast/type_definition.h"
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

// Grammar {{{

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
 *            ::= member
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
 * function_call ::= expression '(' expr_list ')'
 *               ::= '[' expr_list ']'
 *               ::= '{' dict_list '}'
 * expr_list ::=
 *           ::= expression
 *           ::= expression ',' expr_list
 * dict_list ::=
 *           ::= dict_pair
 *           ::= dict_pair ',' dict_list
 * dict_pair ::= expression ':' expression
 *
 * binop_call ::= expression binop expression
 *            ::= expression '[' expression ']'
 * binop ::= '+'
 *       ::= '-'
 *       ::= '*'
 *       ::= '/'
 *       ::= '%'
 *       ::= '&'
 *       ::= '|'
 *       ::= '^'
 *       ::= '=='
 *       ::= '!='
 *       ::= '<'
 *       ::= '>'
 *       ::= '<='
 *       ::= '>='
 *       ::= '&&'
 *       ::= '||'
 * monop_call ::= monop expression
 * monop ::= '!'
 *       ::= '~'
 *       ::= '-'
 *       ::= '#'
 *
 * member  ::= expression '.' name
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

// expression = ( assignment
//              | block
//              | cond_statement
//              | for_loop
//              | while_loop
//              | literal
//              | dict_literal
//              | array_literal
//              | function_definition
//              | monop_call
//              | variable
//              | type_definition
//              | variable_declaration
//              ) { ( binop expression
//                  | '(' expr_list ')'
//                  | '.' name '(' expr_list ')'
//                  | '[' expression ']'
//                  ) }

// }}}
// Parsing {{{

namespace {

template <typename T = std::unique_ptr<ast::expression>>
using parse_res = boost::optional<std::pair<T, vector_ref<std::string>>>;

using arg_t = std::vector<std::unique_ptr<ast::expression>>;

template <typename F>
auto parse_comma_separated_list(vector_ref<std::string> tokens,
                               const F& parse_item)
    -> parse_res<std::vector<decltype(parse_item(tokens)->first)>>;
template <typename F>
auto parse_bracketed_subexpr(vector_ref<std::string> tokens,
                             const F& parse_item,
                             const std::string& opening,
                             const std::string& closing)
    -> decltype(parse_item(tokens));

parse_res<> parse_expression(vector_ref<std::string> tokens);
parse_res<> parse_assignment(vector_ref<std::string> tokens);
parse_res<> parse_block(vector_ref<std::string> tokens);
parse_res<> parse_cond_statement(vector_ref<std::string> tokens);
parse_res<> parse_for_loop(vector_ref<std::string> tokens);
parse_res<> parse_while_loop(vector_ref<std::string> tokens);
parse_res<> parse_monop_call(vector_ref<std::string> tokens);
parse_res<arg_t> parse_function_call(vector_ref<std::string> tokens);
parse_res<std::pair<symbol, std::unique_ptr<ast::expression>>>
  parse_binop_call(vector_ref<std::string> tokens);
parse_res<symbol> parse_member(vector_ref<std::string> tokens);
parse_res<arg_t> parse_expr_list(vector_ref<std::string> tokens);
parse_res<> parse_function_definition(vector_ref<std::string> tokens);
parse_res<> parse_literal(vector_ref<std::string> tokens);
parse_res<> parse_array_literal(vector_ref<std::string> tokens);
parse_res<> parse_dict_literal(vector_ref<std::string> tokens);
parse_res<> parse_type_definition(vector_ref<std::string> tokens);
parse_res<> parse_variable_declaration(vector_ref<std::string> tokens);
parse_res<> parse_name(vector_ref<std::string> tokens);

// Individual parsing functions {{{

// comma-separated list {{{

template <typename F>
auto parse_comma_separated_list(vector_ref<std::string> tokens,
                                const F& parse_item)
    -> parse_res<std::vector<decltype(parse_item(tokens)->first)>>
{
  using res_t = decltype(parse_item(tokens)->first);

  std::vector<res_t> res;

  parse_res<res_t> expr_res;
  if (!(expr_res = parse_item(tokens)))
    return {{ move(res), tokens }};

  do {
    res.push_back(std::move(expr_res->first));
    tokens = expr_res->second;
    if (!tokens.size() || tokens.front() != ",")
      return {{ move(res), tokens }};
    tokens = tokens.remove_prefix(1);
  } while ((expr_res = parse_item(tokens)));
  return {};
}

// }}}
// bracketed subexpr {{{

template <typename F>
auto parse_bracketed_subexpr(vector_ref<std::string> tokens,
                             const F& parse_item,
                             const std::string& opening,
                             const std::string&) -> decltype(parse_item(tokens))
{
  if (tokens.size() && tokens.front() == opening) {
    if (auto item_res = parse_item(tokens.remove_prefix(1))) {
      tokens = item_res->second;
      return {{ move(item_res->first), tokens.remove_prefix(1) }};
    }
  }
  return {};
}

// }}}

// expression {{{

parse_res<> parse_expression(vector_ref<std::string> tokens)
{
  if (tokens.size() && tokens.front() == "(") {
    auto res = parse_expression(tokens.remove_prefix(1));
    return {{ move(res->first), res->second.remove_prefix(1) }};
  }

  parse_res<> res{};

  if (!( (res = parse_assignment(tokens))
      || (res = parse_block(tokens))
      || (res = parse_cond_statement(tokens))
      || (res = parse_for_loop(tokens))
      || (res = parse_while_loop(tokens))
      || (res = parse_function_definition(tokens))
      || (res = parse_monop_call(tokens))
      || (res = parse_literal(tokens))
      || (res = parse_dict_literal(tokens))
      || (res = parse_array_literal(tokens))
      || (res = parse_type_definition(tokens))
      || (res = parse_variable_declaration(tokens))
      || (res = parse_name(tokens))))
    return {};

  tokens = res->second;

  for (;;) {
    if (auto fres = parse_function_call(tokens)) {
      res->first = std::make_unique<ast::function_call>(move(res->first),
                                                        move(fres->first));
      tokens = fres->second;

    } else if (auto mres = parse_member(tokens)) {
      res->first = std::make_unique<ast::member>(move(res->first), mres->first);

      tokens = mres->second;

    } else if (auto bres = parse_binop_call(tokens)) {
      arg_t args{};
      args.push_back(move(bres->first.second));
      auto mem = std::make_unique<ast::member>(move(res->first),
                                               bres->first.first);
      res->first = std::make_unique<ast::function_call>(move(mem), move(args));

      tokens = bres->second;

    } else {
      return {{ move(res->first), tokens }};
    }
  }
}

// }}}
// assignment {{{

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
    do {
      tokens = tokens.remove_prefix(1);
    } while (tokens.front() == "\n" || tokens.front() == ";");
    auto expr_res = parse_expression(tokens);
    if (!expr_res)
      break;
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
  tokens = test_expr_res->second.remove_prefix(1);

  auto then_expr_res = parse_expression(tokens);
  auto then_expr = move(then_expr_res->first);
  tokens = then_expr_res->second;

  auto pair = make_pair(move(test_expr), move(then_expr));
  return {{ move(pair), tokens }};
  return {};
}

parse_res<> parse_inner_cond(vector_ref<std::string> tokens)
{
  std::vector<std::pair<std::unique_ptr<ast::expression>,
                        std::unique_ptr<ast::expression>>> pairs{};

  auto list = parse_comma_separated_list(tokens, parse_cond_pair);
  std::transform(begin(list->first), end(list->first), back_inserter(pairs),
                 [](auto&& i) { return move(i); });
  return {{ std::make_unique<ast::cond_statement>(move(pairs)), list->second }};
}

parse_res<> parse_if_statement(vector_ref<std::string> tokens)
{
  auto test_res = parse_expression(tokens.remove_prefix(1)); // 'if'
  auto test = move(test_res->first);
  tokens = test_res->second.remove_prefix(1); // ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;

  auto pair = std::make_pair(move(test), move(body));

  // this is all a bit weird
  std::vector<decltype(pair)> single_pair{};
  single_pair.push_back(move(pair));
  return {{ std::make_unique<ast::cond_statement>(move(single_pair)), tokens }};
}

parse_res<> parse_cond_statement(vector_ref<std::string> tokens)
{
  if (tokens.size() && tokens[0] == "if")
    return parse_if_statement(tokens);
  if (!tokens.size() || tokens[0] != "cond")
    return {};
   auto res = parse_bracketed_subexpr(tokens.remove_prefix(1), parse_inner_cond,
                                      "{", "}");
   return {{ move(res->first), res->second }};
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
  if (!tokens.size() || tokens.front() != "while")
    return {};
  auto test_res = parse_expression(tokens.remove_prefix(1)); // 'while'
  auto test = move(test_res->first);
  tokens = test_res->second.remove_prefix(1); // ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;
  return {{ std::make_unique<ast::while_loop>(move(test), move(body)),
            tokens }};
}

// }}}
// monop_call {{{

parse_res<symbol> parse_monop(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  const auto& op = tokens.front();
  tokens = tokens.remove_prefix(1);

  if (op == "!")  return {{ {"not"},      tokens }};
  if (op == "-")  return {{ {"negative"}, tokens }};
  if (op == "~")  return {{ {"negate"},   tokens }};
  if (op == "#")  return {{ {"box"},      tokens }};
  return {};
}

parse_res<> parse_monop_call(vector_ref<std::string> tokens)
{
  if (auto mon_res = parse_monop(tokens)) {
    tokens = mon_res->second;
    auto expr_res = parse_expression(tokens);
    tokens = expr_res->second;
    arg_t arg{};
    arg.push_back(move(expr_res->first));
    auto name = std::make_unique<ast::variable>(mon_res->first);
    return {{ std::make_unique<ast::function_call>(move(name), move(arg)),
              tokens }};
  }
  return {};
}

// }}}
// function_call {{{

parse_res<arg_t> parse_function_call(vector_ref<std::string> tokens)
{
  return parse_bracketed_subexpr(tokens, parse_expr_list, "(", ")");
}

// }}}
// binop_call {{{

parse_res<symbol> parse_binop(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  const auto& op = tokens.front();
  tokens = tokens.remove_prefix(1);

  if (op == "+")  return {{ {"add"},            tokens }};
  if (op == "-")  return {{ {"subtract"},       tokens }};
  if (op == "*")  return {{ {"times"},          tokens }};
  if (op == "/")  return {{ {"divides"},        tokens }};
  if (op == "%")  return {{ {"modulo"},         tokens }};
  if (op == "&")  return {{ {"bitand"},         tokens }};
  if (op == "|")  return {{ {"bitor"},          tokens }};
  if (op == "^")  return {{ {"xor"},            tokens }};
  if (op == "==") return {{ {"equals"},         tokens }};
  if (op == "!=") return {{ {"unequal"},        tokens }};
  if (op == "<")  return {{ {"less"},           tokens }};
  if (op == ">")  return {{ {"greater"},        tokens }};
  if (op == "<=") return {{ {"less_equals"},    tokens }};
  if (op == ">=") return {{ {"greater_equals"}, tokens }};
  if (op == "&&") return {{ {"and"},            tokens }};
  if (op == "||") return {{ {"or"},             tokens }};
  return {};
}

parse_res<std::pair<symbol, std::unique_ptr<ast::expression>>>
  parse_binop_call(vector_ref<std::string> tokens)
{
  if (auto bin_res = parse_binop(tokens)) {
    tokens = bin_res->second;
    auto expr_res = parse_expression(tokens);
    tokens = expr_res->second;
    auto pair = std::make_pair(bin_res->first, move(expr_res->first));
    return {{ move(pair), tokens }};
  }
  return {};
}

// }}}
// member {{{

parse_res<symbol> parse_member(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != ".")
    return {};
  tokens = tokens.remove_prefix(1);
  return {{ tokens.front(), tokens.remove_prefix(1) }};
}

// }}}
// expr_list {{{

parse_res<arg_t> parse_expr_list(vector_ref<std::string> tokens)
{
  return parse_comma_separated_list(tokens, parse_expression);
}

// }}}
// function_definition {{{

parse_res<symbol> parse_literal_symbol(vector_ref<std::string> tokens)
{
  if (!tokens.size() || !isnamechar(tokens.front().front()))
    return {};
  return {{ tokens.front(), tokens.remove_prefix(1) }};
}

parse_res<std::vector<symbol>> parse_arg_list(vector_ref<std::string> tokens)
{
  return parse_comma_separated_list(tokens, parse_literal_symbol);
}

parse_res<> parse_function_definition(vector_ref<std::string> tokens)
{
  if (tokens.front() != "fn")
    return {};
  symbol name{""};
  if (tokens[1] == "(") {
    tokens = tokens.remove_prefix(1);
  } else {
    name = tokens[1];
    tokens = tokens.remove_prefix(2);
  }
  auto arg_res = parse_bracketed_subexpr(tokens, parse_arg_list, "(", ")");
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

  if (find(begin(num), end(num), '.') != end(num)) {

    auto value = stod(num);
    auto literal = std::make_unique<ast::literal::floating_point>( value );
    return {{ move(literal), tokens.remove_prefix(1) }};
  }

  int value;

  if (num.front() == '0' && num[1] == 'x') {
    value = std::strtol(num.c_str() + 2, nullptr, 2);
  } else {
    value = std::stoi(num);
  }

  auto literal = std::make_unique<ast::literal::integer>( value );
  return {{ move(literal), tokens.remove_prefix(1) }};
}

parse_res<> parse_string(vector_ref<std::string> tokens)
{
  if (tokens.front().front() != '"')
    return {};
  std::string value{begin(tokens.front()) + 1, end(tokens.front()) - 1};

  auto literal = std::make_unique<ast::literal::string>( value );
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

  auto literal = std::make_unique<ast::literal::boolean>( value );
  return {{ move(literal), tokens.remove_prefix(1) }};
}

parse_res<> parse_nil(vector_ref<std::string> tokens)
{
  if (tokens.front() != "nil")
    return {};

  auto literal = std::make_unique<ast::literal::nil>( );
  return {{ move(literal), tokens.remove_prefix(1) }};
}

parse_res<> parse_symbol(vector_ref<std::string> tokens)
{
  if (tokens.front() != "'")
    return {};
  auto value = symbol{tokens[1]};

  auto literal = std::make_unique<ast::literal::symbol>( value );
  return {{ move(literal), tokens.remove_prefix(2) }};
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
// array_literal {{{

parse_res<> parse_array_literal(vector_ref<std::string> tokens)
{
  if (auto args = parse_bracketed_subexpr(tokens, parse_expr_list, "[", "]")) {
    auto fn_name = std::make_unique<ast::variable>(symbol{"Array"});
    return {{ std::make_unique<ast::function_call>(move(fn_name),
                                                   move(args->first)),
              args->second }};
  }
  return {};
}

// }}}
// dict_literal {{{

parse_res<std::pair<std::unique_ptr<ast::expression>,
                    std::unique_ptr<ast::expression>>>
  parse_dict_pair(vector_ref<std::string> tokens)
{
  auto test = parse_expression(tokens);
  auto body = parse_expression(test->second.remove_prefix(1));
  return {{ std::make_pair(move(test->first), move(test->first)),
            body->second }};
  return {};
}

auto parse_dict_internals(vector_ref<std::string> tokens)
{
  return parse_comma_separated_list(tokens, parse_dict_pair);
}

parse_res<> parse_dict_literal(vector_ref<std::string> tokens)
{
  if (auto args=parse_bracketed_subexpr(tokens, parse_dict_internals, "{", "}"))
    return {{ std::make_unique<ast::cond_statement>(move(args->first)),
              args->second }};
  return {};
}

// }}}
// type_definition {{{

parse_res<std::pair<il::symbol, std::shared_ptr<ast::function_definition>>>
  parse_method(vector_ref<std::string> tokens)
{
  if (tokens.front() != "mem")
    return {};
  symbol name{tokens[1]};
  tokens = tokens.remove_prefix(2);
  auto arg_res = parse_bracketed_subexpr(tokens, parse_arg_list, "(", ")");
  auto args = arg_res->first;
  tokens = arg_res->second.remove_prefix(1); // ':'
  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;

  auto fn = std::make_shared<ast::function_definition>(symbol{""},
                                                       move(body),
                                                       args);

  return {{ {name, fn}, tokens }};
}

parse_res<> parse_type_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "newtype")
    return {};
  tokens = tokens.remove_prefix(1);
  auto name = parse_literal_symbol(tokens);
  tokens = name->second;

  auto methods_res = parse_bracketed_subexpr(tokens,
                         [](auto t)
                           { return parse_comma_separated_list(t,
                                       parse_method); },
                           "{", "}");
  if (methods_res) {
    std::unordered_map<symbol,
                       std::shared_ptr<ast::function_definition>> methods;
    for (const auto& i : methods_res->first)
      methods[i.first] = i.second;

    return {{ std::make_unique<ast::type_definition>( name->first,
                                                      symbol{""},
                                                      methods ),
              methods_res->second }};
  }

  return {};
}

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
  return {{ std::make_unique<ast::variable_declaration>(name, move(value)),
            tokens }};
}

// }}}
// name {{{

parse_res<> parse_name(vector_ref<std::string> tokens)
{
  if (!tokens.size() || !isnamechar(tokens.front().front()))
    return {};
  symbol sym{tokens[0]};
  return {{ std::make_unique<ast::variable>( sym ), tokens.remove_prefix(1) }};
}

// }}}

// }}}

}

std::vector<std::unique_ptr<ast::expression>>
  parser::parse(token_string tokens)
{
  std::vector<std::unique_ptr<ast::expression>> expressions;
  auto first_nonline = std::find_if(begin(tokens), end(tokens),
                                    [](const auto& s) { return s != "\n"; });
  tokens = tokens.remove_prefix(first_nonline - begin(tokens));

  while (tokens.size()) {
    auto res = parse_expression(tokens);
    expressions.push_back(move(res->first));
    tokens = res->second;
    auto first_nonline = std::find_if(begin(tokens), end(tokens),
                                      [](const auto& s) { return s != "\n"; });
    tokens = tokens.remove_prefix(first_nonline - begin(tokens));
  }
  return expressions;
}

// }}}
