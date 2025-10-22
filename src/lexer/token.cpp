#include "token.hpp"

#include <iostream>

namespace aqua::lexing {

const char* toString(TokenType type) noexcept {
    switch (type) {
        case TokenType::FUNC: return "FUNC";
        case TokenType::LET: return "LET";
        case TokenType::IMPORT: return "IMPORT";
        case TokenType::SPAWN: return "SPAWN";
        case TokenType::MATCH: return "MATCH";
        case TokenType::CASE: return "CASE";
        case TokenType::LOOP: return "LOOP";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::MAKE_CHANNEL: return "MAKE_CHANNEL";
        case TokenType::SLEEP: return "SLEEP";
        case TokenType::TRUE: return "TRUE";
        case TokenType::FALSE: return "FALSE";
        case TokenType::NONE: return "NONE";
        case TokenType::INT: return "INT";
        case TokenType::FLOAT: return "FLOAT";
        case TokenType::STRING_TYPE: return "STRING_TYPE";
        case TokenType::BOOL: return "BOOL";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::EQ: return "EQ";
        case TokenType::EQEQ: return "EQEQ";
        case TokenType::BANGEQ: return "BANGEQ";
        case TokenType::GT: return "GT";
        case TokenType::LT: return "LT";
        case TokenType::GTE: return "GTE";
        case TokenType::LTE: return "LTE";
        case TokenType::COLON_EQ: return "COLON_EQ";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::COMMA: return "COMMA";
        case TokenType::DOT: return "DOT";
        case TokenType::COLON: return "COLON";
        case TokenType::ARROW: return "ARROW";
        case TokenType::RARROW: return "RARROW";
        case TokenType::NEWLINE: return "NEWLINE";
        case TokenType::INDENT: return "INDENT";
        case TokenType::DEDENT: return "DEDENT";
        case TokenType::END_OF_FILE: return "EOF";
        default: return "UNKNOWN";
    }
}

std::ostream& operator<<(std::ostream& os, const Token& tok) {
    os << toString(tok.type);
    if (!tok.value.empty()) {
        os << "(" << tok.value << ")";
    }
    os << "@" << tok.line << ":" << tok.column;
    return os;
}

} // namespace aqua::lexing
