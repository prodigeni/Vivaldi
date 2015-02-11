#include "parser.h"

#include "builtins.h"
#include "utils.h"
#include "ast/assignment.h"
#include "ast/block.h"
#include "ast/cond_statement.h"
#include "ast/except.h"
#include "ast/for_loop.h"
#include "ast/function_call.h"
#include "ast/function_definition.h"
#include "ast/literal.h"
#include "ast/logical_and.h"
#include "ast/logical_or.h"
#include "ast/member.h"
#include "ast/member_assignment.h"
#include "ast/try_catch.h"
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

#include <stack>

using namespace vv;
using namespace ast;

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

template <typename T = std::unique_ptr<expression>>
using parse_res = boost::optional<std::pair<T, vector_ref<std::string>>>;

using arg_t = std::vector<std::unique_ptr<expression>>;

parse_res<> parse_expression(vector_ref<std::string> tokens);

parse_res<> parse_nonop_expression(vector_ref<std::string> tokens);
parse_res<> parse_prec0(vector_ref<std::string> tokens); // member, call
parse_res<> parse_prec1(vector_ref<std::string> tokens); // monops
parse_res<> parse_prec2(vector_ref<std::string> tokens); // *, /, %
parse_res<> parse_prec3(vector_ref<std::string> tokens); // +, -
parse_res<> parse_prec4(vector_ref<std::string> tokens); // <<, >>
parse_res<> parse_prec5(vector_ref<std::string> tokens); // &
parse_res<> parse_prec6(vector_ref<std::string> tokens); // ^
parse_res<> parse_prec7(vector_ref<std::string> tokens); // |
parse_res<> parse_prec8(vector_ref<std::string> tokens); // <, >, <=, =>
parse_res<> parse_prec9(vector_ref<std::string> tokens); // ==, !=
parse_res<> parse_prec10(vector_ref<std::string> tokens); // &&
parse_res<> parse_prec11(vector_ref<std::string> tokens); // ||

parse_res<> parse_assignment(vector_ref<std::string> tokens);
parse_res<> parse_block(vector_ref<std::string> tokens);
parse_res<> parse_cond_statement(vector_ref<std::string> tokens);
parse_res<> parse_except(vector_ref<std::string> tokens);
parse_res<> parse_while_loop(vector_ref<std::string> tokens);
parse_res<> parse_function_definition(vector_ref<std::string> tokens);
parse_res<> parse_literal(vector_ref<std::string> tokens);
parse_res<> parse_array_literal(vector_ref<std::string> tokens);
parse_res<> parse_try_catch(vector_ref<std::string> tokens);
parse_res<> parse_type_definition(vector_ref<std::string> tokens);
parse_res<> parse_variable_declaration(vector_ref<std::string> tokens);
parse_res<> parse_variable(vector_ref<std::string> tokens);

parse_res<> parse_symbol(vector_ref<std::string> tokens);
parse_res<> parse_integer(vector_ref<std::string> tokens);
parse_res<> parse_float(vector_ref<std::string> tokens);
parse_res<> parse_bool(vector_ref<std::string> tokens);
parse_res<> parse_nil(vector_ref<std::string> tokens);
parse_res<> parse_string(vector_ref<std::string> tokens);

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
parse_res<arg_t> parse_function_call(vector_ref<std::string> tokens);
parse_res<std::pair<std::unique_ptr<expression>, std::unique_ptr<expression>>>
  parse_cond_pair(vector_ref<std::string> tokens);
parse_res<std::pair<symbol, std::unique_ptr<function_definition>>>
  parse_method_definition(vector_ref<std::string> tokens);

// Individual parsing functions {{{

parse_res<> parse_expression(vector_ref<std::string> tokens)
{
  return parse_prec11(tokens);
}

// Operators {{{

