cmake_minimum_required (VERSION 2.6)
project (WATERS2017)

set(MODEL_PATH
  "${CMAKE_SOURCE_DIR}/model/ChallengeModel2017modified.amxmi"
  CACHE STRING "Filepath of the model file")

#set(USE_LET_MODEL off)
set(USE_LET_MODEL off)

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_CXX_FLAGS_RELEASE -Ofast)

message(">> Model path: " ${MODEL_PATH})
if (USE_LET_MODEL)
  message(">> Using LET")
else(USE_LET_MODEL)
  message(">> Using NORMAL model")
endif(USE_LET_MODEL)

# Include dirs.
add_subdirectory (src)
