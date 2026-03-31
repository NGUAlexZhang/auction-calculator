cmake_minimum_required(VERSION 3.20...4.2.3)

project(AuctionCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent) 

add_subdirectory(third_party/googletest)


enable_testing()

set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/test_data/")
add_library(order_simulator 
            data/order_simulator.cc)
target_include_directories(order_simulator PUBLIC 
                           model
                           data) 
add_executable(data_test
               tests/data_test.cc)
target_link_libraries(data_test PRIVATE 
                      gtest_main
                      order_simulator)
target_compile_definitions(data_test PRIVATE
                           TEST_DATA_DIR="${TEST_DATA_DIR}")

add_library(order_book
            model/order_book.cc)

target_include_directories(order_book PUBLIC 
                           model,
                           utils)


add_executable(order_book_test
               tests/order_book_test.cc)
target_link_libraries(order_book_test PRIVATE
                        gtest_main
                        order_book
                        order_simulator)
target_compile_definitions(order_book_test PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")

include(GoogleTest)

gtest_discover_tests(data_test)
gtest_discover_tests(order_book_test)