template <typename F1, typename F2, typename Pred, typename Transform>
parse_res<> parse_operator_expr(vector_ref<std::string> tokens,
                                const F1& pre,
                                const F2& post,
                                const Pred& test,
                                const Transform& convert)
{
  auto left_res = pre(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  if (tokens.size() && test(tokens.front())) {
    auto left = move(left_res->first);

    symbol method{convert(tokens.front())};

    auto right_res = post(tokens.remove_prefix(1));
    auto right = move(right_res->first);
    tokens = right_res->second;

    auto member = std::make_unique<ast::member>( move(left), method );
    arg_t arg{};
    arg.emplace_back(move(right));

    return {{std::make_unique<function_call>(move(member), move(arg)), tokens}};
  }
  return left_res;
}

parse_res<> parse_prec11(vector_ref<std::string> tokens)
{
  auto left_res = parse_prec10(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  if (tokens.size() && tokens.front() == "||") {
    auto left = move(left_res->first);

    auto right_res = parse_prec11(tokens.remove_prefix(1));
    auto right = move(right_res->first);
    tokens = right_res->second;

    return {{ std::make_unique<logical_or>(move(left), move(right)), tokens }};
  }
  return left_res;
}

parse_res<> parse_prec10(vector_ref<std::string> tokens)
{
  auto left_res = parse_prec9(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  if (tokens.size() && tokens.front() == "&&") {
    auto left = move(left_res->first);
    auto right_res = parse_prec10(tokens.remove_prefix(1));
    auto right = move(right_res->first);
    tokens = right_res->second;

    return {{ std::make_unique<logical_and>(move(left), move(right)), tokens }};
  }
  return left_res;
}

parse_res<> parse_prec9(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec8, parse_prec9,
                             [](const auto& s)
                             {
                               return s == "==" || s == "!=";
                             },
                             [](const auto& s)
                             {
                               return (s == "==") ? "equals" : "unequal";
                             });
}

parse_res<> parse_prec8(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec7, parse_prec8,
                             [](const auto& s)
                             {
                               return s == ">"
                                   || s == "<"
                                   || s == ">="
                                   || s == "<=";
                             },
                             [](const auto& s)
                             {
                               return (s == ">")  ? "greater" :
                                      (s == "<")  ? "less"    :
                                      (s == "<=") ? "greater_equals" :
                                                    "less_equals";
                             });
}

parse_res<> parse_prec7(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec6, parse_prec7,
                             [](const auto& s) { return s == "|"; },
                             [](const auto&) { return "bitor"; });
}

parse_res<> parse_prec6(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec5, parse_prec6,
                             [](const auto& s) { return s == "^"; },
                             [](const auto&) { return "xor"; });
}

parse_res<> parse_prec5(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec4, parse_prec5,
                             [](const auto& s) { return s == "&"; },
                             [](const auto&) { return "bitand"; });
}

parse_res<> parse_prec4(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec3, parse_prec4,
                             [](const auto& s)
                             {
                               return s == ">>" || s == "<<";
                             },
                             [](const auto& s)
                             {
                               return (s == ">>") ? "rshift" : "lshift";
                             });
}

parse_res<> parse_prec3(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec2, parse_prec3,
                             [](const auto& s) { return s == "+" || s == "-"; },
                             [](const auto& s)
                             {
                               return (s == "+") ? "add" : "subtract";
                             });
}

parse_res<> parse_prec2(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec1, parse_prec2,
                             [](const auto& s)
                             {
                               return s == "*" || s == "/" || s == "%";
                             },
                             [](const auto& s)
                             {
                               return (s == "*") ? "times" :
                                      (s == "/") ? "divides" :
                                                   "modulo";
                             });
}

parse_res<> parse_prec1(vector_ref<std::string> tokens)
{
  if (tokens.size() && (tokens.front() == "!"
                     || tokens.front() == "~"
                     || tokens.front() == "-")) {


    symbol method{(tokens.front() == "!") ? "not" :
                  (tokens.front() == "~") ? "negate" :
                                            "negative"};
    auto expr_res = parse_prec1(tokens.remove_prefix(1));
    tokens = expr_res->second;
    auto expr = move(expr_res->first);
    auto member = std::make_unique<ast::member>( move(expr), method );

    arg_t empty{};
    return {{ std::make_unique<function_call>( move(member), move(empty) ),
              tokens }};
  }
  return parse_prec0(tokens);
}

parse_res<> parse_prec0(vector_ref<std::string> tokens)
{
  auto expr_res = parse_nonop_expression(tokens);
  if (!expr_res)
    return expr_res;
  tokens = expr_res->second;

  if (tokens.size() && tokens.front() == "(") {
    auto expr = move(expr_res->first);

    auto list_res = parse_function_call(tokens);
    auto list = move(list_res->first);
    tokens = list_res->second;

    return {{std::make_unique<function_call>(move(expr), move(list)), tokens}};

  } else if (tokens.size() && tokens.front() == ".") {
    auto expr = move(expr_res->first);

    tokens = tokens.remove_prefix(1);
    symbol name{tokens.front()};
    tokens = tokens.remove_prefix(1);

    if (tokens.size() && tokens.front() == "=") {
      auto value_res = parse_expression(tokens.remove_prefix(1));
      auto value = move(value_res->first);
      tokens = value_res->second;
      return {{std::make_unique<member_assignment>(move(expr),name,move(value)),
               tokens}};
    }
    return {{ std::make_unique<member>( move(expr), name ), tokens }};
  }
  return expr_res;
}

