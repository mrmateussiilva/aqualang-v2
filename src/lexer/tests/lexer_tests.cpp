// Framework de teste simples (substituto mínimo de Catch2)
#include <cassert>
#include <iostream>

#include "../token.hpp"
#include "../lexer.hpp"

using namespace aqua::lexing;

int main() {
    {
        std::string code = "func main()\n    let x = 10\n";
        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        assert(tokens.size() > 6);
        assert(tokens[0].type == TokenType::FUNC);
        assert(tokens[1].type == TokenType::IDENTIFIER);
        assert(tokens[1].value == "main");
    }

    {
        std::string code = "func main()\n    let x = 10\n    if x > 5\n        print(\"ok\")\n";
        Lexer lexer(code);
        auto tokens = lexer.tokenize();
        bool hasIndent = false;
        bool hasDedent = false;
        for (const auto& t : tokens) {
            if (t.type == TokenType::INDENT) hasIndent = true;
            if (t.type == TokenType::DEDENT) hasDedent = true;
        }
        assert(hasIndent);
        assert(hasDedent);
    }

    std::cout << "✅ Testes do lexer passaram" << std::endl;
    return 0;
}
