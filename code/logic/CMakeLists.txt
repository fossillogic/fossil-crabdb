# code/logic/CMakeLists.txt

# Define the include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Collect all header files in logic/fossil/*
file(GLOB HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/fossil/*.h)

# List the source files
set(TEST_CODE
    serialize.c
    parser.c
    error.c
    input.c
    output.c
    soap.c
    stream.c
    keyboard.c
    network.c
)

# Create the library target
add_library(fossil-crabdb STATIC ${TEST_CODE} ${HEADER_FILES})

# Link the math library
target_link_libraries(fossil-crabdb PUBLIC m)

# Link to Threads library
find_package(Threads REQUIRED)
target_link_libraries(fossil-crabdb PUBLIC Threads::Threads)

# Set the library to be installed
install(TARGETS fossil-crabdb
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install the header files
install(FILES ${HEADER_FILES} DESTINATION include/fossil)

# Declare the dependency
add_library(fossil_crabdb_dep INTERFACE)
target_link_libraries(fossil_crabdb_dep INTERFACE fossil-crabdb)
target_include_directories(fossil_crabdb_dep INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
