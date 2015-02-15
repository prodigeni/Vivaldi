#include "parser.h"
#include <iostream>

using namespace vv;
using namespace parser;

namespace {

// Validates a string of tokens via recursive descent, so that when we're
// parsing it properly we don't have to worry about malformed syntax

bool trim_test(const std::string& s)
{
  return s == "\n" || s == ";";
};


val_res val_toplevel(vector_ref<std::string> tokens);

val_res val_expression(vector_ref<std::string> tokens); // infix binop
val_res val_monop(vector_ref<std::string> tokens); // prefix monop
val_res val_accessor(vector_ref<std::string> tokens); // member, call, or index
val_res val_noop(vector_ref<std::string> tokens); // any other expression

val_res val_array_literal(vector_ref<std::string> tokens);
val_res val_dict_literal(vector_ref<std::string> tokens);
val_res val_assignment(vector_ref<std::string> tokens);
val_res val_block(vector_ref<std::string> tokens);
val_res val_cond_statement(vector_ref<std::string> tokens);
val_res val_except(vector_ref<std::string> tokens);
val_res val_for_loop(vector_ref<std::string> tokens);
val_res val_function_definition(vector_ref<std::string> tokens);
val_res val_literal(vector_ref<std::string> tokens);
val_res val_new_obj(vector_ref<std::string> tokens);
val_res val_require(vector_ref<std::string> tokens);
val_res val_return(vector_ref<std::string> tokens);
val_res val_try_catch(vector_ref<std::string> tokens);
val_res val_type_definition(vector_ref<std::string> tokens);
val_res val_variable_declaration(vector_ref<std::string> tokens);
val_res val_variable(vector_ref<std::string> tokens);
val_res val_while_loop(vector_ref<std::string> tokens);

// Literals
val_res val_bool(vector_ref<std::string> tokens);
val_res val_float(vector_ref<std::string> tokens);
val_res val_integer(vector_ref<std::string> tokens);
val_res val_nil(vector_ref<std::string> tokens);
val_res val_string(vector_ref<std::string> tokens);
val_res val_symbol(vector_ref<std::string> tokens);

// Helpers
template <typename F>
val_res val_comma_separated_list(vector_ref<std::string> tokens,
                                 const F& val_item);
template <typename F>
val_res val_bracketed_subexpr(vector_ref<std::string> tokens,
                              const F& val_item,
                              const std::string& opening,
                              const std::string& closing);
// a: b pair, as in dicts or cond statements
val_res val_pair(vector_ref<std::string> tokens);

// Definitions

val_res val_toplevel(parser::token_string tokens)
{
  tokens = ltrim_if(tokens, trim_test);

  while (tokens.size()) {
    auto res = val_expression(tokens);
    if (!res)
      return res.invalid() ? res : tokens;
    tokens = ltrim_if(*res, trim_test);
  }
  return tokens;
}

val_res val_expression(vector_ref<std::string> tokens)
{
  auto res = val_monop(tokens);
  if (!res)
    return res;
  tokens = *res;
  if (!tokens.size())
    return tokens;
  if (tokens.front() == "||" ||
      tokens.front() == "&&" ||
      tokens.front() == "==" ||
      tokens.front() == "!=" ||
      tokens.front() == ">=" ||
      tokens.front() == "<=" ||
      tokens.front() == "<"  ||
      tokens.front() == ">"  ||
      tokens.front() == "to" ||
      tokens.front() == "|"  ||
      tokens.front() == "^"  ||
      tokens.front() == "&"  ||
      tokens.front() == "<<" ||
      tokens.front() == ">>" ||
      tokens.front() == "+"  ||
      tokens.front() == "-"  ||
      tokens.front() == "*"  ||
      tokens.front() == "/"  ||
      tokens.front() == "%"  ||
      tokens.front() == "**")
    return val_expression(tokens.subvec(1)); // operator
  return res;
}

val_res val_monop(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  if (tokens.front() == "!" || tokens.front() == "~" || tokens.front() == "-")
    return val_monop(tokens.subvec(1)); // operator
  return val_accessor(tokens);
}

val_res val_accessor(vector_ref<std::string> tokens)
{
  auto base = val_noop(tokens);
  if (!base || !base->size())
    return base;
  tokens = *base;
  while (tokens.size() && (tokens.front() == "("
                        || tokens.front() == "["
                        || tokens.front() == ".")) {

    if (tokens.front() == "(") {
      auto args = val_bracketed_subexpr(tokens, [](auto t)
      {
        return val_comma_separated_list(t, val_expression);
      }, "(", ")");
      if (args.invalid())
        return args;
      if (!args)
        return {tokens.subvec(1), "expected argument list"};
      tokens = *args;
    } else {

      val_res res;
      if (tokens.front() == "[") {
         res = val_bracketed_subexpr(tokens, val_expression, "[", "]");
        if (res.invalid())
          return res;
        if (!res)
          return {tokens.subvec(1), "expected index expression"}; // '['
      } else if (tokens.front() == ".") {
        res = val_variable(tokens.subvec(1)); // '.'
        if (res.invalid())
          return res;
        if (!res)
          return {tokens.subvec(1), "expected member name"}; // '.'
      }
      tokens = *res;
      if (tokens.size() && tokens.front() == "=") {
        auto expr = val_expression(tokens.subvec(1)); // '='
        if (expr || expr.invalid())
          return expr;
        return {tokens.subvec(1), "expected assignment expression"};
      }
    }
  }
  return tokens;
}

val_res val_noop(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  if (tokens.front() == "(") {
    auto expr = val_bracketed_subexpr(tokens, val_expression, "(", ")");
    if (expr || expr.invalid())
      return expr;
    return {tokens.subvec(1), "expected expression in parentheses"}; // ')'
  }

  val_res res;
  if ((res = val_array_literal(tokens))        || res.invalid()) return res;
  if ((res = val_assignment(tokens))           || res.invalid()) return res;
  if ((res = val_block(tokens))                || res.invalid()) return res;
  if ((res = val_cond_statement(tokens))       || res.invalid()) return res;
  if ((res = val_dict_literal(tokens))         || res.invalid()) return res;
  if ((res = val_except(tokens))               || res.invalid()) return res;
  if ((res = val_for_loop(tokens))             || res.invalid()) return res;
  if ((res = val_function_definition(tokens))  || res.invalid()) return res;
  if ((res = val_literal(tokens))              || res.invalid()) return res;
  if ((res = val_new_obj(tokens))              || res.invalid()) return res;
  if ((res = val_require(tokens))              || res.invalid()) return res;
  if ((res = val_return(tokens))               || res.invalid()) return res;
  if ((res = val_try_catch(tokens))            || res.invalid()) return res;
  if ((res = val_type_definition(tokens))      || res.invalid()) return res;
  if ((res = val_variable_declaration(tokens)) || res.invalid()) return res;
  if ((res = val_while_loop(tokens))           || res.invalid()) return res;
  // has to come last, since most keywords would make valid names
  if ((res = val_variable(tokens))             || res.invalid()) return res;
  return {};
}

val_res val_array_literal(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "[")
    return {};
  auto body = val_bracketed_subexpr(tokens, [](auto t)
  {
    return val_comma_separated_list(t, val_expression);
  }, "[", "]");
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected array literal"};
}

