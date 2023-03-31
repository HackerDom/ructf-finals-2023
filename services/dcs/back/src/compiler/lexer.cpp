#include <cctype>
#include <sstream>

#include "lexer.h"
#include "utils/utils.h"

static const char* TokenTypeNames[] = {
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

static const char *GetTokenTypeName(Token::Type t) {
    return TokenTypeNames[static_cast<int>(t)];
}

std::string Token::String() const {
    std::stringstream ss;

    ss << "Token(Type::" << GetTokenTypeName(type) << ", \"" << value << "\")";

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

    if (s.size() > 20) {
        return {Token(Token::Type::Eof, ""), Format("name '%s' too long", std::string(s).c_str())};
    }

    if (s == "fun") {
        return {Token(Token::Type::Fun, s), ""};
    } else if (s == "return") {
        return {Token(Token::Type::Return, s), ""};
    } else if (s == "if") {
        return {Token(Token::Type::If, s), ""};
    } else if (s == "else") {
        return {Token(Token::Type::Else, s), ""};
    }

    return {Token(Token::Type::Name, s), ""};
}

static TokenReadResult readNumber(std::string_view input, int &i) {
    const auto l = static_cast<int>(input.size());
    const auto start = i;

    auto dotFound = false;
    auto eFound = false;

    for (; i < l; ++i) {
        char c = input[i];

        if (eFound) {
            if (!std::isdigit(c)) {
                break;
            }
        } else {
            if (std::isdigit(c)) {
                continue;
            }
            else if (c == '.') {
                if (dotFound) {
                    break;
                }
                dotFound = true;
                continue;
            } else if (c == 'e') {
                eFound = true;
                ++i;
                if (i >= l) {
                    return {Token(Token::Type::Number, ""), "unexpected end while reading number"};
                }
                if (input[i] != '-' && input[i] != '+' && !std::isdigit(input[i])) {
                    return {Token(Token::Type::Number, ""), "unexpected symbol while reading number"};
                }
            } else {
                break;
            }
        }
    }

    auto s = input.substr(start, i - start);

    --i; // for reading cycle

    return {Token(Token::Type::Number, s), ""};
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
            tokens.emplace_back(Token::Type::LeftParen, input.substr(i, 1));
        } else if (c == ')') {
            tokens.emplace_back(Token::Type::RightParen, input.substr(i, 1));
        } else if (c == '{') {
            tokens.emplace_back(Token::Type::LeftBrace, input.substr(i, 1));
        } else if (c == '}') {
            tokens.emplace_back(Token::Type::RightBrace, input.substr(i, 1));
        } else if (c == '=') {
            if (i != len - 1 && input[i + 1] == '=') {
                tokens.emplace_back(Token::Type::Eq, input.substr(i, 2));
                ++i;
            } else {
                tokens.emplace_back(Token::Type::Assign, input.substr(i, 1));
            }
        } else if (c == '<') {
            if (i != len - 1 && input[i + 1] == '=') {
                tokens.emplace_back(Token::Type::Le, input.substr(i, 2));
                ++i;
            } else {
                tokens.emplace_back(Token::Type::Less, input.substr(i, 1));
            }
        } else if (c == '>') {
            if (i != len - 1 && input[i + 1] == '=') {
                tokens.emplace_back(Token::Type::Ge, input.substr(i, 2));
                ++i;
            } else {
                tokens.emplace_back(Token::Type::Great, input.substr(i, 1));
            }
        } else if (c == '!') {
            if (i != len - 1 && input[i + 1] == '=') {
                tokens.emplace_back(Token::Type::Neq, input.substr(i, 2));
                ++i;
            } else {
                return {false, "expected '=' after '!'", std::move(tokens)};
            }
        } else if (c == '+') {
            tokens.emplace_back(Token::Type::Plus, input.substr(i, 1));
        } else if (c == '-') {
            tokens.emplace_back(Token::Type::Minus, input.substr(i, 1));
        } else if (c == '*') {
            tokens.emplace_back(Token::Type::Mul, input.substr(i, 1));
        } else if (c == '/') {
            tokens.emplace_back(Token::Type::Div, input.substr(i, 1));
        } else if (c == ';') {
            tokens.emplace_back(Token::Type::Semicolon, input.substr(i, 1));
        } else if (c == ',') {
            tokens.emplace_back(Token::Type::Comma, input.substr(i, 1));
        } else {
            using namespace std::literals;
            return {false, "unexpected symbol "s + c + " at "s + std::to_string(i)};
        }
    }

    tokens.emplace_back(Token::Type::Eof, "");

    return {true, "", std::move(tokens)};
}
