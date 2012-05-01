#configure_file(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactory.cpp.in $#{PROJECT_BINARY_DIR}/ToolExtensionITKFactory.cpp.in COPYONLY)
#configure_file(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactoryLoader.cpp.in $#{PROJECT_BINARY_DIR}/ToolExtensionITKFactoryLoader.cpp.in COPYONLY)
#configure_file(${PROJECT_SOURCE_DIR}/CMake/ToolGUIExtensionITKFactory.cpp.in $#{PROJECT_BINARY_DIR}/ToolGUIExtensionITKFactory.cpp.in COPYONLY)

MITK_CREATE_MODULE( Segmentation
               INCLUDE_DIRS Algorithms Controllers DataManagement Interactions IO Rendering
               DEPENDS Mitk ipSegmentation mitkIpFunc MitkExt 
)

add_subdirectory(Testing)