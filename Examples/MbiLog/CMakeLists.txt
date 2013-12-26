project(MbiLogExample)

find_package(MITK REQUIRED)

add_executable(MitkLogExample MitkLogExample.cpp)
mitk_use_modules(TARGET MitkLogExample MODULES Mitk)

# subproject support
set_property(TARGET MitkLogExample PROPERTY LABELS ${MITK_DEFAULT_SUBPROJECTS})
foreach(subproject ${MITK_DEFAULT_SUBPROJECTS})
  add_dependencies(${subproject} MitkLogExample)
endforeach()

