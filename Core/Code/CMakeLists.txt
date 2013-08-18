#FIND_PACKAGE(OpenGL)
#IF(NOT OPENGL_FOUND)
#  MESSAGE("GL is required for MITK rendering")
#ENDIF(NOT OPENGL_FOUND )

set(TOOL_CPPS "")

# temporary suppress warnings in the following files until image accessors are fully integrated.
set_source_files_properties( DataManagement/mitkImage.cpp COMPILE_FLAGS -DMITK_NO_DEPRECATED_WARNINGS )
set_source_files_properties( Controllers/mitkSliceNavigationController.cpp COMPILE_FLAGS -DMITK_NO_DEPRECATED_WARNINGS )

# In MITK_ITK_Config.cmake, we set ITK_NO_IO_FACTORY_REGISTER_MANAGER to 1 unless
# the variable is already defined. Setting it to 1 prevents multiple registrations/
# unregistrations of ITK IO factories during library loading/unloading (of MITK
# libraries). However, we need "one" place where the IO factories are registered at
# least once. This could be the application executable, but every executable would
# need to take care of that itself. Instead, we allow the auto registration in the
# Mitk Core library.
set(ITK_NO_IO_FACTORY_REGISTER_MANAGER 0)

MITK_CREATE_MODULE( Mitk
  INCLUDE_DIRS
    ${CppMicroServices_INCLUDE_DIRS} Algorithms Common DataManagement Controllers
    Interactions Interfaces IO Rendering IO/Interface ${MITK_BINARY_DIR}
  INTERNAL_INCLUDE_DIRS
    ${OPENGL_INCLUDE_DIR} ${IPSEGMENTATION_INCLUDE_DIR} ${ANN_INCLUDE_DIR}
    ${CppMicroServices_INTERNAL_INCLUDE_DIRS}
  DEPENDS mbilog tinyxml CppMicroServices
  DEPENDS_INTERNAL pic2vtk
  PACKAGE_DEPENDS ITK GDCM VTK OpenGL
  EXPORT_DEFINE MITK_CORE_EXPORT
  WARNINGS_AS_ERRORS
)

# this is needed for libraries which link to Mitk and need
# symbols from explicitly instantiated templates
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
