#include "parser.h"

#include "builtins.h"
#include "utils.h"
#include "ast/assignment.h"
#include "ast/array.h"
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
#include "ast/object_creation.h"
#include "ast/require.h"
#include "ast/return_statement.h"
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

/// Implements a fairly simple recursive-descent parser

using namespace vv;
using namespace ast;

// Parsing {{{

namespace {

bool trim_test(const std::string& s)
{
  return s=="\n" || s==";";
}

// Declarations {{{

template <typename T = std::unique_ptr<expression>>
using parse_res = boost::optional<std::pair<T, vector_ref<std::string>>>;

using arg_t = std::vector<std::unique_ptr<expression>>;

parse_res<> parse_expression(vector_ref<std::string> tokens);

parse_res<> parse_nonop_expression(vector_ref<std::string> tokens);
parse_res<> parse_prec0(vector_ref<std::string> tokens); // member, call, index
parse_res<> parse_prec1(vector_ref<std::string> tokens); // monops
parse_res<> parse_prec2(vector_ref<std::string> tokens); // **
parse_res<> parse_prec3(vector_ref<std::string> tokens); // *, /, %
parse_res<> parse_prec4(vector_ref<std::string> tokens); // +, -
parse_res<> parse_prec5(vector_ref<std::string> tokens); // <<, >>
parse_res<> parse_prec6(vector_ref<std::string> tokens); // &
parse_res<> parse_prec7(vector_ref<std::string> tokens); // ^
parse_res<> parse_prec8(vector_ref<std::string> tokens); // |
parse_res<> parse_prec9(vector_ref<std::string> tokens); // to
parse_res<> parse_prec10(vector_ref<std::string> tokens); // <, >, <=, =>
parse_res<> parse_prec11(vector_ref<std::string> tokens); // ==, !=
parse_res<> parse_prec12(vector_ref<std::string> tokens); // &&
parse_res<> parse_prec13(vector_ref<std::string> tokens); // ||

parse_res<> parse_array_literal(vector_ref<std::string> tokens);
parse_res<> parse_assignment(vector_ref<std::string> tokens);
parse_res<> parse_block(vector_ref<std::string> tokens);
parse_res<> parse_cond_statement(vector_ref<std::string> tokens);
parse_res<> parse_except(vector_ref<std::string> tokens);
parse_res<> parse_for_loop(vector_ref<std::string> tokens);
parse_res<> parse_function_definition(vector_ref<std::string> tokens);
parse_res<> parse_literal(vector_ref<std::string> tokens);
parse_res<> parse_new_obj(vector_ref<std::string> tokens);
parse_res<> parse_require(vector_ref<std::string> tokens);
parse_res<> parse_return(vector_ref<std::string> tokens);
parse_res<> parse_try_catch(vector_ref<std::string> tokens);
parse_res<> parse_type_definition(vector_ref<std::string> tokens);
parse_res<> parse_variable_declaration(vector_ref<std::string> tokens);
parse_res<> parse_variable(vector_ref<std::string> tokens);
parse_res<> parse_while_loop(vector_ref<std::string> tokens);

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
parse_res<std::pair<symbol, function_definition>>
  parse_method_definition(vector_ref<std::string> tokens);
// }}}
// Individual parsing functions {{{

parse_res<> parse_expression(vector_ref<std::string> tokens)
{
  return parse_prec13(tokens);
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

    auto right_res = post(tokens.subvec(1)); // method (i.e. binop)
    auto right = move(right_res->first);
    tokens = right_res->second;

    auto member = std::make_unique<ast::member>( move(left), method );
    arg_t arg{};
    arg.emplace_back(move(right));

    return {{std::make_unique<function_call>(move(member), move(arg)), tokens}};
  }
  return left_res;
}

