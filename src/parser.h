#ifndef IL_PARSER_H
#define IL_PARSER_H

#include "expression.h"

#include <string>
#include <vector>

namespace il {

namespace parser {

using token_string = std::vector<std::string>;

token_string tokenize(std::istream& input);

std::vector<std::unique_ptr<ast::expression>> parse(const token_string& tokens);

}

}

#endif