val_res val_dict_literal(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "{")
    return {};
  auto body = val_bracketed_subexpr(tokens, [](auto t)
  {
    return val_comma_separated_list(t, val_pair);
  }, "{", "}");
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected dictionary literal"};
}

val_res val_assignment(vector_ref<std::string> tokens)
{
  auto expr = val_variable(tokens);
  if (!expr || !expr->size() || expr->front() != "=")
    return !expr ? expr : val_res{};
  tokens = expr->subvec(1); // '='
  auto val = val_expression(tokens);
  if (val || val.invalid())
    return val;
  return {tokens, "expected expression"};
}

val_res val_block(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "do")
    return {};
  tokens = tokens.subvec(1); // 'do'
  tokens = ltrim_if(tokens, trim_test);

  while (tokens.size() && tokens.front() != "end") {
    auto res = val_expression(tokens);
    if (res.invalid())
      return res;
    if (!res)
      return {tokens, "expected expression or 'end'"};
    tokens = ltrim_if(*res, trim_test);
  }
  if (!tokens.size())
    return {tokens, "expected 'end'"};
  return tokens.subvec(1); // 'end'
}

val_res val_cond_statement(vector_ref<std::string> tokens)
{
  if (!tokens.size() || (tokens.front() != "cond" && tokens.front() != "if"))
    return {};
  tokens = ltrim(tokens.subvec(1), {"\n"}); // 'cond'
  auto body = val_comma_separated_list(tokens, val_pair);
  if (body || body.invalid())
    return body;
  return {tokens, "expected cond pair"};
}

val_res val_except(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "except")
    return {};
  tokens = tokens.subvec(1); // 'except'
  auto body = val_expression(tokens);
  if (body || body.invalid())
    return body;
  return {tokens, "expected expression"};
}