parse_res<> parse_nonop_expression(vector_ref<std::string> tokens)
{
  if (tokens.size() && tokens.front() == "(")
    return parse_bracketed_subexpr(tokens, parse_expression, "(", ")");

  parse_res<> res;
  if ((res = parse_assignment(tokens)))           return res;
  if ((res = parse_block(tokens)))                return res;
  if ((res = parse_cond_statement(tokens)))       return res;
  if ((res = parse_except(tokens)))               return res;
  if ((res = parse_while_loop(tokens)))           return res;
  if ((res = parse_function_definition(tokens)))  return res;
  if ((res = parse_literal(tokens)))              return res;
  if ((res = parse_array_literal(tokens)))        return res;
  if ((res = parse_try_catch(tokens)))            return res;
  if ((res = parse_type_definition(tokens)))      return res;
  if ((res = parse_variable_declaration(tokens))) return res;
  if ((res = parse_variable(tokens)))             return res;
  return {};
}

// }}}
// Other expressions {{{

parse_res<> parse_assignment(vector_ref<std::string> tokens)
{
  if (tokens.size() < 2 || tokens[1] != "=")
    return {};
  auto name = tokens.front();
  auto expr_res = parse_expression(tokens.remove_prefix(2)); // name, '='
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<assignment>( name, move(expr) ), tokens }};
}

parse_res<> parse_block(vector_ref<std::string> tokens)
{
  const static auto trim_test = [](const auto& s) { return s=="\n" || s==";"; };

  if (!tokens.size() || tokens.front() != "{")
    return {};
  tokens = tokens.remove_prefix(1); // '{'

  std::vector<std::unique_ptr<expression>> subexprs;
  tokens = ltrim_if(tokens, trim_test);
  while (tokens.front() != "}") {
    auto expr_res = parse_expression(tokens);
    subexprs.push_back(move(expr_res->first));
    tokens = expr_res->second;
    tokens = ltrim_if(tokens, trim_test);
  }

  tokens = tokens.remove_prefix(1); // '}'
  return {{ std::make_unique<block>( move(subexprs) ), tokens }};
}

parse_res<> parse_cond_statement(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "cond")
    return {};
  tokens = tokens.remove_prefix(1);

  auto pairs_res = parse_bracketed_subexpr(tokens, [](auto tokens)
  {
    return parse_comma_separated_list(tokens, parse_cond_pair);
  }, "{", "}");
  auto pairs = move(pairs_res->first);
  tokens = pairs_res->second;

  return {{ std::make_unique<cond_statement>(move(pairs)), tokens }};
}

parse_res<> parse_except(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "except")
    return {};
  tokens = tokens.remove_prefix(1); // 'except'
  auto expr_res = parse_expression(tokens);
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<except>( move(expr) ), tokens }};
}

parse_res<> parse_while_loop(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "while")
    return {};

  auto test_res = parse_expression(tokens.remove_prefix(1)); // 'while'
  auto test = move(test_res->first);
  tokens = test_res->second;

  auto body_res = parse_expression(tokens.remove_prefix(1)); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_unique<while_loop>( move(test), move(body) ), tokens }};
}

parse_res<> parse_function_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "fn")
    return {};
  tokens = tokens.remove_prefix(1);

  symbol name{""};
  if (tokens.front() != ":") {
    name = tokens.front();
    tokens = tokens.remove_prefix(1);
  }

  auto arg_res = parse_bracketed_subexpr(tokens, [](auto tokens)
  {
    return parse_comma_separated_list(tokens, [](auto t)
    {
      if (t.front() == ")")
        return parse_res<symbol>{};
      return parse_res<symbol>{{ symbol{t.front()}, t.remove_prefix(1) }};
    });
  }, "(", ")");
  auto args = move(arg_res->first);
  tokens = arg_res->second;

  auto body_res = parse_expression(tokens.remove_prefix(1)); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_unique<function_definition>( name, move(body), args ),
            tokens }};
}

