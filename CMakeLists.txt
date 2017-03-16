cmake_minimum_required (VERSION 2.6)
project (WATERS2017)

set(MODEL_PATH
  "${CMAKE_SOURCE_DIR}/model/ChallengeModel_w_LabelStats_fixedLabelMapping_App4mc_v072.amxmi"
  CACHE STRING "Filepath of the model file")

message("Model path: " ${MODEL_PATH})

# Include dirs.
add_subdirectory (src)
