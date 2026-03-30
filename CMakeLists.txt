cmake_minimum_required(VERSION 4.2.3)

project(AuctionCalculator VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent) 
FetchContent_Declare(googletest 
                    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

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
            engine/order_book.cc)

target_include_directories(order_book PUBLIC 
                           engine
                           model)


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