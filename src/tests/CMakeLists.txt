
find_package (Python COMPONENTS Interpreter)

add_custom_target(pre_tests)
add_executable (tests tests.cpp tests_static.cpp tests_gen.cpp)
add_dependencies(tests pre_tests)

target_link_libraries (tests LINK_PUBLIC SeriStruct)
target_compile_features (tests PUBLIC cxx_std_17)

ParseAndAddCatchTests(tests)

add_custom_command(
     TARGET pre_tests
     COMMENT "Generate records"
     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
     COMMAND ${Python_EXECUTABLE} ../idl/ssgen.py -i GenRecords.txt -o .
)

add_custom_command(
     TARGET tests
     COMMENT "Run tests"
     POST_BUILD 
     WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
     COMMAND ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION> --progress --output-on-failure
)