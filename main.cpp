
#include <cstdio>
#include <exception>
#include <fstream>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "lexer.hpp"
#include "parser.hpp"

class Filesystem {
  FILE *m_file = nullptr;

  void closeFile() noexcept {
    if (m_file == nullptr)
      return;

    if (std::fclose(m_file) != 0) {
      std::cerr << "Failed to close File!\n";
    } else {
      std::cout << "Successfully closed File!\n";
    }

    m_file = nullptr;
  }

public:
  std::string readAll() {
    if (m_file == nullptr)
      throw std::runtime_error("File cant be read!");

    if (std::fseek(m_file, 0, SEEK_END) != 0) {
      throw std::runtime_error("Seek Failed!");
    }

    long size = std::ftell(m_file);

    if (size < 0)
      throw std::runtime_error("Tell Failed!");

    std::rewind(m_file);

    std::string content(static_cast<size_t>(size), '\0');
    size_t read = std::fread(content.data(), 1, content.size(), m_file);
    content.resize(read);

    return content;
  }

  Filesystem(const char *path, const char *mode) {
    m_file = std::fopen(path, mode);

    if (m_file == nullptr) {
      throw std::runtime_error("Error Opening File!");
    }

    std::cout << "Opened File: " << path << "\n";
  }

  ~Filesystem() { closeFile(); }

  Filesystem(Filesystem &&other) noexcept : m_file(other.m_file) {
    other.m_file = nullptr;
  }

  Filesystem &operator=(Filesystem &&other) noexcept {
    if (this == &other) {
      return *this;
    }

    closeFile();
    m_file = other.m_file;
    other.m_file = nullptr;

    return *this;
  }
};

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Calculator requires atleast one File Argument!\n";
  }

  try {
    Filesystem filesystem(argv[1], "rb");
    std::string content = filesystem.readAll();
    std::string_view view = content;

    std::cout << view.size() << "\n";

    Lexer lexer(view);
    std::vector<Token> tokens = lexer.tokenize();

    for (auto t : tokens) {
      std::cout << tokenTypeToString(t.type) << "\n";
    }

    Parser parser(tokens);
    parser.parse();

  } catch (const std::exception &except) {
    std::cerr << "Trapped: " << except.what() << "\n";
    return 1;
  }

  return 0;
}
