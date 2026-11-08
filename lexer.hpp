#pragma once

#include <cstddef>
#include <iostream>
#include <stack>
#include <string>
#include <string_view>
#include <vector>

enum class TokenType {
  Number,
  String,
  Boolean,

  Identifier,

  KwPrint,
  KwLocal,

  Plus,
  Minus,
  Star,
  Slash,

  Equals,

  LeftParent,
  RightParent,

  End,

  Invalid,
};

struct Token {
  TokenType type;
  std::string_view text;
  size_t line;
  size_t column;
};

bool isWhitespace(char c);
bool isDigit(char c);
bool isAlphaNumeric(char c);

TokenType getKeywordType(std::string_view text);

std::string_view tokenTypeToString(TokenType type);

class Lexer {
  std::string_view source;
  size_t position{0};
  size_t line{1};
  size_t column{1};

  bool atEnd() const { return position >= source.size(); }
  void skipWhitepace() {
    while (!atEnd() && isWhitespace(peek())) {
      advance();
    }
  }

  bool isComment() const { return !atEnd() && peek() == '-' && peek(1) == '-'; }

  void skipComment() {
    advance();
    advance();

    while (!atEnd() && peek() != '\n') {
      advance();
    }

    if (!atEnd()) {
      advance();
    }
  }

  char peek(size_t offset = 0) const {
    if (position + offset >= source.size()) {
      return '\0';
    }

    return source[offset + position];
  }

  char advance() {
    if (atEnd())
      return '\0';

    char c = source[position++];
    if (c == '\n') {
      line++;
      column = 1;
    } else {
      column++;
    }

    return c;
  }

public:
  Lexer(std::string_view sv) : source(sv) {
    std::cout << "Lexer Initialized!\n";
  }

  Token nextToken() {
    while (!atEnd()) {
      if (isWhitespace(peek())) {
        skipWhitepace();
      } else if (isComment()) {
        skipComment();
      } else {
        break;
      }
    }

    size_t start = position;
    size_t startLine = line;
    size_t startColumn = column;

    if (atEnd())
      return Token{TokenType::End, "", line, column};

    char c = source.at(position);

    if (isDigit(c)) {
      while (!atEnd() && isDigit(peek())) {
        advance();
      }

      return Token{
          TokenType::Number,
          source.substr(start, position - start),
          startLine,
          startColumn,
      };
    } else if (isAlphaNumeric(c)) {

      while (!atEnd() && isAlphaNumeric(peek())) {
        advance();
      }

      std::string_view text = source.substr(start, position - start);

      return Token{
          getKeywordType(text),
          text,
          startLine,
          startColumn,
      };
    }

    switch (peek()) {
    case '+':
      advance();

      return Token{
          TokenType::Plus,
          source.substr(start, 1),
          startLine,
          startColumn,
      };
    case '-':
      advance();

      return Token{
          TokenType::Minus,
          source.substr(start, 1),
          startLine,
          startColumn,
      };
    case '*':
      advance();

      return Token{
          TokenType::Star,
          source.substr(start, 1),
          startLine,
          startColumn,
      };
    case '/':
      advance();

      return Token{
          TokenType::Slash,
          source.substr(start, 1),
          startLine,
          startColumn,
      };

    case '=':
      advance();

      return Token{
          TokenType::Equals,
          source.substr(start, 1),
          startLine,
          startColumn,
      };
    case '(':
      advance();

      return Token{
          TokenType::LeftParent,
          source.substr(start, 1),
          startLine,
          startColumn,
      };

    case ')':
      advance();

      return Token{
          TokenType::RightParent,
          source.substr(start, 1),
          startLine,
          startColumn,
      };
    case '"':
      advance();

      while (!atEnd() && peek() != '"') {
        if (peek() == '\\')
          advance();

        advance();
      }

      if (atEnd() || peek() != '"')
        return Token{
            TokenType::Invalid,
            "",
            startLine,
            startColumn,
        };

      advance();

      return Token{
          TokenType::String,
          source.substr(start + 1, position - 2 - start),
          startLine,
          startColumn,
      };

    default:
      advance();

      return Token{
          TokenType::Invalid,
          source.substr(start, 1),
          startLine,
          startColumn,
      };
    }

    return Token{TokenType::End, "", line, column};
  }

  std::vector<Token> tokenize() {
    std::vector<Token> tokens;

    Token token;

    do {
      token = nextToken();
      tokens.push_back(token);
    } while (token.type != TokenType::End);

    return tokens;
  }
};

inline bool isDigit(char c) { return c >= '0' && c <= '9'; }
inline bool isWhitespace(char c) {
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

inline bool isAlphaNumeric(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' ||
         isDigit(c);
}

inline TokenType getKeywordType(std::string_view text) {
  if (text == "local")
    return TokenType::KwLocal;
  if (text == "print")
    return TokenType::KwPrint;
  if (text == "true" || text == "false")
    return TokenType::Boolean;

  return TokenType::Identifier;
}

inline std::string_view tokenTypeToString(TokenType type) {
  switch (type) {
  case TokenType::Number:
    return "Number";
  case TokenType::String:
    return "String";
  case TokenType::Boolean:
    return "Boolean";
  case TokenType::Identifier:
    return "Identifier";
  case TokenType::KwLocal:
    return "KwLocal";
  case TokenType::KwPrint:
    return "KwPrint";
  case TokenType::Plus:
    return "Plus";
  case TokenType::Minus:
    return "Minus";
  case TokenType::Star:
    return "Star";
  case TokenType::Slash:
    return "Slash";
  case TokenType::Equals:
    return "Equals";
  case TokenType::LeftParent:
    return "LeftParent";
  case TokenType::RightParent:
    return "RightParent";
  case TokenType::End:
    return "End";
  case TokenType::Invalid:
    return "Invalid";
  }

  return "Unknown";
}
