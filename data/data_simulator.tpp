#ifndef __DATA_SIMULATOR_H__
#include "data_simulator.h"
#endif

template <CanReadFromCSV T>
DataSimulator<T>::DataSimulator(const std::filesystem::path& file_path) {
  this->read_csv(file_path);
}

template <CanReadFromCSV T>
void DataSimulator<T>::read_csv(const std::filesystem::path& file_path) {
  this->file.open(file_path);
  if (!this->file.is_open()) {
    throw std::runtime_error("Could not open file: " + file_path.string());
  }
  std::string header_line;
  if (!std::getline(this->file, header_line)) {
    throw std::runtime_error("Could not read header from file: " +
                             file_path.string());
  }
  this->column_names.clear();
  this->line_number = 0;
  std::istringstream ss(header_line);
  std::string column_name;
  while (std::getline(ss, column_name, ',')) {
    this->column_names.push_back(column_name);
  }
}

template <CanReadFromCSV T>
bool DataSimulator<T>::has_next() const {
  return !this->file.eof();
}

template <CanReadFromCSV T>
DataSimulator<T>& DataSimulator<T>::operator>>(T& obj) {
  std::string line;

  if (!std::getline(this->file, line)) {
    this->file.setstate(std::ios::failbit);
    return *this;
  }

  std::istringstream ss(line);
  if (!(ss >> obj)) {
    this->file.setstate(std::ios::failbit);
  }

  return *this;
}

template <CanReadFromCSV T>
DataSimulator<T>::operator bool() const {
  return static_cast<bool>(this->file);
}

template <CanReadFromCSV T>
DataSimulator<T>::~DataSimulator() {
  if (this->file.is_open()) {
    this->file.close();
  }
}
