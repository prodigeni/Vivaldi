#include "parser.h"

using namespace vv;
using namespace parser;

namespace {

template <typename F>
val_res val_comma_separated_list(vector_ref<std::string> tokens,
                                 const F& val_item);
template <typename F>
val_res val_bracketed_subexpr(vector_ref<std::string> tokens,
                              const F& val_item,
                              const std::string& opening,
                              const std::string& closing);

val_res val_expression(          vector_ref<std::string> tokens);
val_res val_assignment(          vector_ref<std::string> tokens);
val_res val_block(               vector_ref<std::string> tokens);
val_res val_cond_statement(      vector_ref<std::string> tokens);
val_res val_for_loop(            vector_ref<std::string> tokens);
val_res val_while_loop(          vector_ref<std::string> tokens);
val_res val_function_call(       vector_ref<std::string> tokens);
val_res val_monop_call(          vector_ref<std::string> tokens);
val_res val_binop_call(          vector_ref<std::string> tokens);
val_res val_member(              vector_ref<std::string> tokens);
val_res val_except(              vector_ref<std::string> tokens);
val_res val_expr_list(           vector_ref<std::string> tokens);
val_res val_function_definition( vector_ref<std::string> tokens);
val_res val_literal(             vector_ref<std::string> tokens);
val_res val_array_literal(       vector_ref<std::string> tokens);
val_res val_dict_literal(        vector_ref<std::string> tokens);
val_res val_try_catch(           vector_ref<std::string> tokens);
val_res val_type_definition(     vector_ref<std::string> tokens);
val_res val_variable_declaration(vector_ref<std::string> tokens);
val_res val_name(                vector_ref<std::string> tokens);

// comma-separated list {{{

template <typename F>
val_res val_comma_separated_list(vector_ref<std::string> tokens,
                                 const F& val_item)
{
  val_res expr_res{};
  if (!(expr_res = val_item(tokens)))
    return tokens;

  do {
    tokens = *expr_res;
    if (!tokens.size() || tokens.front() != ",")
      return tokens;
    tokens = tokens.remove_prefix(1);
  } while ((expr_res = val_item(tokens)));

  return expr_res;
}

// }}}
// bracketed subexpr {{{

template <typename F>
val_res val_bracketed_subexpr(vector_ref<std::string> tokens,
                              const F& val_item,
                              const std::string& opening,
                              const std::string& closing)
{
  if (tokens.size() && tokens.front() == opening) {
    auto item_res = val_item(tokens.remove_prefix(1));
    if (!item_res)
      return item_res;
    tokens = *item_res;
    if (tokens.size() && tokens.front() == closing)
      return tokens.remove_prefix(1);
    return {tokens, "expected '" + closing + '\''};
  }
  return {};
}

// }}}
// individual validating functions {{{

// expression {{{

val_res val_expression(vector_ref<std::string> tokens)
{
  if (tokens.size() && tokens.front() == "(")
    return val_bracketed_subexpr(tokens, val_expression, "(", ")");

  val_res res{};

  // Try each, and return on successful validation or on error
  if (!( ((res = val_assignment(tokens))           || res.invalid())
      || ((res = val_block(tokens))                || res.invalid())
      || ((res = val_cond_statement(tokens))       || res.invalid())
      || ((res = val_except(tokens))               || res.invalid())
      || ((res = val_for_loop(tokens))             || res.invalid())
      || ((res = val_while_loop(tokens))           || res.invalid())
      || ((res = val_function_definition(tokens))  || res.invalid())
      || ((res = val_monop_call(tokens))           || res.invalid())
      || ((res = val_literal(tokens))              || res.invalid())
      || ((res = val_dict_literal(tokens))         || res.invalid())
      || ((res = val_array_literal(tokens))        || res.invalid())
      || ((res = val_try_catch(tokens))            || res.invalid())
      || ((res = val_type_definition(tokens))      || res.invalid())
      || ((res = val_variable_declaration(tokens)) || res.invalid())
      || ((res = val_name(tokens)))                || res.invalid()))
    return res;
  if (res.invalid())
    return res;

  auto nres = res;
  for (;;) {
    res = nres;
    tokens = *res;
    if (!( (nres = val_function_call(tokens))
        || (nres = val_member(tokens))
        || (nres = val_binop_call(tokens))))
      return res;
  }
}

// }}}
// assignment {{{

val_res val_assignment(vector_ref<std::string> tokens)
{
  if (auto name_res = val_name(tokens)) {
    tokens = *name_res;
    if (tokens.size() < 2 || tokens.front() != "=")
      return {};
    tokens = tokens.remove_prefix(1);
    return val_expression(tokens);
  }
  return {};
}

// }}}
// block {{{

val_res val_block(vector_ref<std::string> tokens)
{
  if (tokens.size() < 2 || tokens.front() != "{")
    return {};
  tokens = tokens.remove_prefix(1);
  auto first_nonsep = std::find_if(begin(tokens), end(tokens),
                                   [](const auto& t)
                                     { return t != "\n" && t != ";"; });
  tokens = tokens.remove_prefix(first_nonsep - begin(tokens));
  while (tokens.size() && tokens.front() != "}") {
    val_res expr_res;
    if (!(expr_res = val_expression(tokens)))
      return {tokens, "expected expression"};
    tokens = *expr_res;
    if (tokens.size() && (tokens.front() == "\n" || tokens.front() == ";")) {
      auto first_nonsep = std::find_if(begin(tokens), end(tokens),
                                       [](const auto& t)
                                         { return t != "\n" && t != ";"; });
      tokens = tokens.remove_prefix(first_nonsep - begin(tokens));
    }
  }
  if (tokens.size() && tokens.front() == "}")
    return tokens.remove_prefix(1);
  return {tokens, "expected '}'"};
}

// }}}
// cond_statement {{{

val_res val_if_statement(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "if")
    return {};
  if (auto test_res = val_expression(tokens.remove_prefix(1))) {
    tokens = *test_res;
    if (tokens.size() && tokens.front() == ":") {
      auto val = val_expression(tokens.remove_prefix(1));
      if (val)
        return val;
      return {tokens, "expected expression"};
    }
    return {tokens, "expected ':'"};
  }
  return {tokens, "expected expression"};
}