parse_res<> parse_prec13(vector_ref<std::string> tokens)
{
  auto left_res = parse_prec12(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  if (tokens.size() && tokens.front() == "||") {
    auto left = move(left_res->first);

    auto right_res = parse_prec13(tokens.subvec(1)); // '||'
    auto right = move(right_res->first);
    tokens = right_res->second;

    return {{ std::make_unique<logical_or>(move(left), move(right)), tokens }};
  }
  return left_res;
}

parse_res<> parse_prec12(vector_ref<std::string> tokens)
{
  auto left_res = parse_prec11(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  if (tokens.size() && tokens.front() == "&&") {
    auto left = move(left_res->first);
    auto right_res = parse_prec12(tokens.subvec(1)); // '&&'
    auto right = move(right_res->first);
    tokens = right_res->second;

    return {{ std::make_unique<logical_and>(move(left), move(right)), tokens }};
  }
  return left_res;
}

parse_res<> parse_prec11(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec10, parse_prec11,
                             [](const auto& s)
                             {
                               return s == "==" || s == "!=";
                             },
                             [](const auto& s)
                             {
                               return (s == "==") ? "equals" : "unequal";
                             });
}

parse_res<> parse_prec10(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec9, parse_prec10,
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
                                      (s == ">=") ? "greater_equals" :
                                                    "less_equals";
                             });
}

parse_res<> parse_prec9(vector_ref<std::string> tokens)
{
  auto left_res = parse_prec8(tokens);
  if (!left_res)
    return left_res;
  tokens = left_res->second;

  if (tokens.size() && tokens.front() == "to") {
    auto left = move(left_res->first);
    auto right_res = parse_prec9(tokens.subvec(1)); // 'to'
    auto right = move(right_res->first);
    tokens = right_res->second;

    arg_t args;
    args.emplace_back(move(left));
    args.emplace_back(move(right));

    auto range = std::make_unique<variable>( symbol{"Range"} );

    return {{std::make_unique<object_creation>(move(range), move(args)), tokens}};
  }
  return left_res;
}

parse_res<> parse_prec8(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec7, parse_prec8,
                             [](const auto& s) { return s == "|"; },
                             [](const auto&) { return "bitor"; });
}

parse_res<> parse_prec7(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec6, parse_prec7,
                             [](const auto& s) { return s == "^"; },
                             [](const auto&) { return "xor"; });
}

parse_res<> parse_prec6(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec5, parse_prec6,
                             [](const auto& s) { return s == "&"; },
                             [](const auto&) { return "bitand"; });
}

parse_res<> parse_prec5(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec4, parse_prec5,
                             [](const auto& s)
                             {
                               return s == ">>" || s == "<<";
                             },
                             [](const auto& s)
                             {
                               return (s == ">>") ? "rshift" : "lshift";
                             });
}

parse_res<> parse_prec4(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec3, parse_prec4,
                             [](const auto& s) { return s == "+" || s == "-"; },
                             [](const auto& s)
                             {
                               return (s == "+") ? "add" : "subtract";
                             });
}

