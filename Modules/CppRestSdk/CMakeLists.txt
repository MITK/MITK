if(MITK_USE_CppRestSdk)

  MITK_CREATE_MODULE(
    DEPENDS MitkCore
    #WARNINGS_NO_ERRORS 
    AUTOLOAD_WITH MitkCore
  )
  
  if(TARGET ${MODULE_TARGET})
    find_package(OpenSSL REQUIRED)
	
    target_link_libraries(${MODULE_TARGET} PRIVATE cpprestsdk::cpprest)
    target_link_libraries(${MODULE_TARGET} PUBLIC OpenSSL::SSL)
  endif()
  
  add_subdirectory(test)

endif(MITK_USE_CppRestSdk)
