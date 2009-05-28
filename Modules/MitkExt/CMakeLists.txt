IF(WIN32)
  OPTION(MITK_USE_TD_MOUSE "Enable support for 3D Connexion SpaceNavigator" OFF)
ENDIF(WIN32)

MITK_GENERATE_TOOLS_LIBRARY()

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactory.cpp.in COPYONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactoryLoader.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactoryLoader.cpp.in COPYONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/CMake/ToolGUIExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolGUIExtensionITKFactory.cpp.in COPYONLY)

MITK_CREATE_MODULE( MitkExt
               INCLUDE_DIRS Algorithms Controllers DataManagement Interactions IO Rendering
               DEPENDS Mitk ipSegmentation
               PROVIDES mitkCoreExt
               GENERATED_CPP ${TOOL_CPPS}
)



