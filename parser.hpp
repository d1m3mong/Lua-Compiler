#pragma once

#include <algorithm>
#include <charconv>
#include <exception>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <variant>
#include <vector>

#include "ast.hpp"
#include "lexer.hpp"

class Parser {
  std::vector<Token> tokens;
  size_t position{0};
  std::vector<ASTNode> ast;

  int getBindingPower(TokenType type) const {
    switch (type) {
    case TokenType::Plus:
    case TokenType::Minus:
      return 10;
    case TokenType::Star:
    case TokenType::Slash:
      return 20;
    case TokenType::KwAnd:
      return 5;
    case TokenType::KwOr:
      return 3;
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
    case TokenType::KwAnd:
    case TokenType::KwOr:
    case TokenType::EqualEqual:
    case TokenType::LessEqual:
    case TokenType::LessThan:
    case TokenType::GreaterEqual:
    case TokenType::GreaterThan:
    case TokenType::NotEqual:
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

  std::optional<ASTNode> parseExpression(int minBindingPower = 0) {
    if (!peek().has_value())
      return std::nullopt;

    auto primaryResult = parsePrimary();
    if (!primaryResult)
      return std::nullopt;

    std::unique_ptr<ASTNode> left =
        std::make_unique<ASTNode>(std::move(primaryResult.value()));
    TokenType op;

    while (true) {
      if (!peek().has_value())
        break;

      if (isOperator(peek()->type)) {
        if (getBindingPower(peek()->type) < minBindingPower)
          break;
      } else {
        break;
      }

      int bindingPower = getBindingPower(peek()->type);

      op = advance().value().type;

      auto result = parseExpression(bindingPower + 1);
      if (!result)
        return std::nullopt;

      auto right = std::make_unique<ASTNode>(std::move(result.value()));

      left = std::make_unique<ASTNode>(std::make_unique<BinaryExprNode>(
          std::move(left), std::move(right), op));
    }

    return std::move(*left);
  }

  bool atEnd() const {
    auto res = peek();
    return !res.has_value() || res->type == TokenType::End;
  }

  std::optional<ASTNode> parsePrimary() {
    if (!peek().has_value())
      return std::nullopt;

    switch (peek().value().type) {
    case TokenType::Number: {
      double value{};
      try {
        value = transformStringToDigit(peek().value());
      } catch (const std::exception &ec) {
        std::cerr << ec.what() << "\n";
        return std::nullopt;
      }

      advance();
      return NumberNode{value};
    }

    case TokenType::Identifier: {
      std::string_view text = peek().value().text;

      advance();

      return IdentifierNode{text};
    }
    case TokenType::Boolean: {
      bool result = transformStringToBool(peek().value());

      advance();

      return BooleanNode{result};
    }
    case TokenType::String: {
      std::string_view text = peek().value().text;

      advance();

      return StringNode{text};
    }
    case TokenType::LeftParent: {
      advance();

      auto innerAST = parseExpression();
      if (!innerAST.has_value())
        return std::nullopt;

      if (!match(TokenType::RightParent))
        return std::nullopt;

      return innerAST;
    }
    case TokenType::Minus: {
      TokenType c = peek().value().type;

      advance();

      auto innerAST = parseExpression(30);
      if (!innerAST.has_value())
        return std::nullopt;

      std::unique_ptr<ASTNode> operand =
          std::make_unique<ASTNode>(std::move(innerAST.value()));

      return UnaryExprNode{c, std::move(operand)};
    }
    default:
      return std::nullopt;
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

    auto result = parseExpression();
    if (!result) //|| !atEnd())
      return false;

    ast.push_back(std::move(result.value()));

    return true;
  }

  double transformStringToDigit(const Token &t) {
    double value{};

    auto [ptr, ec] =
        std::from_chars(t.text.data(), t.text.data() + t.text.size(), value);
    if (ec != std::errc{} || ptr != t.text.data() + t.text.size()) {
      throw std::invalid_argument("No valid double!");
    }

    return value;
  }

  bool transformStringToBool(const Token &t) {
    if (t.text == "true")
      return true;
    return false;
  }

public:
  Parser(std::vector<Token> &tokens) : tokens(tokens) {
    std::cout << "Initialized Parser!\n";
  }

  const std::vector<ASTNode> &getAST() const { return ast; }

  bool parse() {
    while (!atEnd()) {
      if (!parseDeclaration()) {
        std::cerr << "Parse Error at token: " << position << '\n';
        return false;
      }
    }

    std::cout << "Parse Successfully!\n";
    return true;
  }
};

inline void showAST(const ASTNode *node) {
  std::visit(
      [](const auto &value) {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, NumberNode>) {
          std::cout << value.value;
        } else if constexpr (std::is_same_v<T, StringNode>) {
          std::cout << value.text;
        } else if constexpr (std::is_same_v<T, IdentifierNode>) {
          std::cout << value.text;
        } else if constexpr (std::is_same_v<T, BooleanNode>) {
          std::cout << value.value;
        } else if constexpr (std::is_same_v<T, UnaryExprNode>) {
          std::cout << tokenTypeToString(value.op);
          showAST(value.operand.get());
        } else if constexpr (std::is_same_v<T, BinaryExprPtr>) {
          std::cout << "(";
          showAST(value->left.get());
          std::cout << " " << tokenTypeToString(value->op) << " ";
          showAST(value->right.get());
          std::cout << ")";
        }
      },
      *node);
}
