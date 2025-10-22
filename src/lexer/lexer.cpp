#include "lexer.hpp"
#include "utils/error.hpp"

#include <cctype>
#include <sstream>
#include <unordered_map>

namespace aqua::lexing {

static const std::unordered_set<std::string> kKeywords = {
    "func","let","import","spawn","match","case","loop","break","continue",
    "if","else","return","make_channel","sleep","true","false","None",
    // tipos
    "int","float","string","bool"
};

Lexer::Lexer(std::string source)
    : input(std::move(source)) {}

bool Lexer::isKeyword(std::string_view word) {
    return kKeywords.find(std::string(word)) != kKeywords.end();
}

char Lexer::peek(size_t lookahead) const {
    size_t pos = index + lookahead;
    if (pos >= input.size()) return '\0';
    return input[pos];
}

bool Lexer::eof(size_t lookahead) const {
    return index + lookahead >= input.size();
}

char Lexer::advance() {
    if (eof()) return '\0';
    char c = input[index++];
    if (c == '\n') {
        line++;
        column = 1;
        atLineStart = true;
    } else {
        column++;
    }
    return c;
}

void Lexer::emit(TokenType type, const std::string& value) {
    tokens.push_back(Token{type, value, line, column});
}

void Lexer::skipSpaces() {
    while (!eof() && peek() == ' ' && !atLineStart) {
        advance();
    }
}

bool Lexer::skipComment() {
    if (!eof() && peek() == '#') {
        while (!eof() && advance() != '\n') { /* skip */ }
        return true;
    }
    return false;
}

void Lexer::emitPendingIndents(int indent) {
    int current = indentStack.back();
    if (indent > current) {
        indentStack.push_back(indent);
        tokens.push_back(Token{TokenType::INDENT, "", line, column});
    } else if (indent < current) {
        while (!indentStack.empty() && indentStack.back() > indent) {
            indentStack.pop_back();
            tokens.push_back(Token{TokenType::DEDENT, "", line, column});
        }
        if (indentStack.empty() || indentStack.back() != indent) {
            throw LexerError(formatError("IndentationError", line, column, "Indentação inválida"));
        }
    }
}

void Lexer::handleNewline() {
    // Consumir o \n
    advance();
    tokens.push_back(Token{TokenType::NEWLINE, "\n", line - 1, 1});

    // Calcular indentação da próxima linha: apenas espaços; tabs não suportados
    int indent = 0;
    while (!eof() && peek() == ' ') {
        indent++;
        advance();
    }
    atLineStart = false; // já consumimos espaços do início

    // Se a linha está vazia ou comentário, mantenha estado de início de linha para próxima
    if (peek() == '\n' || peek() == '#') {
        // linhas vazias não alteram indentação
        return;
    }

    emitPendingIndents(indent);
}

static bool isDecimalDigit(char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; }

std::optional<Token> Lexer::readNumber() {
    if (!isDecimalDigit(peek())) return std::nullopt;

    int startLine = line;
    int startColumn = column;

    std::string lexeme;
    bool hasDot = false;

    while (!eof()) {
        char c = peek();
        if (isDecimalDigit(c)) {
            lexeme.push_back(advance());
        } else if (c == '.' && !hasDot && isDecimalDigit(peek(1))) {
            hasDot = true;
            lexeme.push_back(advance());
        } else {
            break;
        }
    }

    return Token{TokenType::NUMBER, lexeme, startLine, startColumn};
}

bool Lexer::isIdentStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool Lexer::isIdentPart(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::optional<Token> Lexer::readIdentifierOrKeyword() {
    if (!isIdentStart(peek())) return std::nullopt;

    int startLine = line;
    int startColumn = column;

    std::string lexeme;
    while (!eof() && isIdentPart(peek())) {
        lexeme.push_back(advance());
    }

    // keywords e booleanos/None
    if (lexeme == "true") return Token{TokenType::TRUE, lexeme, startLine, startColumn};
    if (lexeme == "false") return Token{TokenType::FALSE, lexeme, startLine, startColumn};
    if (lexeme == "None") return Token{TokenType::NONE, lexeme, startLine, startColumn};

    if (lexeme == "func") return Token{TokenType::FUNC, lexeme, startLine, startColumn};
    if (lexeme == "let") return Token{TokenType::LET, lexeme, startLine, startColumn};
    if (lexeme == "import") return Token{TokenType::IMPORT, lexeme, startLine, startColumn};
    if (lexeme == "spawn") return Token{TokenType::SPAWN, lexeme, startLine, startColumn};
    if (lexeme == "match") return Token{TokenType::MATCH, lexeme, startLine, startColumn};
    if (lexeme == "case") return Token{TokenType::CASE, lexeme, startLine, startColumn};
    if (lexeme == "loop") return Token{TokenType::LOOP, lexeme, startLine, startColumn};
    if (lexeme == "break") return Token{TokenType::BREAK, lexeme, startLine, startColumn};
    if (lexeme == "continue") return Token{TokenType::CONTINUE, lexeme, startLine, startColumn};
    if (lexeme == "if") return Token{TokenType::IF, lexeme, startLine, startColumn};
    if (lexeme == "else") return Token{TokenType::ELSE, lexeme, startLine, startColumn};
    if (lexeme == "return") return Token{TokenType::RETURN, lexeme, startLine, startColumn};
    if (lexeme == "make_channel") return Token{TokenType::MAKE_CHANNEL, lexeme, startLine, startColumn};
    if (lexeme == "sleep") return Token{TokenType::SLEEP, lexeme, startLine, startColumn};

    if (lexeme == "int") return Token{TokenType::INT, lexeme, startLine, startColumn};
    if (lexeme == "float") return Token{TokenType::FLOAT, lexeme, startLine, startColumn};
    if (lexeme == "string") return Token{TokenType::STRING_TYPE, lexeme, startLine, startColumn};
    if (lexeme == "bool") return Token{TokenType::BOOL, lexeme, startLine, startColumn};
    if (lexeme == "and") return Token{TokenType::AND, lexeme, startLine, startColumn};
    if (lexeme == "or") return Token{TokenType::OR, lexeme, startLine, startColumn};
    if (lexeme == "not") return Token{TokenType::NOT, lexeme, startLine, startColumn};

    return Token{TokenType::IDENTIFIER, lexeme, startLine, startColumn};
}

std::optional<Token> Lexer::readString() {
    if (peek() != '"') return std::nullopt;

    int startLine = line;
    int startColumn = column;

    advance(); // opening quote
    std::string result;

    while (!eof()) {
        char c = advance();
        if (c == '"') {
            return Token{TokenType::STRING, result, startLine, startColumn};
        }
        if (c == '\\') {
            if (eof()) break;
            char esc = advance();
            switch (esc) {
                case 'n': result.push_back('\n'); break;
                case 't': result.push_back('\t'); break;
                case '"': result.push_back('"'); break;
                case '\\': result.push_back('\\'); break;
                default:
                    // mantém literal desconhecido como é
                    result.push_back('\\');
                    result.push_back(esc);
                    break;
            }
        } else {
            result.push_back(c);
        }
    }

    throw LexerError(formatError("LexError", startLine, startColumn, "String não terminada"));
}

std::optional<Token> Lexer::readOperatorOrPunct() {
    int startLine = line;
    int startColumn = column;

    char c = peek();

    // Operadores compostos
    if (c == ':' && peek(1) == '=') {
        advance(); advance();
        return Token{TokenType::COLON_EQ, ":=", startLine, startColumn};
    }
    if (c == '=' && peek(1) == '=') { advance(); advance(); return Token{TokenType::EQEQ, "==", startLine, startColumn}; }
    if (c == '!' && peek(1) == '=') { advance(); advance(); return Token{TokenType::BANGEQ, "!=", startLine, startColumn}; }
    if (c == '>' && peek(1) == '=') { advance(); advance(); return Token{TokenType::GTE, ">=", startLine, startColumn}; }
    if (c == '<' && peek(1) == '=') { advance(); advance(); return Token{TokenType::LTE, "<=", startLine, startColumn}; }
    if (c == '=' && peek(1) == '>') { advance(); advance(); return Token{TokenType::ARROW, "=>", startLine, startColumn}; }
    if (c == '-' && peek(1) == '>') { advance(); advance(); return Token{TokenType::RARROW, "->", startLine, startColumn}; }

    // Simples
    switch (c) {
        case '+': advance(); return Token{TokenType::PLUS, "+", startLine, startColumn};
        case '-': advance(); return Token{TokenType::MINUS, "-", startLine, startColumn};
        case '*': advance(); return Token{TokenType::STAR, "*", startLine, startColumn};
        case '/': advance(); return Token{TokenType::SLASH, "/", startLine, startColumn};
        case '%': advance(); return Token{TokenType::PERCENT, "%", startLine, startColumn};
        case '=': advance(); return Token{TokenType::EQ, "=", startLine, startColumn};
        case '>': advance(); return Token{TokenType::GT, ">", startLine, startColumn};
        case '<': advance(); return Token{TokenType::LT, "<", startLine, startColumn};
        case '(': advance(); return Token{TokenType::LPAREN, "(", startLine, startColumn};
        case ')': advance(); return Token{TokenType::RPAREN, ")", startLine, startColumn};
        case '[': advance(); return Token{TokenType::LBRACKET, "[", startLine, startColumn};
        case ']': advance(); return Token{TokenType::RBRACKET, "]", startLine, startColumn};
        case '{': advance(); return Token{TokenType::LBRACE, "{", startLine, startColumn};
        case '}': advance(); return Token{TokenType::RBRACE, "}", startLine, startColumn};
        case ',': advance(); return Token{TokenType::COMMA, ",", startLine, startColumn};
        case '.': advance(); return Token{TokenType::DOT, ".", startLine, startColumn};
        case ':': advance(); return Token{TokenType::COLON, ":", startLine, startColumn};
        default:
            return std::nullopt;
    }
}

std::vector<Token> Lexer::tokenize() {
    tokens.clear();

    // Log simples
    // std::cerr << "[Lexer] Iniciando tokenização..." << std::endl;

    while (!eof()) {
        // Início de linha: processar indentação
        if (atLineStart) {
            int indent = 0;
            while (!eof() && peek() == ' ') { indent++; advance(); }
            atLineStart = false;

            // Linha vazia ou comentário: emite NEWLINE e continua
            if (peek() == '\n' || peek() == '#') {
                // não altera indentação em linhas vazias
                // consome comentário se houver
                if (peek() == '#') { while (!eof() && advance() != '\n') {} }
                // consome a quebra (se presente)
                if (peek() == '\n') { advance(); }
                tokens.push_back(Token{TokenType::NEWLINE, "\n", line - 1, 1});
                continue;
            }

            emitPendingIndents(indent);
        }

        // Comentários no meio da linha
        if (skipComment()) {
            // Comentário consome até \n; o avanço já contabilizou NEWLINE
            continue;
        }

        // Quebra de linha explícita
        if (peek() == '\n') {
            handleNewline();
            continue;
        }

        // Espaços intermediários (não no início)
        if (peek() == ' ') {
            skipSpaces();
            continue;
        }

        // Strings
        if (auto tok = readString()) { tokens.push_back(*tok); continue; }
        // Número
        if (auto tok = readNumber()) { tokens.push_back(*tok); continue; }
        // Ident/keyword
        if (auto tok = readIdentifierOrKeyword()) { tokens.push_back(*tok); continue; }
        // Operador/pontuação
        if (auto tok = readOperatorOrPunct()) { tokens.push_back(*tok); continue; }

        // Caractere não reconhecido
        std::string msg = std::string("Caractere não reconhecido '") + peek() + "'";
        throw LexerError(formatError("LexError", line, column, msg));
    }

    // Encerrar com dedents pendentes
    while (indentStack.size() > 1) {
        indentStack.pop_back();
        tokens.push_back(Token{TokenType::DEDENT, "", line, column});
    }

    tokens.push_back(Token{TokenType::END_OF_FILE, "", line, column});
    return tokens;
}

} // namespace aqua::lexing
