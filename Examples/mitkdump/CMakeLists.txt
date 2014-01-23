project(mitkdump)
find_package(MITK)

# Check prerequisites for this application.
# We need the Mitk module.
MITK_CHECK_MODULE(result Qmitk)
if(result)
  message(SEND_ERROR "MITK module(s) \"${result}\" not available from the MITK build at ${MITK_DIR}")
endif()

# Set-up the build system to use the Mitk module
MITK_USE_MODULE(Qmitk DICOMReader)
include_directories(${ALL_INCLUDE_DIRECTORIES})
link_directories(${ALL_LIBRARY_DIRS})

add_executable(mitkdump mitkdump.cpp)
target_link_libraries(mitkdump ${ALL_LIBRARIES} )

add_executable(mitkdumpdir mitkdumpdir.cpp)
target_link_libraries(mitkdumpdir ${ALL_LIBRARIES} )

# subproject support
set_property(TARGET mitkdump PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
set_property(TARGET mitkdumpdir PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
  add_dependencies(${subproject} mitkdump)
  add_dependencies(${subproject} mitkdumpdir)
endforeach()

