MITK_CREATE_MODULE(
  DEPENDS MitkLegacyGL MitkSceneSerializationBase
  PACKAGE_DEPENDS
    PUBLIC SOFA
    PRIVATE Boost GLUT GLEW
)

if(TARGET ${MODULE_TARGET})
  target_compile_options(${MODULE_TARGET} PRIVATE "$<$<CXX_COMPILER_ID:MSVC>:/wd4068 /wd4250 /wd4251 /wd4267 /wd4275>")
endif()