parse_res<> parse_literal(vector_ref<std::string> tokens)
{
  parse_res<> res;
  if ((res = parse_float(tokens)))   return res;
  if ((res = parse_integer(tokens))) return res;
  if ((res = parse_bool(tokens)))    return res;
  if ((res = parse_nil(tokens)))     return res;
  if ((res = parse_symbol(tokens)))  return res;
  if ((res = parse_string(tokens)))  return res;
  return res;
}

parse_res<> parse_array_literal(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "[")
    return {};

  auto vals_res = parse_bracketed_subexpr(tokens, [](auto t)
  {
    return parse_comma_separated_list(t, parse_expression);
  },"[", "]");
  auto vals = move(vals_res->first);
  tokens = vals_res->second;

  // Ugly, but necessary, since function arguments are pushed (and so popped)
  // backwards
  reverse(begin(vals), end(vals));

  auto name = std::make_unique<variable>( symbol{"Array"} );
  return {{ std::make_unique<function_call>(move(name), move(vals)), tokens }};
}

parse_res<> parse_try_catch(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "try")
    return {};
  tokens = tokens.remove_prefix(2); // 'try' ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;

  tokens = ltrim(tokens, {"\n"});
  tokens = tokens.remove_prefix(1); // 'catch'
  auto exception_name = tokens.front();
  tokens = tokens.remove_prefix(2); // name ':'

  auto catcher_res = parse_expression(tokens);
  auto catcher = move(catcher_res->first);
  tokens = catcher_res->second;

  return {{ std::make_unique<try_catch>( move(body), move(catcher) ), tokens }};
}

parse_res<> parse_type_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "class")
    return{};
  symbol name{tokens[1]};
  tokens = tokens.remove_prefix(2); // 'class' name

  symbol parent{"Object"};
  if (tokens.front() == ":") {
    parent = tokens[1];
    tokens = tokens.remove_prefix(2); // ':' parent
  }

  auto method_res = parse_bracketed_subexpr(tokens, [](auto t)
  {
    return parse_comma_separated_list(t, parse_method_definition);
  }, "{", "}");
  auto methods = move(method_res->first);
  tokens = method_res->second;

  std::unordered_map<symbol, std::unique_ptr<function_definition>> method_map;
  for (auto&& i : methods)
    method_map.emplace(i.first, move(i.second));

  return {{ std::make_unique<type_definition>( name, parent, move(method_map) ),
            tokens }};
}

parse_res<> parse_variable_declaration(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "let")
    return {};
  auto name = tokens[1];
  auto expr_res = parse_expression(tokens.remove_prefix(3)); // 'let', name, '='
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<variable_declaration>(name, move(expr)), tokens }};
}

parse_res<> parse_variable(vector_ref<std::string> tokens)
{
  if (!tokens.size() || !isnamechar(tokens.front().front()))
    return {};
  symbol name{tokens.front()};
  tokens = tokens.remove_prefix(1);
  return {{ std::make_unique<variable>(name), tokens }};
}

// }}}
// Literals {{{

parse_res<> parse_symbol(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "'")
    return {};
  symbol name{tokens[1]};
  tokens = tokens.remove_prefix(2); // ''' name
  return {{ std::make_unique<literal::symbol>( name ), tokens }};
}

parse_res<> parse_integer(vector_ref<std::string> tokens)
{
  // parse_float should already be called by the time we get here, so assume
  // we're an integer
  if (!tokens.size() || !isdigit(tokens.front().front()))
    return {};
  auto str = tokens.front();
  tokens = tokens.remove_prefix(1); // number

  int val{};
  if (str.front() == '0' && str.size() > 1) {
    switch (str[1]) {
    case 'x': val = stoi(str.substr(2), nullptr, 16); break;
    case 'b': val = stoi(str.substr(2), nullptr, 2);  break;
    default:  val = stoi(str.substr(1), nullptr, 8);  break;
    }
  } else {
    val = stoi(str);
  }
  return {{ std::make_unique<literal::integer>( stoi(str) ), tokens }};
}

parse_res<> parse_float(vector_ref<std::string> tokens)
{
  if (!tokens.size() || !isdigit(tokens.front().front()))
    return {};
  auto str = tokens.front();
  tokens = tokens.remove_prefix(1); // number

  // Could this instead call parse_integer? Yes. Would that make more sense?
  // Probably. Oh well.
  if (find(begin(str), end(str), '.') == end(str))
    return {};
  return {{ std::make_unique<literal::floating_point>( stod(str) ), tokens }};
}

