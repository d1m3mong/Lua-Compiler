#pragma once

#include <iostream>
#include <memory>
#include <string_view>
#include <variant>

#include "lexer.hpp"

struct BinaryExprNode;
struct UnaryExprNode;
struct LocalDeclarationNode;
struct AssignmentNode;
struct PrintNode;

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
                             IdentifierNode, UnaryExprNode,
                             LocalDeclarationNode, AssignmentNode, PrintNode>;
struct UnaryExprNode {
  TokenType op;
  std::unique_ptr<ASTNode> operand;

  UnaryExprNode(TokenType op, std::unique_ptr<ASTNode> operand)
      : op(op), operand(std::move(operand)) {}
};

struct PrintNode {
  std::unique_ptr<ASTNode> content;
};

struct LocalDeclarationNode {
  std::string name;
  std::unique_ptr<ASTNode> initializer;
};

struct AssignmentNode {
  std::string name;
  std::unique_ptr<ASTNode> assignment;
};

struct BinaryExprNode {
  std::unique_ptr<ASTNode> left;
  std::unique_ptr<ASTNode> right;
  TokenType op;

  BinaryExprNode(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right,
                 TokenType op)
      : left(std::move(left)), right(std::move(right)), op(op) {}
};
