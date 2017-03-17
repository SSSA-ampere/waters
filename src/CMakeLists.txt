# Include dirs.
include_directories(.)
include_directories(./tinyxml)

set (SOURCE_FILES main.cpp strtools.cpp Task2.cpp Runnable2.cpp
  Label2.cpp EventChains2.cpp EventChains2_elem.cpp milpData.cpp copy_in_newstruct.cpp)

add_executable (${PROJECT_NAME} ${SOURCE_FILES})

add_definitions(-DMODEL_PATH="${MODEL_PATH}")
target_compile_features (${PROJECT_NAME} PRIVATE cxx_range_for)

add_library (tinyxml STATIC tinyxml/tinyxml2.cpp)

target_link_libraries (${PROJECT_NAME} tinyxml)
