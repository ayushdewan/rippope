# Test configuration
add_executable(piece_table_test 
    main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../piece_table.c
)

target_include_directories(piece_table_test PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

# Copy test file to build directory
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test.txt ${CMAKE_CURRENT_BINARY_DIR}/test.txt COPYONLY)

# Add test
add_test(NAME piece_table_test COMMAND piece_table_test test.txt)