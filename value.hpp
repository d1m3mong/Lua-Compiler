#pragma once

#include <string>
#include <variant>

using Value = std::variant<double, bool, std::string>;
