#pragma once

#include <iostream>
#include <memory>
#include <string_view>
#include <variant>

#include "lexer.hpp"

struct BinaryExprNode;
struct UnaryExprNode;

struct NumberNode {
  double value;
};

struct BooleanNode {
  bool value;
};

struct StringNode {
  std::string_view text;
};

struct IdentifierNode {
  std::string_view text;
};

using BinaryExprPtr = std::unique_ptr<BinaryExprNode>;
using ASTNode = std::variant<NumberNode, BinaryExprPtr, BooleanNode, StringNode,
                             IdentifierNode, UnaryExprNode>;
struct UnaryExprNode {
  TokenType op;
  std::unique_ptr<ASTNode> operand;

  UnaryExprNode(TokenType op, std::unique_ptr<ASTNode> operand)
      : op(op), operand(std::move(operand)) {}
};

struct BinaryExprNode {
  std::unique_ptr<ASTNode> left;
  std::unique_ptr<ASTNode> right;
  TokenType op;

  BinaryExprNode(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right,
                 TokenType op)
      : left(std::move(left)), right(std::move(right)), op(op) {}
};
