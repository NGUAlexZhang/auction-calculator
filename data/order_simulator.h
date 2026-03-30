#pragma once
#include <filesystem>
#include <fstream>
#include <vector>

#include "order.h"

class OrderSimulator {
 public:
  explicit OrderSimulator() = delete;
  explicit OrderSimulator(const std::filesystem::path& order_path);
  ~OrderSimulator();
  [[nodiscard]] Order next_order();
  [[nodiscard]] bool has_next() const;

 private:
  void read_csv(const std::filesystem::path& order_path);
  std::ifstream file;
  std::vector<std::string> column_names;
  std::uint32_t line_number;
};