#if(WIN32)
#  option(MITK_USE_TD_MOUSE "Enable support for 3D Connexion SpaceNavigator" OFF)
#endif(WIN32)

configure_file(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactory.cpp.in COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/CMake/ToolExtensionITKFactoryLoader.cpp.in ${PROJECT_BINARY_DIR}/ToolExtensionITKFactoryLoader.cpp.in COPYONLY)
configure_file(${PROJECT_SOURCE_DIR}/CMake/ToolGUIExtensionITKFactory.cpp.in ${PROJECT_BINARY_DIR}/ToolGUIExtensionITKFactory.cpp.in COPYONLY)

MITK_CREATE_MODULE( MitkExt
               INCLUDE_DIRS Algorithms Controllers DataManagement Interactions IO Rendering
               DEPENDS Mitk LegacyAdaptors IpPicSupport ipSegmentation mitkIpFunc PlanarFigure ImageStatistics
               DEPENDS_INTERNAL pic2vtk
)

# this is needed for libraries which link to mitkCoreExt and need
# symbols from explicitly instantiated templates like
# mitk::UnstructuredGridVtkWriter<vtkUnstructuredGridWriter> which is referenced in
# mitk::UnstructuredGridVtkWriterTest in the MitkExtTestDriver executable.
if(MINGW)
  get_target_property(_mitkCoreExt_MINGW_linkflags mitkCoreExt LINK_FLAGS)
  if(NOT _mitkCoreExt_MINGW_linkflags)
    set(_mitkCoreExt_MINGW_linkflags "")
  endif(NOT _mitkCoreExt_MINGW_linkflags)
  set_target_properties(mitkCoreExt PROPERTIES LINK_FLAGS "${_mitkCoreExt_MINGW_linkflags} -Wl,--export-all-symbols")
endif(MINGW)

add_subdirectory(Testing)

