#pragma once

#include <string>
#include <optional>
#include <ostream>

namespace aqua::lexing {

enum class TokenType {
    // Palavras-chave
    FUNC,
    LET,
    IMPORT,
    SPAWN,
    MATCH,
    CASE,
    LOOP,
    BREAK,
    CONTINUE,
    IF,
    ELSE,
    RETURN,
    MAKE_CHANNEL,
    SLEEP,
    TRUE,
    FALSE,
    NONE,

    // Tipos
    INT,
    FLOAT,
    STRING_TYPE,
    BOOL,

    // Identificadores e literais
    IDENTIFIER,
    NUMBER,
    STRING,

    // Operadores
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    EQ,
    EQEQ,
    BANGEQ,
    GT,
    LT,
    GTE,
    LTE,
    COLON_EQ, // :=
    AND,
    OR,
    NOT,

    // Delimitadores e pontuação
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    LBRACE,
    RBRACE,
    COMMA,
    DOT,
    COLON,
    ARROW,     // =>
    RARROW,    // ->

    // Controle de layout
    NEWLINE,
    INDENT,
    DEDENT,

    // Fim de arquivo
    END_OF_FILE
};

struct Token {
    TokenType type{};
    std::string value{};
    int line{1};
    int column{1};
};

const char* toString(TokenType type) noexcept;
std::ostream& operator<<(std::ostream& os, const Token& tok);

} // namespace aqua::lexing
