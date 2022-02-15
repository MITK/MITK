file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  mitkCESTImageNormalizationFilter.cpp
  mitkCustomTagParser.cpp
  mitkCESTImageDetectionHelper.cpp
  mitkExtractCESTOffset.cpp
  mitkCESTPropertyHelper.cpp
)

set(RESOURCE_FILES
  1416.json
  1485.json
  1494.json
  1500.json
  1503.json
  1504.json
  1519.json
  1520.json
  1521.json
  1522.json
  1583.json
)
