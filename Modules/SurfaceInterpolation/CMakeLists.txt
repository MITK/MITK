MITK_CREATE_MODULE(
  DEPENDS MitkContourModel MitkAlgorithmsExt MitkImageStatistics
  PACKAGE_DEPENDS PUBLIC ITK|ITKEigen3
)

if(TARGET ${MODULE_TARGET})
  # MitkAlgorithmsExt propagates OpenMP, which makes Eigen parallelize the dense
  # solve of the surface interpolation. That runs on a worker thread next to the
  # thread pools of the application, where the competing threads made it about
  # ten times slower than solving on a single thread.
  target_compile_definitions(${MODULE_TARGET} PRIVATE EIGEN_DONT_PARALLELIZE)
endif()

add_subdirectory(Testing)
