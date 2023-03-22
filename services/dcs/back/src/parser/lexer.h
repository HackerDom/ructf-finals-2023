#pragma once

#include <vector>
#include <string>
#include <string_view>

enum TokenType {
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

struct Token {
    TokenType type;
    std::string_view value;

    Token(TokenType type, std::string_view value): type(type), value(std::move(value)) {
    }

    bool operator==(const Token &other) const {
        return type == other.type && value == other.value;
    }

    bool isAssign() const {
        return type == TokenType::Assign;
    }

    bool isSemicolon() const {
        return type == TokenType::Semicolon;
    }

    bool isLeftParen() const {
        return type == TokenType::LeftParen;
    }

    bool isRightParen() const {
        return type == TokenType::RightParen;
    }

    bool isLeftBrace() const {
        return type == TokenType::LeftBrace;
    }

    bool isRightBrace() const {
        return type == TokenType::RightBrace;
    }

    bool isFun() const {
        return type == TokenType::Fun;
    }

    bool isReturn() const {
        return type == TokenType::Return;
    }

    bool isIf() const {
        return type == TokenType::If;
    }

    bool isElse() const {
        return type == TokenType::Else;
    }

    bool isPlus() const {
        return type == TokenType::Plus;
    }

    bool isMinus() const {
        return type == TokenType::Minus;
    }

    bool isMul() const {
        return type == TokenType::Mul;
    }

    bool isDiv() const {
        return type == TokenType::Div;
    }

    bool isComma() const {
        return type == TokenType::Comma;
    }

    bool isLess() const {
        return type == TokenType::Less;
    }

    bool isGreat() const {
        return type == TokenType::Great;
    }

    bool isEq() const {
        return type == TokenType::Eq;
    }

    bool isNeq() const {
        return type == TokenType::Neq;
    }

    bool isLe() const {
        return type == TokenType::Le;
    }

    bool isGe() const {
        return type == TokenType::Ge;
    }

    bool isName() const {
        return type == TokenType::Name;
    }

    bool isNumber() const {
        return type == TokenType::Number;
    }

    bool isEof() const {
        return type == TokenType::Eof;
    }

    std::string String() const;
};


struct TokenizeResult {
    bool success;
    std::string errorMessage;
    std::vector<Token> tokens;
};

TokenizeResult TokenizeString(std::string_view str);
