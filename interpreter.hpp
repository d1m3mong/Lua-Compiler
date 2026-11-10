#pragma once

#include "ast.hpp"
#include "environment.hpp"
#include "lexer.hpp"
#include <concepts>
#include <exception>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

#include "value.hpp"

class Interpreter {
  Environment environment;

public:
  void interpret(const std::vector<ASTNode> &ast) {
    for (const auto &node : ast) {
      evaluate(node);
    }
  }

  Value getVariable(const std::string &name) const {
    return environment.get(name);
  }

  Value
  evaluate(const ASTNode &node) { // prefix: s = string, b = bool, d = double!!
    return std::visit(
        [this](const auto &value) -> Value {
          using T = std::decay_t<decltype(value)>;

          if constexpr (std::is_same_v<T, NumberNode>) {
            return value.value;
          } else if constexpr (std::is_same_v<T, BooleanNode>) {
            return value.value;
          } else if constexpr (std::is_same_v<T, StringNode>) {
            return std::string(value.text);
          } else if constexpr (std::is_same_v<T, IdentifierNode>) {
            return environment.get(std::string(value.text));
          } else if constexpr (std::is_same_v<T, LocalDeclarationNode>) {
            Value result = evaluate(*value.initializer);
            environment.define(value.name, std::move(result));
            return result;
          } else if constexpr (std::is_same_v<T, AssignmentNode>) {
            Value result = evaluate(*value.assignment);
            environment.assign(value.name, std::move(result));
            return result;
          } else if constexpr (std::is_same_v<T, PrintNode>) {
            Value result = evaluate(*value.content);

            std::visit([](const auto &v) { std::cout << v << "\n"; }, result);

            return result;
          }

          else if constexpr (std::is_same_v<T, BinaryExprPtr>) {
            Value left = evaluate(*value->left);

            switch (value->op) {
            case TokenType::Plus: {
              Value right = evaluate(*value->right);
              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);

              if (!dL || !dR)
                throw std::runtime_error("Invalid operand types!");

              return *dL + *dR;
            }

            case TokenType::Minus: {
              Value right = evaluate(*value->right);

              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);

              if (!dL || !dR)
                throw std::runtime_error("Invalid operand types!");

              return *dL - *dR;
            }

            case TokenType::Star: {
              Value right = evaluate(*value->right);

              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);

              if (!dL || !dR)
                throw std::runtime_error("Invalid operand types!");

              return *dL * *dR;
            }

            case TokenType::Slash: {
              Value right = evaluate(*value->right);

              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);

              if (!dL || !dR)
                throw std::runtime_error("Invalid operand types!");

              if (*dR == 0)
                throw std::runtime_error("Trying to divide by 0!");

              return *dL / *dR;
            }
            case TokenType::EqualEqual: {
              Value right = evaluate(*value->right);

              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);
              if (dL && dR) {
                return *dL == *dR;
              }

              auto *bL = std::get_if<bool>(&left);
              auto *bR = std::get_if<bool>(&right);
              if (bL && bR) {
                return *bL == *bR;
              }

              auto *sL = std::get_if<std::string>(&left);
              auto *sR = std::get_if<std::string>(&right);
              if (sL && sR) {
                return *sL == *sR;
              }

              throw std::runtime_error("Invalid comparision!");
            }
            case TokenType::NotEqual: {
              Value right = evaluate(*value->right);

              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);
              if (dL && dR) {
                return *dL != *dR;
              }

              auto *bL = std::get_if<bool>(&left);
              auto *bR = std::get_if<bool>(&right);
              if (bL && bR) {
                return *bL != *bR;
              }

              auto *sL = std::get_if<std::string>(&left);
              auto *sR = std::get_if<std::string>(&right);
              if (sL && sR) {
                return *sL != *sR;
              }

              throw std::runtime_error("Invalid comparision!");
            }
            case TokenType::LessThan: {
              Value right = evaluate(*value->right);

              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);

              if (dL && dR)
                return *dL < *dR;

              throw std::runtime_error("Invalid comparision!");
            }
            case TokenType::GreaterThan: {
              Value right = evaluate(*value->right);

              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);

              if (dL && dR)
                return *dL > *dR;

              throw std::runtime_error("Invalid comparision!");
            }
            case TokenType::LessEqual: {
              Value right = evaluate(*value->right);

              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);

              if (dL && dR)
                return *dL <= *dR;

              throw std::runtime_error("Invalid comparision!");
            }
            case TokenType::GreaterEqual: {
              Value right = evaluate(*value->right);

              auto *dL = std::get_if<double>(&left);
              auto *dR = std::get_if<double>(&right);

              if (dL && dR)
                return *dL >= *dR;

              throw std::runtime_error("Invalid comparision!");
            }
            case TokenType::KwAnd: {
              auto *bL = std::get_if<bool>(&left);
              if (!bL)
                throw std::runtime_error("Invalid operand types!");

              if (bL) {
                if (!*bL)
                  return false;
              }

              Value right = evaluate(*value->right);
              auto *bR = std::get_if<bool>(&right);

              if (bL && bR) {
                return *bL && *bR;
              }

              throw std::runtime_error("Invalid comparision!");
            }
            case TokenType::KwOr: {
              auto *bL = std::get_if<bool>(&left);
              if (!bL)
                throw std::runtime_error("Invalid operand types!");

              if (bL) {
                if (*bL)
                  return true;
              }

              Value right = evaluate(*value->right);
              auto *bR = std::get_if<bool>(&right);

              if (bL && bR) {
                return *bL || *bR;
              }

              throw std::runtime_error("Invalid comparision!");
            }

            default:
              throw std::runtime_error("Unsupported AST Node!");
            }
          } else if constexpr (std::is_same_v<T, UnaryExprNode>) {
            Value operand = evaluate(*value.operand);

            auto *number = std::get_if<double>(&operand);
            if (!number)
              throw std::runtime_error("Invalid operand type!");

            if (value.op == TokenType::Minus)
              return -*number;

            throw std::runtime_error("Unsupported Unary operator!");
          } else {
            throw std::runtime_error("Unsupported AST Node!");
          }
        },
        node);
  }
};

inline void showInterpreter(const std::vector<Value> &results) {
  std::cout << "Interpreter Results\n";

  for (size_t i = 0; i < results.size(); ++i) {
    std::cout << "[" << i << "] ";

    std::visit(
        [](const auto &value) {
          using T = std::decay_t<decltype(value)>;

          if constexpr (std::is_same_v<T, double>) {
            std::cout << "Type: double\n";
            std::cout << "Value: " << value << '\n';
          } else if constexpr (std::is_same_v<T, bool>) {
            std::cout << "Type: bool\n";
            std::cout << "Value: " << std::boolalpha << value << '\n';
          } else if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "Type: string\n";
            std::cout << "Value: " << value << '\n';
          }
        },
        results[i]);

    std::cout << '\n';
  }

  std::cout << "\n";
}
