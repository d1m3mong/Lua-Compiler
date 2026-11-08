#pragma once

#include <iostream>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

#include "lexer.hpp"

class Parser {
  std::vector<Token> tokens;
  size_t position{0};

  int getBindingPower(TokenType type) const {
    switch (type) {
    case TokenType::Plus:
    case TokenType::Minus:
      return 10;
    case TokenType::Star:
    case TokenType::Slash:
      return 20;
    default:
      return 0;
    }
  }

  bool isOperator(TokenType type) const {
    switch (type) {
    case TokenType::Plus:
    case TokenType::Minus:
    case TokenType::Slash:
    case TokenType::Star:
      return true;

    default:
      return false;
    }
  }

  std::optional<Token> peek(size_t offset = 0) const {
    if (position + offset >= tokens.size())
      return std::nullopt;

    return tokens[position + offset];
  }

  std::optional<Token> advance() {
    if (position >= tokens.size())
      return std::nullopt;

    Token t = tokens[position++];

    return t;
  }

  bool match(TokenType type) {
    if (!peek().has_value())
      return false;

    if (peek().value().type == type) {
      advance();
      return true;
    }

    return false;
  }

  bool parseExpression(int minBindingPower = 0) {
    if (!peek().has_value())
      return false;

    if (!parsePrimary())
      return false;

    while (true) {
      if (isOperator(peek()->type)) {
        if (getBindingPower(peek()->type) < minBindingPower)
          break;
      } else {
        break;
      }

      int bindingPower = getBindingPower(peek()->type);

      advance();
      bool result = parseExpression(bindingPower + 1);
      if (!result)
        return false;
    }

    return true;
  }

  bool atEnd() const {
    auto res = peek();
    return !res.has_value() || res->type == TokenType::End;
  }

  bool parsePrimary() {
    if (!peek().has_value())
      return false;

    switch (peek().value().type) {
    case TokenType::Number:
    case TokenType::Identifier:
    case TokenType::Boolean:
    case TokenType::String:
      advance();
      return true;
    default:
      return false;
    }
  }

  bool parseDeclaration() {
    if (!match(TokenType::KwLocal))
      return false;
    if (!match(TokenType::Identifier))
      return false;
    if (!match(TokenType::Equals))
      return false;
    // if(!match(TokenType::Boolean) || !match(TokenType::String) ||
    // !match(TokenType::Number)) return false;

    bool success = parseExpression();
    if (!success || !atEnd())
      return false;

    std::cout << "Parse Successfully!\n";
    return true;
  }

public:
  Parser(std::vector<Token> &tokens) : tokens(tokens) {
    std::cout << "Initialized Parser!\n";
  }

  void parse() { parseDeclaration(); }
};
