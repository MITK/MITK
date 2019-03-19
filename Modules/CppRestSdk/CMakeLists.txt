if(MITK_USE_CppRestSdk)

  MITK_CREATE_MODULE(
    DEPENDS MitkCore CppMicroServices
    #WARNINGS_NO_ERRORS 
    AUTOLOAD_WITH MitkCore
  )
  find_package(OpenSSL REQUIRED)

  target_link_libraries(${MODULE_TARGET} PRIVATE cpprestsdk::cpprest)
  target_link_libraries(${MODULE_TARGET} PUBLIC OpenSSL::SSL)
  
add_subdirectory(test)

endif(MITK_USE_CppRestSdk)
