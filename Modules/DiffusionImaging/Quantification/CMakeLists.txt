# With apple gcc 4.2.1 the following waring leads to an build error if boost is enabled
if(APPLE)
  mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=empty-body" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
endif()

#DiffusionImaging/Quantification
MITK_CREATE_MODULE(
  SUBPROJECTS MITK-DTI
  INCLUDE_DIRS Algorithms IODataStructures IODataStructures/TbssImages Rendering ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS MitkDiffusionCore MitkFiberTracking MitkGraphAlgorithms
  WARNINGS_AS_ERRORS
)

add_subdirectory(Testing)
