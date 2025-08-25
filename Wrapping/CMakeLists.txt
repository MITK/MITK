if(MITK_USE_SWIG)
  include(UseSWIG)
  include(mitkSwigPrepareFiles)

  # Path to common files
  set(MITK_WRAPPING_COMMON_DIR "${MITK_SOURCE_DIR}/Wrapping/Common")

  add_subdirectory(Python)
endif()
