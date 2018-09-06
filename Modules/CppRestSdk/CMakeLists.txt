if(MITK_USE_CppRestSdk)

  MITK_CREATE_MODULE(
    DEPENDS MitkCore
  )

  target_link_libraries(${MODULE_TARGET} PRIVATE cpprestsdk|cpprest)

endif(MITK_USE_CppRestSdk)