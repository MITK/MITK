#if(WIN32)
#  option(MITK_USE_TD_MOUSE "Enable support for 3D Connexion SpaceNavigator" OFF)
#endif(WIN32)

configure_file(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactory.cpp.in COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactoryLoader.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactoryLoader.cpp.in COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/CMake/ToolGUIExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolGUIExtensionITKFactory.cpp.in COPYONLY)

MITK_CREATE_MODULE( MitkExt
               INCLUDE_DIRS Algorithms Controllers DataManagement Interactions IO Rendering
               DEPENDS MitkDataTypesExt ImageExtraction IpPicSupport
               PACKAGE_DEPENDS ANN ITK|ITKConnectedComponents+ITKBinaryMathematicalMorphology
               DEPRECATED_SINCE 2014.03
)

if(BUILD_TESTING)
  add_subdirectory(Testing)
endif()