parse_res<> parse_prec3(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec2, parse_prec3,
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

parse_res<> parse_prec2(vector_ref<std::string> tokens)
{
  return parse_operator_expr(tokens, parse_prec1, parse_prec2,
                             [](const auto& s) { return s == "**"; },
                             [](const auto&) { return "pow"; });
}

parse_res<> parse_prec1(vector_ref<std::string> tokens)
{
  if (tokens.size() && (tokens.front() == "!"
                     || tokens.front() == "~"
                     || tokens.front() == "-")) {


    symbol method{(tokens.front() == "!") ? "not" :
                  (tokens.front() == "~") ? "negate" :
                                            "negative"};
    auto expr_res = parse_prec1(tokens.subvec(1)); // monop
    tokens = expr_res->second;
    auto expr = move(expr_res->first);
    auto member = std::make_unique<ast::member>( move(expr), method );

    return {{ std::make_unique<function_call>( move(member), arg_t{} ),
              tokens }};
  }
  return parse_prec0(tokens);
}

// TODO: split function calls, members, and indexing into their own functions so
// this one isn't so monstruously long
parse_res<> parse_prec0(vector_ref<std::string> tokens)
{
  auto expr_res = parse_nonop_expression(tokens);
  if (!expr_res)
    return expr_res;
  tokens = expr_res->second;

  auto expr = move(expr_res->first);
  while (tokens.size() && (tokens.front() == "("
                        || tokens.front() == "."
                        || tokens.front() == "[")) {
    if (tokens.front() == "(") {
      auto list_res = parse_function_call(tokens);
      auto list = move(list_res->first);
      tokens = list_res->second;

      expr = std::make_unique<function_call>(move(expr), move(list));

    } else if (tokens.front() == "[") {
      auto idx_res = parse_bracketed_subexpr(tokens, parse_expression, "[", "]");
      auto idx = move(idx_res->first);
      tokens = idx_res->second;

      if (tokens.size() && tokens.front() == "=") {
        auto value_res = parse_expression(tokens.subvec(1)); // '='
        auto value = move(value_res->first);
        tokens = value_res->second;

        auto member = std::make_unique<ast::member>( move(expr), symbol{"set_at"} );
        arg_t args{};
        args.emplace_back(move(idx));
        args.emplace_back(move(value));

        return {{ std::make_unique<function_call>( move(member), move(args) ),
                  tokens }};
      }

      auto member = std::make_unique<ast::member>( move(expr), symbol{"at"} );
      arg_t arg{};
      arg.emplace_back(move(idx));
      expr = std::make_unique<function_call>(move(member), move(arg));

    } else {
      tokens = tokens.subvec(1); // '.'
      symbol name{tokens.front()};
      tokens = tokens.subvec(1); // name

      if (tokens.size() && tokens.front() == "=") {
        auto value_res = parse_expression(tokens.subvec(1)); // '='
        auto value = move(value_res->first);
        tokens = value_res->second;
        return {{ std::make_unique<member_assignment>( move(expr),
                                                       name,
                                                       move(value) ),
                  tokens }};
      }
      expr = std::make_unique<member>( move(expr), name );
    }
  }
  return {{ move(expr), tokens }};
}

parse_res<> parse_nonop_expression(vector_ref<std::string> tokens)
{
  if (tokens.size() && tokens.front() == "(")
    return parse_bracketed_subexpr(tokens, parse_expression, "(", ")");

  parse_res<> res;
  if ((res = parse_array_literal(tokens)))        return res;
  if ((res = parse_assignment(tokens)))           return res;
  if ((res = parse_block(tokens)))                return res;
  if ((res = parse_cond_statement(tokens)))       return res;
  if ((res = parse_except(tokens)))               return res;
  if ((res = parse_for_loop(tokens)))             return res;
  if ((res = parse_function_definition(tokens)))  return res;
  if ((res = parse_literal(tokens)))              return res;
  if ((res = parse_new_obj(tokens)))              return res;
  if ((res = parse_require(tokens)))              return res;
  if ((res = parse_return(tokens)))               return res;
  if ((res = parse_try_catch(tokens)))            return res;
  if ((res = parse_type_definition(tokens)))      return res;
  if ((res = parse_variable_declaration(tokens))) return res;
  if ((res = parse_while_loop(tokens)))           return res;
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
  auto expr_res = parse_expression(tokens.subvec(2)); // name '='
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<assignment>( name, move(expr) ), tokens }};
}

parse_res<> parse_block(vector_ref<std::string> tokens)
{
  const static auto trim_test = [](const auto& s) { return s=="\n" || s==";"; };

  if (!tokens.size() || tokens.front() != "do")
    return {};
  tokens = tokens.subvec(1); // 'do'

  std::vector<std::unique_ptr<expression>> subexprs;
  tokens = ltrim_if(tokens, trim_test);
  while (tokens.front() != "end") {
    auto expr_res = parse_expression(tokens);
    subexprs.push_back(move(expr_res->first));
    tokens = expr_res->second;
    tokens = ltrim_if(tokens, trim_test);
  }

  tokens = tokens.subvec(1); // 'end'
  return {{ std::make_unique<block>( move(subexprs) ), tokens }};
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

  return {{ std::make_unique<array>( move(vals) ), tokens }};
}

parse_res<> parse_cond_statement(vector_ref<std::string> tokens)
{
  if (!tokens.size() || (tokens.front() != "cond" && tokens.front() != "if"))
    return {};
  tokens = ltrim(tokens.subvec(1), {"\n"});

  auto pairs_res = parse_comma_separated_list(tokens, parse_cond_pair);
  auto pairs = move(pairs_res->first);
  tokens = pairs_res->second;

  return {{ std::make_unique<cond_statement>(move(pairs)), tokens }};
}