val_res val_for_loop(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "for")
    return {};

  auto iter = val_variable(tokens.subvec(1)); // 'for'
  if (!iter)
    return {tokens.subvec(1), "expected variable name"}; // 'for'
  tokens = *iter;
  if (!tokens.size() || tokens.front() != "in")
    return {tokens, "expected 'in'"};

  auto range = val_expression(tokens.subvec(1)); // 'in'
  if (range.invalid())
    return range;
  if (!range)
    return {tokens.subvec(1), "expected expression"}; // 'in'
  tokens = *range;

  if (!tokens.size() || tokens.front() != ":")
    return {tokens, "expected ':'"};
  auto body = val_expression(tokens.subvec(1)); // ':'
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected expression"}; // ':'
}

val_res val_function_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "fn")
    return {};
  tokens = tokens.subvec(1); // 'fn'

  if (auto name = val_variable(tokens))
    tokens = *name;

  auto arglist = val_bracketed_subexpr(tokens, [](auto t)
  {
    return val_comma_separated_list(t, val_variable);
  }, "(", ")");
  if (arglist.invalid())
    return arglist;
  if (!arglist)
    return {tokens, "expected argument list"};
  tokens = *arglist;

  if (!tokens.size() || tokens.front() != ":")
    return {tokens, "expected ':'"};

  auto body = val_expression(tokens.subvec(1)); // ':'
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected expression"}; // ':'
}

val_res val_literal(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  val_res res;
  if ((res = val_bool(tokens))    || res.invalid()) return res;
  if ((res = val_float(tokens))   || res.invalid()) return res;
  if ((res = val_integer(tokens)) || res.invalid()) return res;
  if ((res = val_nil(tokens))     || res.invalid()) return res;
  if ((res = val_string(tokens))  || res.invalid()) return res;
  if ((res = val_symbol(tokens))  || res.invalid()) return res;
  return {};
}

val_res val_new_obj(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "new")
    return {};
  tokens = tokens.subvec(1); // 'new'

  auto name = val_variable(tokens);
  if (!name)
    return {tokens, "expected variable name"};
  tokens = *name;

  auto args = val_bracketed_subexpr(tokens, [](auto t)
  {
    return val_comma_separated_list(t, val_expression);
  }, "(", ")");
  if (args || args.invalid())
    return args;
  return {tokens, "expected argument list"};
}

val_res val_require(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "require")
    return {};
  auto value = val_string(tokens.subvec(1)); // 'require'
  if (value || value.invalid())
    return value;
  return {tokens.subvec(1), "expected expression"}; // 'require'
}

val_res val_return(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "return")
    return {};
  auto value = val_expression(tokens.subvec(1)); // 'return'
  if (value || value.invalid())
    return value;
  return {tokens.subvec(1), "expected expression"}; // 'return'
}

val_res val_try_catch(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "try")
    return {};
  if (tokens.size() < 2 || tokens[1] != ":")
    return {tokens.subvec(1), "expected ':'"}; // 'try'
  tokens = tokens.subvec(2); // 'try' ':'

  auto body = val_expression(tokens);
  if (body.invalid())
    return body;
  if (!body)
    return {tokens, "expected expression"};
  tokens = ltrim(*body, {"\n"});
  if (!tokens.size() || tokens.front() != "catch")
    return {tokens, "expected 'catch'"};
  auto name = val_variable(tokens.subvec(1)); // 'catch'
  if (!name)
    return {tokens, "expected variable name"};
  tokens = *name;

  if (!tokens.size() || tokens.front() != ":")
    return {tokens, "expected ':'"};
  tokens = tokens.subvec(1); // ':'

  auto catcher = val_expression(tokens);
  if (catcher || catcher.invalid())
    return catcher;
  return {tokens, "expected expression"};
}

val_res val_type_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "class")
    return {};
  auto name = val_variable(tokens.subvec(1)); // 'class'
  if (!name)
    return {tokens.subvec(1), "expected variable name"}; // 'class'
  tokens = *name;

  if (tokens.size() && tokens.front() == ":") {
    auto parent = val_variable(tokens.subvec(1)); // ':'
    if (!parent)
      return {tokens.subvec(1), "expected variable name"}; // ':'
    tokens = *parent;
  }

  tokens = ltrim_if(tokens, trim_test);
  while (tokens.size() && tokens.front() != "end") {
    auto next_fn = val_function_definition(tokens);
    if (next_fn.invalid())
      return next_fn;
    if (!next_fn)
      return {tokens, "expected method definition or 'end'"};
    tokens = ltrim_if(*next_fn, trim_test);
  }
  if (tokens.size())
    return tokens.subvec(1); // 'end'
  return {tokens, "expected 'end'"};
}

