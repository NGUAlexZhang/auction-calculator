set(TEST_DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../test_data/")

enable_testing()

add_executable(data_test
               data_test.cc)

add_executable(order_book_test
               order_book_test.cc)

add_executable(call_auction_test
               call_auction_test.cc)

add_executable(event_processor_test
               event_processor_test.cc)

target_link_libraries(data_test PRIVATE 
                      gtest_main)
target_include_directories(data_test PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/../data
                           ${CMAKE_CURRENT_SOURCE_DIR}/../model)
target_link_libraries(data_test PRIVATE
                      data_reader)

target_link_libraries(order_book_test PRIVATE
                      gtest_main
                      order_book)
target_include_directories(order_book_test PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/../data)

target_link_libraries(call_auction_test PRIVATE
                      gtest_main
                      auction_lib)
target_include_directories(call_auction_test PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/../auction
                           ${CMAKE_CURRENT_SOURCE_DIR}/../model)

target_link_libraries(event_processor_test PRIVATE
                      gtest_main
                      data_reader)
target_include_directories(event_processor_test PRIVATE
                           ${CMAKE_CURRENT_SOURCE_DIR}/../auction
                           ${CMAKE_CURRENT_SOURCE_DIR}/../data
                           ${CMAKE_CURRENT_SOURCE_DIR}/../model)

target_compile_definitions(order_book_test PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")
target_compile_definitions(data_test PRIVATE TEST_DATA_DIR="${TEST_DATA_DIR}")
include(GoogleTest)

gtest_discover_tests(data_test)
gtest_discover_tests(order_book_test)
gtest_discover_tests(call_auction_test)
gtest_discover_tests(event_processor_test)
