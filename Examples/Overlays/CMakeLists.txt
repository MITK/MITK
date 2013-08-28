project(OverlayExample)

find_package(MITK REQUIRED)
  MITK_USE_MODULE(QmitkExt)
  MITK_USE_MODULE(MitkExt)

include_directories(${ALL_INCLUDE_DIRECTORIES})

add_executable(OverlayExample OverlayExample.cpp)
target_link_libraries(OverlayExample ${ALL_LIBRARIES})

# subproject support
set_property(TARGET OverlayExample PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
  add_dependencies(${subproject} OverlayExample)
endforeach()