val_res val_variable_declaration(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "let")
    return {};
  auto name = val_variable(tokens.subvec(1)); // 'let'
  if (!name)
    return {tokens.subvec(1), "expected variable name"}; // 'let'
  tokens = *name;
  if (!tokens.size() || tokens.front() != "=")
    return {tokens, "expected '='"};
  auto value = val_expression(tokens.subvec(1)); // '='
  if (value || value.invalid())
    return value;
  return {tokens.subvec(1), "expected expression"}; // '='
}

val_res val_variable(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  if (isdigit(tokens.front().front()))
    return {};
  if (!all_of(begin(tokens.front()), end(tokens.front()), isnamechar))
    return {};
  return tokens.subvec(1); // variable
}

val_res val_while_loop(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "while")
    return {};
  tokens = tokens.subvec(1); // 'while'
  auto test = val_expression(tokens);
  if (test.invalid())
    return test;
  if (!test)
    return {tokens, "expected expression"};

  tokens = *test;
  if (!tokens.size() || tokens.front() != ":")
    return {tokens, "expected ':'"};
  auto body = val_expression(tokens.subvec(1)); // ':'
  if (body || body.invalid())
    return body;
  return {tokens.subvec(1), "expected expression"}; // ':'
}

val_res val_bool(vector_ref<std::string> tokens)
{
  if (!tokens.size() || (tokens.front() != "true" && tokens.front() != "false"))
    return {};
  return tokens.subvec(1); // 'true'/'false'
}

val_res val_float(vector_ref<std::string> tokens)
{
  if (!tokens.size() || !isdigit(tokens.front().front()))
    return {};
  // Most errors should be dealt with in tokenizing, so just ensure this really
  // is a float
  const auto& str = tokens.front();
  if (find(begin(str), end(str), '.') == end(str))
    return {};
  return tokens.subvec(1); // number
}

val_res val_integer(vector_ref<std::string> tokens)
{
  if (!tokens.size() || !isdigit(tokens.front().front()))
    return {};
  const auto& str = tokens.front();
  if (str.front() == '0' && str.size() > 1) {
    // all other errors should be dealt with in tokenizing
    if (!isdigit(str[1]) && str.size() == 2)
      return {tokens, "invalid number"};
  }
  return tokens.subvec(1); // number
}

val_res val_nil(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "nil")
    return {};
  return tokens.subvec(1); // 'nil'
}

val_res val_string(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front().front() != '"')
    return {};
  // How many ways are there to screw up a string, really?
  if (tokens.front().back() != '"')
    return {tokens, "invalid string"};
  return tokens.subvec(1); // string
}

val_res val_symbol(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "'")
    return {};
  return tokens.subvec(2); // '\'' symbol
}

template <typename F>
val_res val_comma_separated_list(vector_ref<std::string> tokens,
                                 const F& val_item)
{
  val_res res{};
  while ((res = val_item(tokens))) {
    tokens = *res;
    if (!tokens.size() || tokens.front() != ",")
      return tokens;
    tokens = ltrim(tokens.subvec(1), {"\n"}); // ','
  }
  if (res.invalid())
    return res;
  return tokens;
}

template <typename F>
val_res val_bracketed_subexpr(vector_ref<std::string> tokens,
                              const F& val_item,
                              const std::string& opening,
                              const std::string& closing)
{
  if (!tokens.size() || tokens.front() != opening)
    return {};
  tokens = ltrim_if(tokens.subvec(1), trim_test); // opening
  auto item = val_item(tokens);
  if (!item)
    return item;
  tokens = *item;
  if (!tokens.size() || tokens.front() != closing)
    return {tokens, "expected '" + closing + '\''};
  return tokens.subvec(1); // closing
}

val_res val_pair(vector_ref<std::string> tokens)
{
  auto left = val_expression(tokens);
  if (!left)
    return left;
  tokens = *left;
  if (!tokens.size() || tokens.front() != ":")
    return {tokens, "expected ':'"};
  auto right = val_expression(tokens.subvec(1)); // ':'
  if (right || right.invalid())
    return right;
  return {tokens.subvec(1), "expected expression"}; // ':'
}

}

val_res parser::is_valid(parser::token_string tokens)
{
  auto res = val_toplevel(tokens);
  if (res && res->size())
    return {*res, "expected end of input"};
  return res;
}
