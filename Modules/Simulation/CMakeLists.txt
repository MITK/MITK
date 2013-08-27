if(MITK_USE_SOFA)
  MITK_CREATE_MODULE(Simulation
    DEPENDS Mitk
    PACKAGE_DEPENDS Boost SOFA
  )

  if(MSVC)
    set(module_compile_flags "${module_compile_flags} /wd4250 /wd4251 /wd4267 /wd4275")
    set_target_properties(${MODULE_PROVIDES} PROPERTIES COMPILE_FLAGS "${module_compile_flags}")
  endif()

  add_subdirectory(Testing)
endif()
