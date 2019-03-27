if(MITK_USE_CppRestSdk)

  MITK_CREATE_MODULE(
    DEPENDS MitkCore
    AUTOLOAD_WITH MitkCore
  )

  if(TARGET ${MODULE_TARGET})
    target_link_libraries(${MODULE_TARGET} PUBLIC cpprestsdk::cpprest OpenSSL::SSL)
  endif()

  add_subdirectory(test)

endif(MITK_USE_CppRestSdk)