parse_res<> parse_except(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "except")
    return {};
  tokens = tokens.subvec(1); // 'except'
  auto expr_res = parse_expression(tokens);
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<except>( move(expr) ), tokens }};
}

parse_res<> parse_for_loop(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "for")
    return {};
  tokens = tokens.subvec(1); // 'for'

  auto iterator = tokens.front();
  tokens = tokens.subvec(2); // iterator 'in'

  auto range_res = parse_expression(tokens);
  auto range = move(range_res->first);
  tokens = range_res->second.subvec(1); // ':'

  auto body_res = parse_expression(tokens); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_unique<for_loop>( iterator, move(range), move(body) ),
            tokens }};
}

parse_res<> parse_function_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "fn")
    return {};
  tokens = tokens.subvec(1);

  symbol name;
  if (tokens.front() != "(") {
    name = tokens.front();
    tokens = tokens.subvec(1); // name
  }

  auto arg_res = parse_bracketed_subexpr(tokens, [](auto tokens)
  {
    return parse_comma_separated_list(tokens, [](auto t)
    {
      if (t.front() == ")")
        return parse_res<symbol>{};
      return parse_res<symbol>{{ symbol{t.front()}, t.subvec(1) }}; // arg name
    });
  }, "(", ")");
  auto args = move(arg_res->first);
  tokens = arg_res->second;

  auto body_res = parse_expression(tokens.subvec(1)); // ':'
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

parse_res<> parse_new_obj(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "new")
    return {};
  tokens = tokens.subvec(1); // 'new'

  auto type_res = parse_variable(tokens);
  auto type = move(type_res->first);
  tokens = type_res->second;
  tokens = type_res->second;

  auto args_res = parse_function_call(tokens);
  auto args = move(args_res->first);
  tokens = args_res->second;

  return {{ std::make_unique<object_creation>( move(type), move(args) ),
            tokens }};
}

parse_res<> parse_require(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "require")
    return {};
  tokens = tokens.subvec(1); // 'require'
  std::string filename{++begin(tokens.front()), --end(tokens.front())};
  tokens = tokens.subvec(1); // filename
  return {{ std::make_unique<require>( filename ), tokens }};
}

parse_res<> parse_return(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "return")
    return {};
  tokens = tokens.subvec(1); // 'except'
  auto expr_res = parse_expression(tokens);
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<return_statement>( move(expr) ), tokens }};
}

parse_res<> parse_try_catch(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "try")
    return {};
  tokens = tokens.subvec(2); // 'try' ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;

  tokens = ltrim(tokens, {"\n"});
  tokens = tokens.subvec(1); // 'catch'
  symbol exception_name{tokens.front()};
  std::vector<symbol> exception_arg{exception_name};
  tokens = tokens.subvec(2); // name ':

  auto catcher_res = parse_expression(tokens);
  auto catcher = move(catcher_res->first);
  tokens = catcher_res->second;

  return {{std::make_unique<try_catch>(move(body),exception_name,move(catcher)),
           tokens}};
}

parse_res<> parse_type_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "class")
    return{};
  symbol name{tokens[1]};
  tokens = tokens.subvec(2); // 'class' name

  symbol parent{"Object"};
  if (tokens.front() == ":") {
    parent = tokens[1];
    tokens = tokens.subvec(2); // ':' parent
  }

  std::unordered_map<symbol, function_definition> method_map;
  tokens = ltrim_if(tokens, trim_test);
  while (tokens.front() != "end") {
    auto method = parse_method_definition(tokens);
    method_map.insert(std::make_pair(method->first.first,
                                     method->first.second));
    tokens = ltrim_if(method->second, trim_test);
  }
  tokens = tokens.subvec(1); // 'end'

  return {{ std::make_unique<type_definition>( name, parent, move(method_map) ),
            tokens }};
}

