#pragma once

#include <algorithm>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "value.hpp"

class Environment {
  std::unordered_map<std::string, Value> environment;

public:
  void define(const std::string &name, Value value) {
    auto [it, inserted] = environment.emplace(name, std::move(value));
    if (!inserted)
      throw std::runtime_error("Variable " + name + " was already defined!");
  }
  Value get(const std::string &name) const {
    auto it = environment.find(name);
    if (it == environment.end()) {
      throw std::runtime_error("Key: " + name + " could not be found!");
    }

    return it->second;
  }
  bool contains(const std::string &name) const {
    return environment.contains(name);
  }

  void assign(const std::string &name, Value value) {
    auto it = environment.find(name);
    if (it == environment.end())
      throw std::runtime_error("Cannot assign to undefined variable: " + name);
    it->second = std::move(value);
  }
};
