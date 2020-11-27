find_package(OpenMesh COMPONENTS ${OpenMesh_REQUIRED_COMPONENTS_BY_MODULE} REQUIRED)

foreach(openmesh_component ${OpenMesh_REQUIRED_COMPONENTS_BY_MODULE})
  list(APPEND ALL_LIBRARIES "OpenMesh${openmesh_component}")
endforeach()

set(ALL_COMPILE_DEFINITIONS -D_USE_MATH_DEFINES)