parse_res<> parse_variable_declaration(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "let")
    return {};
  auto name = tokens[1];
  auto expr_res = parse_expression(tokens.subvec(3)); // 'let' name '='
  auto expr = move(expr_res->first);
  tokens = expr_res->second;
  return {{ std::make_unique<variable_declaration>(name, move(expr)), tokens }};
}

parse_res<> parse_variable(vector_ref<std::string> tokens)
{
  if (!tokens.size() || !isnamechar(tokens.front().front()))
    return {};
  symbol name{tokens.front()};
  tokens = tokens.subvec(1); // name
  return {{ std::make_unique<variable>(name), tokens }};
}

parse_res<> parse_while_loop(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "while")
    return {};

  auto test_res = parse_expression(tokens.subvec(1)); // 'while'
  auto test = move(test_res->first);
  tokens = test_res->second;

  auto body_res = parse_expression(tokens.subvec(1)); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_unique<while_loop>( move(test), move(body) ), tokens }};
}

// }}}
// Literals {{{

parse_res<> parse_symbol(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "'")
    return {};
  symbol name{tokens[1]};
  tokens = tokens.subvec(2); // ''' name
  return {{ std::make_unique<literal::symbol>( name ), tokens }};
}

parse_res<> parse_integer(vector_ref<std::string> tokens)
{
  // parse_float should already be called by the time we get here, so assume
  // we're an integer
  if (!tokens.size() || !isdigit(tokens.front().front()))
    return {};
  auto str = tokens.front();
  tokens = tokens.subvec(1); // number

  return {{ std::make_unique<literal::integer>( to_int(str) ), tokens }};
}

parse_res<> parse_float(vector_ref<std::string> tokens)
{
  if (!tokens.size() || !isdigit(tokens.front().front()))
    return {};
  auto str = tokens.front();
  tokens = tokens.subvec(1); // number

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
  tokens = tokens.subvec(1); // value
  return {{ std::make_unique<literal::boolean>( value ), tokens }};
}

parse_res<> parse_nil(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "nil")
    return {};
  tokens = tokens.subvec(1); // 'nil'
  return {{ std::make_unique<literal::nil>( ), tokens }};
}

parse_res<> parse_string(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front().front() != '"')
    return {};
  // inc/decrement to remove quotes
  std::string val{++begin(tokens.front()), --end(tokens.front())};
  tokens = tokens.subvec(1); // val
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
    tokens = ltrim(tokens.subvec(1), {"\n"}); // ','
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
  tokens = ltrim(tokens.subvec(1), {"\n"}); // opening
  auto res = parse_item(tokens);
  res->second = ltrim(res->second, {"\n"}).subvec(1); // closing
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
  tokens = test_res->second.subvec(1); // ':'

  auto body_res = parse_expression(tokens);
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ make_pair(move(test), move(body)), tokens }};
}

// Syntactically identical to (named) function definitions, but the returned
// result is different
parse_res<std::pair<symbol, function_definition>>
  parse_method_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "fn")
    return {};
  tokens = tokens.subvec(1); // 'fn'

  symbol name{tokens.front()};
  tokens = tokens.subvec(1); // name

  auto arg_res = parse_bracketed_subexpr(tokens, [](auto tokens)
  {
    return parse_comma_separated_list(tokens, [](auto t)
    {
      if (t.front() == ")")
        return parse_res<symbol>{};
      return parse_res<symbol>{{ symbol{t.front()}, t.subvec(1) }}; // argname
    });
  }, "(", ")");
  auto args = move(arg_res->first);
  tokens = arg_res->second;

  auto body_res = parse_expression(tokens.subvec(1)); // ':'
  auto body = move(body_res->first);
  tokens = body_res->second;

  return {{ std::make_pair( name,function_definition{ {}, move(body), args} ),
            tokens}};
}

// }}}

// }}}

}

std::vector<std::unique_ptr<expression>> parser::parse(token_string tokens)
{
  std::vector<std::unique_ptr<expression>> expressions;
  tokens = ltrim_if(tokens, trim_test);

  while (tokens.size()) {
    auto res = parse_expression(tokens);
    expressions.push_back(move(res->first));
    tokens = res->second;
    tokens = ltrim_if(res->second, trim_test);
  }
  return expressions;
}

// }}}
