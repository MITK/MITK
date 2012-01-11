FIND_PACKAGE(OpenGL)
IF(NOT OPENGL_FOUND)
  MESSAGE("GL is required for MITK rendering")
ENDIF(NOT OPENGL_FOUND )

SET(TOOL_CPPS "")

MITK_CREATE_MODULE( Mitk
  INCLUDE_DIRS Algorithms DataManagement Controllers Interactions IO Rendering Service ${MITK_BINARY_DIR}
  INTERNAL_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} ${IPSEGMENTATION_INCLUDE_DIR} ${ANN_INCLUDE_DIR}
  DEPENDS mbilog tinyxml
  DEPENDS_INTERNAL IIL4MITK pic2vtk
  PACKAGE_DEPENDS ITK VTK
  EXPORT_DEFINE MITK_CORE_EXPORT
)

# this is needed for libraries which link to Mitk and need
# symbols from explicitly instantiated templates like
# mitk::SurfaceVtkWriter<vtkSTLWriter> which is referenced in
# QmitkCommonFunctionality in the QmitkExt library.
IF(MINGW)
  GET_TARGET_PROPERTY(_mitkCore_MINGW_linkflags Mitk LINK_FLAGS)
  IF(NOT _mitkCore_MINGW_linkflags)
    SET(_mitkCore_MINGW_linkflags "")
  ENDIF(NOT _mitkCore_MINGW_linkflags)
  SET_TARGET_PROPERTIES(Mitk PROPERTIES LINK_FLAGS "${_mitkCore_MINGW_linkflags} -Wl,--export-all-symbols")
ENDIF(MINGW)

# replacing Mitk by Mitk [due to removing the PROVIDES macro
TARGET_LINK_LIBRARIES(Mitk ${LIBRARIES_FOR_${KITNAME}_CORE}  ${IPFUNC_LIBRARY} ipSegmentation ann)
TARGET_LINK_LIBRARIES(Mitk ${OPENGL_LIBRARIES} )
TARGET_LINK_LIBRARIES(Mitk gdcmCommon gdcmIOD gdcmDSED)

IF(MSVC_IDE OR MSVC_VERSION OR MINGW)
  TARGET_LINK_LIBRARIES(Mitk psapi.lib)
ENDIF(MSVC_IDE OR MSVC_VERSION OR MINGW)


# verify ITK has been built with GDCM >= 2.0.14
set(GDCM_FULL_VERSION "${GDCM_MAJOR_VERSION}.${GDCM_MINOR_VERSION}.${GDCM_BUILD_VERSION}")
set(MITK_REQUIRED_GDCM_VERSION "2.0.14")
if(GDCM_FULL_VERSION VERSION_LESS MITK_REQUIRED_GDCM_VERSION)
  message(SEND_ERROR "Mitk: MITK requires ITK with at least GDCM version ${MITK_REQUIRED_GDCM_VERSION}.\nFound version ${GDCM_FULL_VERSION} (GDCM NOT found if you don't see a version here)")
else(GDCM_FULL_VERSION VERSION_LESS MITK_REQUIRED_GDCM_VERSION)
  message(STATUS "Mitk: Found GDCM version ${GDCM_FULL_VERSION}")
endif(GDCM_FULL_VERSION VERSION_LESS MITK_REQUIRED_GDCM_VERSION)


# build tests?
OPTION(BUILD_TESTING "Build the MITK Core tests." ON)
IF(BUILD_TESTING)
ENABLE_TESTING()
ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
