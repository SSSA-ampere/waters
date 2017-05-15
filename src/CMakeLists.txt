# Include dirs.
include_directories(.)
include_directories(./tinyxml)

set (SOURCE_FILES
#  annealing.cpp
  copy_in_newstruct.cpp
  EventChains2.cpp
  EventChains2_elem.cpp
  genetic.cpp
  Label2.cpp
  main.cpp
  main_waters.cpp
  milpData.cpp
  optimization.cpp
  RT.cpp
  Runnable2.cpp
  strtools.cpp
  Task2.cpp
  analyse_data.cpp)

add_executable (${PROJECT_NAME} ${SOURCE_FILES})

add_definitions(-DMODEL_PATH="${MODEL_PATH}")

if (USE_LET_MODEL)
  add_definitions(-DUSE_LET_MODEL=true)
else(USE_LET_MODEL)
  add_definitions(-DUSE_LET_MODEL=false)
endif(USE_LET_MODEL)

target_compile_features (${PROJECT_NAME} PRIVATE cxx_range_for)

add_library (tinyxml STATIC tinyxml/tinyxml2.cpp)

target_link_libraries (${PROJECT_NAME} tinyxml)
