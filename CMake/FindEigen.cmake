find_path(Eigen_INCLUDE_DIR
  NAMES Eigen/Eigen
  PATHS ${Eigen_DIR} ${MITK_EXTERNAL_PROJECT_PREFIX} ${CMAKE_PREFIX_PATH}
  PATH_SUFFIXES include include/eigen3
)

if (NOT TARGET Eigen)
  add_library(Eigen INTERFACE IMPORTED GLOBAL)
  set_property(TARGET Eigen APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Eigen_INCLUDE_DIR})
endif()

find_package_handle_standard_args(Eigen
  FOUND_VAR Eigen_FOUND
  REQUIRED_VARS Eigen_INCLUDE_DIR
)

