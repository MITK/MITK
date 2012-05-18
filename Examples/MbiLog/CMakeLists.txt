project(MbiLogExample)

find_package(MITK REQUIRED)
MITK_USE_MODULE(Mitk)

include_directories(${ALL_INCLUDE_DIRECTORIES})

add_executable(MitkLogExample MitkLogExample.cpp)
target_link_libraries(MitkLogExample ${ALL_LIBRARIES})

# subproject support
set_property(TARGET MitkLogExample PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
  add_dependencies(${subproject} MitkLogExample)
endforeach()

