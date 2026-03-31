cmake_minimum_required(VERSION 3.20...4.2.3)

project(AuctionCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(order_simulator SHARED
            data/order_simulator.cc)
target_include_directories(order_simulator PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}/model
                           ${CMAKE_CURRENT_SOURCE_DIR}/data) 

add_library(order_book SHARED
            model/order_book.cc)

target_include_directories(order_book PUBLIC 
                           ${CMAKE_CURRENT_SOURCE_DIR}/model
                           ${CMAKE_CURRENT_SOURCE_DIR}/utils)
add_subdirectory(third_party/googletest)
add_subdirectory(tests)