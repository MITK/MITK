if(MITK_USE_SOFA)
  MITK_CREATE_MODULE(
    DEPENDS MitkSceneSerializationBase
    PACKAGE_DEPENDS Boost GLUT GLEW SOFA
  )

  if(MSVC)
    list(APPEND module_compile_flags /wd4250 4251 4267 4275)
    set_target_properties(${MODULE_TARGET} PROPERTIES COMPILE_FLAGS "${module_compile_flags}")
  endif()
endif()
