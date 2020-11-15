mitk_create_module(
  DEPENDS MitkCore
  PACKAGE_DEPENDS OpenMesh|OpenMeshTools
)

if(TARGET ${MODULE_TARGET})
  target_link_libraries(${MODULE_TARGET} PRIVATE OpenMeshTools)
  target_compile_definitions(${MODULE_TARGET} PRIVATE -D_USE_MATH_DEFINES)
endif()
