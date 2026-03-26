#pragma once
#include <filesystem>
#include <fstream>
#include <vector>
#include "model/order.h"

class OrderSimulator {
public:
    explicit OrderSimulator() = default;
    explicit OrderSimulator(const std::filesystem::path& order_path);
    [[nodiscard]] Order next_order();

private:
    void read_csv(const std::filesystem::path& order_path);
    std::ifstream file;
    bool eof_reached;
    std::vector<std::string> column_names;
};