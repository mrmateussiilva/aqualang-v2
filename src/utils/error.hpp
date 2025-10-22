#pragma once

#include <stdexcept>
#include <string>

namespace aqua {

class LexerError : public std::runtime_error {
public:
    explicit LexerError(const std::string& message)
        : std::runtime_error(message) {}
};

inline std::string formatError(const std::string& kind, int line, int column, const std::string& message) {
    return kind + " at line " + std::to_string(line) + ", column " + std::to_string(column) + ": " + message;
}

} // namespace aqua
