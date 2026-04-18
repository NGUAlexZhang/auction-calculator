#pragma once
#define __DATA_SIMULATOR_H__
#include <concepts>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

template <typename T>
concept CanReadFromCSV = requires(std::istream& is, T& obj) {
  { is >> obj } -> std::same_as<std::istream&>;
};

template <CanReadFromCSV T>
class DataSimulator {
 public:
  explicit DataSimulator(const std::filesystem::path& file_path);
  ~DataSimulator();
  [[nodiscard]] bool has_next() const;
  DataSimulator& operator>>(T& obj);
  explicit operator bool() const;

 private:
  void read_csv(const std::filesystem::path& file_path);
  std::ifstream file;
  std::vector<std::string> column_names;
  std::uint32_t line_number;
};

#include "data_simulator.tpp"