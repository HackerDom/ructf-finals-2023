#pragma once

#include <vector>
#include <string>
#include <string_view>

struct Token {
    enum Type {
        Assign      = 0,
        Semicolon   = 1,
        LeftParen   = 2,
        RightParen  = 3,
        LeftBrace   = 4,
        RightBrace  = 5,
        Fun         = 6,
        Return      = 7,
        If          = 8,
        Else        = 9,
        Plus        = 10,
        Minus       = 11,
        Mul         = 12,
        Div         = 13,
        Comma       = 14,
        Less        = 15,
        Great       = 16,
        Eq          = 17,
        Neq         = 18,
        Le          = 19,
        Ge          = 20,
        Name        = 21,
        Number      = 22,
        Eof         = 23
    };

    Type type;
    std::string_view value;

    Token(Type type, std::string_view value): type(type), value(value) {
    }

    bool operator==(const Token &other) const {
        return type == other.type && value == other.value;
    }

    [[nodiscard]] std::string String() const;
};


struct TokenizeResult {
    bool Success;
    std::string ErrorMessage;
    std::vector<Token> Tokens;
};

TokenizeResult TokenizeString(std::string_view str);