parse_res<> parse_bool(vector_ref<std::string> tokens)
{
  if (!tokens.size() || (tokens.front() != "true" && tokens.front() != "false"))
    return {};
  bool value{tokens.front() == "true"};
  tokens = tokens.remove_prefix(1); // value
  return {{ std::make_unique<literal::boolean>( value ), tokens }};
}

parse_res<> parse_nil(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "nil")
    return {};
  tokens = tokens.remove_prefix(1); // 'nil'
  return {{ std::make_unique<literal::nil>( ), tokens }};
}

parse_res<> parse_string(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front().front() != '"')
    return {};
  // inc/decrement to remove quotes
  std::string val{++begin(tokens.front()), --end(tokens.front())};
  tokens = tokens.remove_prefix(1); // val
  return {{ std::make_unique<literal::string>( val ), tokens }};
}

// }}}
// Helpers {{{

template <typename F>
auto parse_comma_separated_list(vector_ref<std::string> tokens,
                               const F& parse_item)
    -> parse_res<std::vector<decltype(parse_item(tokens)->first)>>
{
  tokens = ltrim(tokens, {"\n"});
  std::vector<decltype(parse_item(tokens)->first)> items;
  decltype(parse_item(tokens)) item_res = parse_item(tokens);
  while (item_res) {
    items.push_back(std::move(item_res->first));
    tokens = item_res->second;
    if (!tokens.size() || tokens.front() != ",")
      return {{ move(items), tokens }};
    tokens = ltrim(tokens.remove_prefix(1), {"\n"}); // ','
    item_res = parse_item(tokens);
  }
  return {{ move(items), tokens }};
}

template <typename F>
auto parse_bracketed_subexpr(vector_ref<std::string> tokens,
                             const F& parse_item,
                             const std::string& opening,
                             const std::string&)
    -> decltype(parse_item(tokens))
{
  if (!tokens.size() || tokens.front() != opening)
    return {};
  tokens = tokens.remove_prefix(1); // opening
  auto res = parse_item(tokens);
  res->second = res->second.remove_prefix(1); // closing
  return res;
}

parse_res<arg_t> parse_function_call(vector_ref<std::string> tokens)
{
  return parse_bracketed_subexpr(tokens, [](auto t)
  {
    return parse_comma_separated_list(t, parse_expression);
  },"(", ")");
}

parse_res<std::pair<std::unique_ptr<expression>, std::unique_ptr<expression>>>
  parse_cond_pair(vector_ref<std::string> tokens)
{
  auto test_res = parse_expression(tokens);
  if (!test_res)
    return {};
  auto test = move(test_res->first);
  tokens = test_res->second.remove_prefix(1); // ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ make_pair(move(test), move(body)), tokens }};
}

// Syntactically identical to (named) function definitions, but the returned
// result is different
parse_res<std::pair<symbol, std::unique_ptr<function_definition>>>
  parse_method_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "fn")
    return {};
  tokens = tokens.remove_prefix(1);

  symbol name{""};
  if (tokens.front() != ":") {
    name = tokens.front();
    tokens = tokens.remove_prefix(1);
  }

  auto arg_res = parse_bracketed_subexpr(tokens, [](auto tokens)
  {
    return parse_comma_separated_list(tokens, [](auto t)
    {
      if (t.front() == ")")
        return parse_res<symbol>{};
      return parse_res<symbol>{{ symbol{t.front()}, t.remove_prefix(1) }};
    });
  }, "(", ")");
  auto args = move(arg_res->first);
  tokens = arg_res->second;

  auto body_res = parse_expression(tokens.remove_prefix(1)); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ make_pair(name, std::make_unique<function_definition>( symbol{""},
                                                                   move(body),
                                                                   args )),
            tokens }};
}

// }}}

// }}}

}

std::vector<std::unique_ptr<expression>> parser::parse(token_string tokens)
{
  std::vector<std::unique_ptr<expression>> expressions;
  tokens = ltrim(tokens, {"\n"});

  while (tokens.size()) {
    auto res = parse_expression(tokens);
    expressions.push_back(move(res->first));
    tokens = res->second;
    tokens = ltrim(res->second, {"\n"});
  }
  return expressions;
}

// }}}