val_res val_cond_statement(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "cond")
    return val_if_statement(tokens);
  return val_dict_literal(tokens.remove_prefix(1)); // convenient cheat
}

// }}}
// for_loop {{{

val_res val_for_loop(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "for")
    return {};
  if (auto it_res = val_name(tokens.remove_prefix(1))) {
    tokens = *it_res;
    if (!tokens.size() || tokens.front() != "in")
      return {};
    if (auto range_res = val_expression(tokens.remove_prefix(1))) {
      tokens = *it_res;
      if (!tokens.size() || tokens.front() != ":")
        return {};
      return val_expression(tokens.remove_prefix(1));
    }
  }
  return {};
}

// }}}
// while_loop {{{

val_res val_while_loop(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "while")
    return {};
  if (auto test_res = val_expression(tokens.remove_prefix(1))) {
    tokens = *test_res;
    if (!tokens.size() || tokens.front() != ":")
      return {tokens, "expected ':'"};
    auto value = val_expression(tokens.remove_prefix(1));
    if (value)
      return value;
  }
  return {tokens, "expected expression"};
}

// }}}
// function_call {{{

val_res val_function_call(vector_ref<std::string> tokens)
{
  return val_bracketed_subexpr(tokens, val_expr_list, "(", ")");
}

// }}}
// monop_call {{{

val_res val_monop(vector_ref<std::string> tokens)
{
  if (tokens.size() && ( tokens.front() == "!"
                      || tokens.front() == "-"
                      || tokens.front() == "~"
                      || tokens.front() == "#"))
    return tokens.remove_prefix(1);
  return {};
}


val_res val_monop_call(vector_ref<std::string> tokens)
{
  if (auto op_res = val_monop(tokens)) {
    auto expr = val_expression(*op_res);
    if (expr)
      return expr;
    return {*op_res, "expected argument"};
  }
  return {};
}

// }}}
// binop_call {{{

