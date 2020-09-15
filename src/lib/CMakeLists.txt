add_library (SeriStruct SeriStruct.cpp)
target_include_directories (SeriStruct PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features (SeriStruct PUBLIC cxx_std_17)