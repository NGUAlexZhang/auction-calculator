#ifndef __DATA_SIMULATOR_H__
#include "data_simulator.h"
#endif

template<CanReadFromCSV T>
DataSimulator<T>::DataSimulator(const std::filesystem::path& file_path) {
  this->read_csv(file_path);
}

template<CanReadFromCSV T>
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

template<CanReadFromCSV T>
void DataSimulator<T>::read_next_obj(T& obj) {
  std::string line;
  if (this->file.eof()) {
    throw std::runtime_error("End of file reached");
  }
  std::getline(this->file, line);
  std::istringstream ss(line);
  ss >> obj;
}

template<CanReadFromCSV T>
bool DataSimulator<T>::has_next() const { return !this->file.eof(); }

template<CanReadFromCSV T>
DataSimulator<T>& DataSimulator<T>::operator >> (T& obj) {
  this->read_next_obj(obj);
  return *this;
}

template<CanReadFromCSV T>
DataSimulator<T>::operator bool() const {
  return this->file.is_open() && !this->file.eof();
}

template<CanReadFromCSV T>
DataSimulator<T>::~DataSimulator() {
  if (this->file.is_open()) {
    this->file.close();
  }
}
