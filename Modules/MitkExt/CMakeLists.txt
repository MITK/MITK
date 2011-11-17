#IF(WIN32)
#  OPTION(MITK_USE_TD_MOUSE "Enable support for 3D Connexion SpaceNavigator" OFF)
#ENDIF(WIN32)

CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactory.cpp.in COPYONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactoryLoader.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactoryLoader.cpp.in COPYONLY)
CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/CMake/ToolGUIExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolGUIExtensionITKFactory.cpp.in COPYONLY)

MITK_CREATE_MODULE( MitkExt
               INCLUDE_DIRS Algorithms Controllers DataManagement Interactions IO Rendering
               DEPENDS Mitk LegacyAdaptors IpPicSupport ipSegmentation mitkIpFunc PlanarFigure ImageStatistics
               DEPENDS_INTERNAL pic2vtk IIL4MITK
)

# this is needed for libraries which link to mitkCoreExt and need
# symbols from explicitly instantiated templates like
# mitk::UnstructuredGridVtkWriter<vtkUnstructuredGridWriter> which is referenced in
# mitk::UnstructuredGridVtkWriterTest in the MitkExtTestDriver executable.
IF(MINGW)
  GET_TARGET_PROPERTY(_mitkCoreExt_MINGW_linkflags mitkCoreExt LINK_FLAGS)
  IF(NOT _mitkCoreExt_MINGW_linkflags)
    SET(_mitkCoreExt_MINGW_linkflags "")
  ENDIF(NOT _mitkCoreExt_MINGW_linkflags)
  SET_TARGET_PROPERTIES(mitkCoreExt PROPERTIES LINK_FLAGS "${_mitkCoreExt_MINGW_linkflags} -Wl,--export-all-symbols")
ENDIF(MINGW)

ADD_SUBDIRECTORY(Testing)

