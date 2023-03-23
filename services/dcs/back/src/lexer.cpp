#include "lexer.h"

#include <cctype>
#include <sstream>

static const char* tokenTypeNames[] = {
    "Assign",
    "Semicolon",
    "LeftParen",
    "RightParen",
    "LeftBrace",
    "RightBrace",
    "Fun",
    "Return",
    "If",
    "Else",
    "Plus",
    "Minus",
    "Mul",
    "Div",
    "Comma",
    "Less",
    "Great",
    "Eq",
    "Neq",
    "Le",
    "Ge",
    "Name",
    "Number",
    "Eof"
};

static const char *GetTokenTypeName(TokenType t) {
    return tokenTypeNames[static_cast<int>(t)];
}

std::string Token::String() const {
    std::stringstream ss;

    ss << "Token(TokenType::" << GetTokenTypeName(type) << ", \"" << value << "\")";

    return ss.str();
}

struct TokenReadResult {
    Token token;
    std::string errorMessage;
};

static inline bool isNameOrKeywordSymbol(char c) {
    return c == '_' || std::isalpha(c) || std::isdigit(c);
}

static TokenReadResult readNameOrKeyword(std::string_view input, int &i) {
    const auto l = static_cast<int>(input.size());
    const auto start = i;

    while (i < l && isNameOrKeywordSymbol(input[i])) {
        ++i;
    }

    auto s = input.substr(start, i - start);

    --i; // for reading cycle

    if (s == "fun") {
        return {Token(TokenType::Fun, s), ""};
    } else if (s == "return") {
        return {Token(TokenType::Return, s), ""};
    } else if (s == "if") {
        return {Token(TokenType::If, s), ""};
    } else if (s == "else") {
        return {Token(TokenType::Else, s), ""};
    }

    return {Token(TokenType::Name, s), ""};
}

static inline bool isNumberSymbol(char c) {
    return c == '.' || std::isdigit(c);
}

static TokenReadResult readNumber(std::string_view input, int &i) {
    const auto l = static_cast<int>(input.size());
    const auto start = i;

    if (input[i] == '+' || input[i] == '-') {
        ++i;
    }

    auto dotCounter = 0;

    while (i < l && isNumberSymbol(input[i])) {
        if (input[i] == '.') {
            ++dotCounter;
        }

        ++i;
    }

    if (dotCounter > 1) {
        std::stringstream ss;
        ss << "too many dots for '" << input.substr(start, i - start) << "'";
        return {Token(TokenType::Eof, ""), ss.str()};
    }

    auto s = input.substr(start, i - start);

    --i; // for reading cycle

    return {Token(TokenType::Number, s), ""};
}

TokenizeResult TokenizeString(std::string_view input) {
    auto tokens = std::vector<Token>();

    auto len = static_cast<int>(input.size());

    for (int i = 0; i < len; ++i) {
        const auto c = input[i];

        if (std::isspace(c)) {
            continue;
        }
        else if (std::isalpha(c) || c == '_') {
            auto r = readNameOrKeyword(input, i);

            if (!r.errorMessage.empty()) {
                return {false, r.errorMessage, std::move(tokens)};
            }

            tokens.emplace_back(r.token);
        } else if (std::isdigit(c)) {
            auto r = readNumber(input, i);

            if (!r.errorMessage.empty()) {
                return {false, r.errorMessage, std::move(tokens)};
            }

            tokens.emplace_back(r.token);
        } else if (c == '(') {
            tokens.emplace_back(TokenType::LeftParen, input.substr(i, 1));
        } else if (c == ')') {
            tokens.emplace_back(TokenType::RightParen, input.substr(i, 1));
        } else if (c == '{') {
            tokens.emplace_back(TokenType::LeftBrace, input.substr(i, 1));
        } else if (c == '}') {
            tokens.emplace_back(TokenType::RightBrace, input.substr(i, 1));
        } else if (c == '=') {
            if (i != len - 1 && input[i + 1] == '=') {
                tokens.emplace_back(TokenType::Eq, input.substr(i, 2));
                ++i;
            } else {
                tokens.emplace_back(TokenType::Assign, input.substr(i, 1));
            }
        } else if (c == '<') {
            if (i != len - 1 && input[i + 1] == '=') {
                tokens.emplace_back(TokenType::Le, input.substr(i, 2));
                ++i;
            } else {
                tokens.emplace_back(TokenType::Less, input.substr(i, 1));
            }
        } else if (c == '>') {
            if (i != len - 1 && input[i + 1] == '=') {
                tokens.emplace_back(TokenType::Ge, input.substr(i, 2));
                ++i;
            } else {
                tokens.emplace_back(TokenType::Great, input.substr(i, 1));
            }
        } else if (c == '!') {
            if (i != len - 1 && input[i + 1] == '=') {
                tokens.emplace_back(TokenType::Neq, input.substr(i, 2));
                ++i;
            } else {
                return {false, "expected '=' after '!'", std::move(tokens)};
            }
        } else if (c == '+') {
            if (i != len - 1 && std::isdigit(input[i + 1])) {
                auto r = readNumber(input, i);

                if (!r.errorMessage.empty()) {
                    return {false, r.errorMessage, std::move(tokens)};
                }

                tokens.emplace_back(r.token);
            } else {
                tokens.emplace_back(TokenType::Plus, input.substr(i, 1));
            }
        } else if (c == '-') {
            if (i != len - 1 && std::isdigit(input[i + 1])) {
                auto r = readNumber(input, i);

                if (!r.errorMessage.empty()) {
                    return {false, r.errorMessage, std::move(tokens)};
                }

                tokens.emplace_back(r.token);
            } else {
                tokens.emplace_back(TokenType::Minus, input.substr(i, 1));
            }
        } else if (c == '*') {
            tokens.emplace_back(TokenType::Mul, input.substr(i, 1));
        } else if (c == '/') {
            tokens.emplace_back(TokenType::Div, input.substr(i, 1));
        } else if (c == ';') {
            tokens.emplace_back(TokenType::Semicolon, input.substr(i, 1));
        } else if (c == ',') {
            tokens.emplace_back(TokenType::Comma, input.substr(i, 1));
        } else {
            using namespace std::literals;
            return {false, "unexpected symbol "s + c + " at "s + std::to_string(i)};
        }
    }

    tokens.emplace_back(TokenType::Eof, "");

    return {true, "", std::move(tokens)};
}