val_res val_binop(vector_ref<std::string> tokens)
{
  if (tokens.size() && ( tokens.front() == "+"
                      || tokens.front() == "-"
                      || tokens.front() == "*"
                      || tokens.front() == "/"
                      || tokens.front() == "%"
                      || tokens.front() == "&"
                      || tokens.front() == "|"
                      || tokens.front() == "^"
                      || tokens.front() == "=="
                      || tokens.front() == "!="
                      || tokens.front() == "<"
                      || tokens.front() == ">"
                      || tokens.front() == "<="
                      || tokens.front() == ">="
                      || tokens.front() == "&&"
                      || tokens.front() == "||"))
    return tokens.remove_prefix(1);
  return {};
}

val_res val_binop_call(vector_ref<std::string> tokens)
{
  if (auto op_res = val_binop(tokens)) {
    auto right_arg = val_expression(*op_res);
    if (right_arg)
      return right_arg;
    return {*op_res, "expected right-hand argument"};
  }
  return {};
}

// }}}
// member {{{

val_res val_member(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != ".")
    return {};
  tokens = tokens.remove_prefix(1);
  if (tokens.size() && tokens.front() == "=")
    return val_expression(tokens.remove_prefix(1));
  return tokens;
}

// }}}
// except {{{

val_res val_except(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "except")
    return {};
  if (auto expr = val_expression(tokens.remove_prefix(1)))
    return expr;
  return {tokens, "expected expression"};
}

// }}}
// expr_list {{{

val_res val_expr_list(vector_ref<std::string> tokens)
{
  return val_comma_separated_list(tokens, val_expression);
}

// }}}
// function_definition {{{

val_res val_parameter_list(vector_ref<std::string> tokens)
{
  return val_comma_separated_list(tokens, val_name);
}

val_res val_function_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "fn")
    return {};
  tokens = tokens.remove_prefix(1);

  if (tokens.size() && val_name(tokens))
    tokens = tokens.remove_prefix(1);

  if (auto param_res = val_bracketed_subexpr(tokens, val_parameter_list,
                                             "(", ")")) {
    tokens = *param_res;
    if (!tokens.size() || tokens.front() != ":")
      return {tokens, "expected ':'"};
    auto expr = val_expression(tokens.remove_prefix(1));
    if (expr || expr.invalid())
      return expr;
    return {tokens, "expected expression"};
  }
  return {tokens, "expected parameter list"};
}

// }}}
// literal {{{

val_res val_number(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  const auto& num = tokens.front();
  auto last = find_if_not(begin(num), end(num), isdigit);
  if (last == begin(num))
    return {};
  if (last != end(num)) {
    if (*last != '.' || find_if_not(last, end(num), isdigit) != end(num))
      return {};
  }
  return tokens.remove_prefix(1);
}

val_res val_string(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  const auto& str = tokens.front();
  if (str.front() != '"' || str.back() != '"')
    return {};
  return tokens.remove_prefix(1);
}

val_res val_bool(vector_ref<std::string> tokens)
{
  if (tokens.size() && (tokens.front() == "true" || tokens.front() == "false"))
    return tokens.remove_prefix(1);
  return {};
}

val_res val_nil(vector_ref<std::string> tokens)
{
  if (tokens.size() && tokens.front() == "nil")
    return tokens.remove_prefix(1);
  return {};
}

val_res val_symbol(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "'")
    return {};
  return val_name(tokens.remove_prefix(1));
}

val_res val_literal(vector_ref<std::string> tokens)
{
  val_res res;
 if ((res = val_number(tokens))
  || (res = val_string(tokens))
  || (res = val_bool(tokens))
  || (res = val_nil(tokens))
  || (res = val_symbol(tokens)))
   ;
 return res;
}

// }}}
// array_literal {{{

val_res val_array_literal(vector_ref<std::string> tokens)
{
  auto arr = val_bracketed_subexpr(tokens, val_expr_list, "[", "]");
  if (arr || !tokens.size() || tokens.front() != "[")
    return arr;
  return {tokens.remove_prefix(1), "expected array literal"};
}

// }}}
// dict_literal {{{

val_res val_dict_pair(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  if (auto test_res = val_expression(tokens)) {
    tokens = *test_res;
    if (tokens.size() && tokens.front() == ":")
      return val_expression(tokens.remove_prefix(1));
  }
  return {};
}

