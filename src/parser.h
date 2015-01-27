#ifndef IL_PARSER_H
#define IL_PARSER_H

#include "expression.h"
#include "utils.h"

#include <istream>
#include <string>
#include <vector>

namespace il {

namespace parser {

using token_string = vector_ref<std::string>;

std::vector<std::string> tokenize(std::istream& input);

bool is_valid(token_string tokens);

std::vector<std::unique_ptr<ast::expression>> parse(token_string tokens);

}

}

#endif
