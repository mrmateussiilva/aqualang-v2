#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "lexer/token.hpp"
#include "lexer/lexer.hpp"

using namespace aqua::lexing;

int main(int argc, char** argv) {
    std::ios::sync_with_stdio(false);

    if (argc < 2) {
        std::cerr << "Uso: aqua-lexer <arquivo.aqua>" << std::endl;
        return 1;
    }

    const std::string filename = argv[1];
    std::ifstream in(filename);
    if (!in) {
        std::cerr << "Erro ao abrir arquivo: " << filename << std::endl;
        return 1;
    }

    std::ostringstream buffer;
    buffer << in.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    for (const auto& t : tokens) {
        std::cout << t << "\n";
    }

    return 0;
}