val_res val_dict_internals(vector_ref<std::string> tokens)
{
  return val_comma_separated_list(tokens, val_dict_pair);
}

val_res val_dict_literal(vector_ref<std::string> tokens)
{
  return val_bracketed_subexpr(tokens, val_dict_internals, "{", "}");
}

// }}}
// try_catch {{{

val_res val_try_catch(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "try")
    return {};
  if (tokens.size() < 2 || tokens[1] != ":")
    return { tokens, "expected ':' after 'try'" };

  if (auto body_res = val_expression(tokens.remove_prefix(2))) {
    tokens = *body_res;
    tokens = ltrim(tokens, {"\n"});
    if (!tokens.size() || tokens.front() != "catch")
      return { tokens, "expected 'catch'" };
    if (auto name_res = val_name(tokens.remove_prefix(1))) {
      tokens = *name_res;
      if (!tokens.size() || tokens.front() != ":")
        return { tokens, "expected ':' after 'catch'" };
      return val_expression(tokens.remove_prefix(1)); // ':'
    }
  }
  return {};
}

// }}}
// type_definition {{{

// Extremely similar, but not quite identical, to function validation---
// function names are optional in functions (i.e. lambdas) but not in methods,
// so they have to be evaluated separately. That could just be a bool parameter
// to one validator function, but having them separate future-proofs it and
// makes error messages marginally friendlier
val_res val_method_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "fn")
    return {};
  tokens = tokens.remove_prefix(1);

  if (!tokens.size() || !val_name(tokens))
    return {tokens, "expected method name"};
  tokens = tokens.remove_prefix(1);

  if (auto param_res = val_bracketed_subexpr(tokens, val_parameter_list,
                                             "(", ")")) {
    tokens = *param_res;
    if (!tokens.size() || tokens.front() != ":")
      return {tokens, "expected ':'"};

    auto expr = val_expression(tokens.remove_prefix(1));
    if (expr || expr.invalid())
      return expr;
    return {tokens, "expected expression"};
  }
  return {tokens, "expected parameter list"};
}

val_res val_type_definition(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "class")
    return {};
  if (auto name_res = val_name(tokens.remove_prefix(1))) {

    auto body = val_bracketed_subexpr(*name_res,
         [](auto t)
           { return val_comma_separated_list(t, val_method_definition); },
          "{", "}");
    if (body || body.invalid())
      return body;
    return {*name_res, "expected method list"};
  }
  return {tokens, "expected type name"};
}

// }}}
// variable_declaration {{{

val_res val_variable_declaration(vector_ref<std::string> tokens)
{
  if (!tokens.size() || tokens.front() != "let")
    return {};
  if (auto name_res = val_name(tokens.remove_prefix(1))) {
    tokens = *name_res;
    if (!tokens.size() || tokens.front() != "=")
      return {tokens, "expected '='"};
    auto expr = val_expression(tokens.remove_prefix(1));
    if (expr)
      return expr;
    return {tokens, "expected expression"};
  }
  return {tokens, "expected variable name"};
}

// }}}
// name {{{

val_res val_name(vector_ref<std::string> tokens)
{
  if (!tokens.size())
    return {};
  const auto& potential_name = tokens.front();
  if (isdigit(potential_name.front()) || !all_of(begin(potential_name),
                                                 end(potential_name),
                                                 isnamechar))
    return {};
  return tokens.remove_prefix(1);
}

// }}}

// }}}

}

val_res parser::is_valid(parser::token_string tokens)
{
  auto first_nonline = std::find_if(begin(tokens), end(tokens),
                                    [](const auto& s) { return s != "\n"; });
  tokens = tokens.remove_prefix(first_nonline - begin(tokens));

  while (tokens.size()) {
    auto res = val_expression(tokens);
    if (!res)
      return res;
    tokens = *res;
    auto first_nonline = std::find_if(begin(tokens), end(tokens),
                                      [](const auto& s) { return s != "\n"; });
    tokens = tokens.remove_prefix(first_nonline - begin(tokens));
  }
  return tokens;
}
