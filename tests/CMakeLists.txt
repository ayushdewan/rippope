# Test configuration
add_executable(piece_table_test main.c)
target_include_directories(piece_table_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

# Copy test file to build directory
configure_file(test.txt test.txt COPYONLY)

# Add test
add_test(NAME piece_table_test COMMAND piece_table_test test.txt)