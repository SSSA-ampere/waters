# Include dirs.
include_directories(.)
include_directories(./tinyxml)

set (SOURCE_FILES main.cpp)

add_executable (${PROJECT_NAME} ${SOURCE_FILES})

target_compile_features (${PROJECT_NAME} PRIVATE cxx_range_for)
