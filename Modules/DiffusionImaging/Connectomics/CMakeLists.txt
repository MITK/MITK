# With apple clang 5.0 the following waring leads to an build error if boost is enabled
if(APPLE)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=tautological-constant-out-of-range-compare" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

# Boost throws a lot of unused variable warnings on linux
if(UNIX)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=unused-variable" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=unneeded-internal-declaration" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

# there are additional warnings in boost headers in Release mode with special gcc versions
if(CMAKE_COMPILER_IS_GNUCXX)
  if (CMAKE_BUILD_TYPE STREQUAL "Release" AND (${GCC_VERSION} VERSION_EQUAL "4.8" OR ${GCC_VERSION} VERSION_GREATER "4.8"))
    mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=maybe-uninitialized" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
    mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=unused-parameter" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
  endif()
endif()

MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms Algorithms/BrainParcellation IODataStructures Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkDiffusionCore MitkFiberTracking
  PACKAGE_DEPENDS VTK|vtkInfovisLayout
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
