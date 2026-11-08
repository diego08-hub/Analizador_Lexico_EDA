#pragma once

#include <string>
#include <vector>
#include "types.h"
#include "parse_table.h"
#include "../src/lexer.h"

namespace parser_ll1 {

class Parser {
public:
    Parser();
    explicit Parser(const ParseTable& table);

    // Analiza una sentencia SQL en texto plano directamente usando el Lexer
    ParseResult parse(const std::string& sqlQuery, bool verbose = false);

    // Analiza una lista de tokens previamente generada
    ParseResult parseTokens(const std::vector<Token>& tokens, bool verbose = false);

    const ParseTable& getTable() const { return table_; }

private:
    ParseTable table_;
};

} // namespace parser_ll1
