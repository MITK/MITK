FIND_PACKAGE(OpenGL)
IF(NOT OPENGL_FOUND)
  MESSAGE("GL is required for MITK rendering")
ENDIF(NOT OPENGL_FOUND )
IF(CMAKE_COMPILER_IS_GNUCC)
  OPTION(MITK_USE_GCC_VISIBILITY "Use gcc's -fvisibility option to hide exported symbols" OFF) 
  IF(MITK_USE_GCC_VISIBILITY)
    ADD_DEFINITIONS("-fvisibility=hidden")
  ENDIF(MITK_USE_GCC_VISIBILITY)
ENDIF(CMAKE_COMPILER_IS_GNUCC)


SET(TOOL_CPPS "")
IF(MITK_TEST_COVERAGE)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
ENDIF(MITK_TEST_COVERAGE)

MITK_CREATE_MODULE( Mitk
  INCLUDE_DIRS Algorithms DataManagement Controllers Interactions IO Rendering ${MITK_BINARY_DIR}
  INTERNAL_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} ${IPSEGMENTATION_INCLUDE_DIR} ${ANN_INCLUDE_DIR}
  PROVIDES mitkCore
  DEPENDS mitkIpPic mbilog tinyxml
  DEPENDS_INTERNAL IIL4MITK pic2vtk
  PACKAGE_DEPENDS ITK VTK
)

# this is needed for libraries which link to mitkCore and need
# symbols from explicitly instantiated templates like
# mitk::SurfaceVtkWriter<vtkSTLWriter> which is referenced in
# QmitkCommonFunctionality in the QmitkExt library.
IF(MINGW)
  GET_TARGET_PROPERTY(_mitkCore_MINGW_linkflags mitkCore LINK_FLAGS)
  IF(NOT _mitkCore_MINGW_linkflags)
    SET(_mitkCore_MINGW_linkflags "")
  ENDIF(NOT _mitkCore_MINGW_linkflags)
  SET_TARGET_PROPERTIES(mitkCore PROPERTIES LINK_FLAGS "${_mitkCore_MINGW_linkflags} -Wl,--export-all-symbols")
ENDIF(MINGW)

TARGET_LINK_LIBRARIES(mitkCore ${LIBRARIES_FOR_${KITNAME}_CORE}  ${IPFUNC_LIBRARY} ipSegmentation ann)
TARGET_LINK_LIBRARIES(mitkCore ${OPENGL_LIBRARIES} )

IF(MSVC_IDE OR MSVC_VERSION OR MINGW)
  TARGET_LINK_LIBRARIES(mitkCore psapi.lib)
ENDIF(MSVC_IDE OR MSVC_VERSION OR MINGW)

OPTION(BUILD_TESTING "Build the MITK Core tests." ON)
IF(BUILD_TESTING)
ENABLE_TESTING()
ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
