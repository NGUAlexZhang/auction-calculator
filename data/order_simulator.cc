#pragma once

#include "order_simulator.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <initializer_list>

OrderSimulator::OrderSimulator(const std::filesystem::path& order_path) {
    this->read_csv(order_path);
}


void OrderSimulator::read_csv(const std::filesystem::path& order_path) {
    this->file.open(order_path);
    if (!this->file.is_open()) {
        throw std::runtime_error("Could not open file: " + order_path.string());
    }
    std::string header_line;
    if (!std::getline(this->file, header_line)) {
        throw std::runtime_error("Could not read header from file: " + order_path.string());
    }
    this->column_names.clear();
    std::stringstream ss(header_line);
    std::string column_name;
    while (std::getline(ss, column_name, ',')) {
        this->column_names.push_back(column_name);
    }
}

Order OrderSimulator::next_order(){
    std::string line;
    if(this->file.eof()) {
        throw std::runtime_error("End of file reached");
    }
    std::getline(this->file, line);
    std::stringstream ss(line);
    std::vector<std::string> values;
    std::string column;
    while(std::getline(ss, column, ',')) {
        values.push_back(column);
    }
    if (values.size() != this->column_names.size()) {
        throw std::runtime_error("Column count mismatch in line: " + line);
    }
    Order order;
    std::istringstream datetime_stream(values[0]);
    datetime_stream >> std::chrono::parse("%Y/%m/%d %H:%M:%S", order.datetime);
    order.sym = values[1];
    order.price = std::stod(values[2]);
    order.size = std::stoull(values[3]);
    order.side = std::stoi(values[4]);
    order.order_type = std::stoul(values[5]);
    order.order_id = std::stoull(values[6]);
    order.channel_no = std::stoul(values[7]);
    order.seq_no = std::stoll(values[8]);
    order.biz_index = std::stoll(values[9]);
    return order;
}