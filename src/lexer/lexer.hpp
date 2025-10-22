#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <stack>
#include <unordered_set>
#include <optional>

#include "token.hpp"

namespace aqua::lexing {

class Lexer {
public:
    explicit Lexer(std::string source);

    // Tokeniza toda a entrada e retorna a lista completa de tokens
    std::vector<Token> tokenize();

    // Verifica se uma palavra é palavra-chave
    static bool isKeyword(std::string_view word);

private:
    // Entrada e posição
    std::string input;
    size_t index{0};
    int line{1};
    int column{1};

    // Controle de indentação
    std::vector<int> indentStack{0};
    bool atLineStart{true};

    // Tokens acumulados
    std::vector<Token> tokens;

    // Leitura básica
    char peek(size_t lookahead = 0) const;
    bool eof(size_t lookahead = 0) const;
    char advance();

    // Construção de tokens
    void emit(TokenType type, const std::string& value = "");

    // Consumidores
    void skipSpaces();
    bool skipComment();
    void handleNewline();
    void emitPendingIndents(int indent);

    std::optional<Token> readNumber();
    std::optional<Token> readIdentifierOrKeyword();
    std::optional<Token> readString();
    std::optional<Token> readOperatorOrPunct();

    // Utilitários
    static bool isIdentStart(char c);
    static bool isIdentPart(char c);
};

} // namespace aqua::lexing
