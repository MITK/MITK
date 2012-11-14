#FIND_PACKAGE(OpenGL)
#IF(NOT OPENGL_FOUND)
#  MESSAGE("GL is required for MITK rendering")
#ENDIF(NOT OPENGL_FOUND )

# Configure the C++ Micro Services Code for MITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

set(US_NAMESPACE "mitk")
set(US_HEADER_PREFIX "mitk")
set(US_BASECLASS_NAME "itk::LightObject")
set(US_BASECLASS_HEADER "mitkServiceBaseObject.h")
set(US_BASECLASS_PACKAGE "ITK")
set(US_ENABLE_THREADING_SUPPORT 1)
set(US_ENABLE_AUTOLOADING_SUPPORT 1)
set(US_EMBEDDING_LIBRARY Mitk)
set(US_BUILD_TESTING ${BUILD_TESTING})
if(BUILD_TESTING)
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Common)
  set(US_TEST_LABELS MITK-Core)
endif()

add_subdirectory(CppMicroServices)

include(${CMAKE_CURRENT_BINARY_DIR}/CppMicroServices/CppMicroServicesConfig.cmake)

set(TOOL_CPPS "")

# temporary suppress warnings in the following files until image accessors are fully integrated.
set_source_files_properties( DataManagement/mitkImage.cpp COMPILE_FLAGS -DMITK_NO_DEPRECATED_WARNINGS )
set_source_files_properties( Controllers/mitkSliceNavigationController.cpp COMPILE_FLAGS -DMITK_NO_DEPRECATED_WARNINGS )

MITK_CREATE_MODULE( Mitk
  INCLUDE_DIRS
    ${CppMicroServices_INCLUDE_DIRS} Algorithms Common DataManagement Controllers
    Interactions Interfaces IO Rendering ${MITK_BINARY_DIR}
  INTERNAL_INCLUDE_DIRS
    ${OPENGL_INCLUDE_DIR} ${IPSEGMENTATION_INCLUDE_DIR} ${ANN_INCLUDE_DIR}
    ${CppMicroServices_INTERNAL_INCLUDE_DIRS}
  DEPENDS mbilog tinyxml
  DEPENDS_INTERNAL pic2vtk
  PACKAGE_DEPENDS ITK VTK OpenGL
  EXPORT_DEFINE MITK_CORE_EXPORT
  WARNINGS_AS_ERRORS
)

# this is needed for libraries which link to Mitk and need
# symbols from explicitly instantiated templates like
# mitk::SurfaceVtkWriter<vtkSTLWriter> which is referenced in
# QmitkCommonFunctionality in the QmitkExt library.
if(MINGW)
  get_target_property(_mitkCore_MINGW_linkflags Mitk LINK_FLAGS)
  if(NOT _mitkCore_MINGW_linkflags)
    set(_mitkCore_MINGW_linkflags "")
  endif(NOT _mitkCore_MINGW_linkflags)
  set_target_properties(Mitk PROPERTIES LINK_FLAGS "${_mitkCore_MINGW_linkflags} -Wl,--export-all-symbols")
endif(MINGW)

# replacing Mitk by Mitk [due to removing the PROVIDES macro
TARGET_LINK_LIBRARIES(Mitk ${LIBRARIES_FOR_${KITNAME}_CORE}  ${IPFUNC_LIBRARY} ipSegmentation ann)
#TARGET_LINK_LIBRARIES(Mitk ${OPENGL_LIBRARIES} )
TARGET_LINK_LIBRARIES(Mitk gdcmCommon gdcmIOD gdcmDSED)

if(MSVC_IDE OR MSVC_VERSION OR MINGW)
  target_link_libraries(Mitk psapi.lib)
endif(MSVC_IDE OR MSVC_VERSION OR MINGW)

# build tests?
OPTION(BUILD_TESTING "Build the MITK Core tests." ON)
IF(BUILD_TESTING)
ENABLE_TESTING()
ADD_SUBDIRECTORY(Testing)
ENDIF(BUILD_TESTING)
