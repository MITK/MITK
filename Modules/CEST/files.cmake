file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkCESTImageNormalizationFilter.cpp
  mitkCustomTagParser.cpp
  mitkCESTImageDetectionHelper.cpp
)

set(RESOURCE_FILES
  1416.json
  1485.json
  1494.json
)
