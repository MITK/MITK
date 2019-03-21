if(MITK_USE_CppRestSdk)

  MITK_CREATE_MODULE(
    DEPENDS MitkCore MitkCppRestSdk
    PACKAGE_DEPENDS Qt5|Core
    #WARNINGS_NO_ERRORS 
	AUTOLOAD_WITH MitkQtWidgets
  )
  
  if(TARGET ${MODULE_TARGET})
    find_package(OpenSSL REQUIRED)
	
    target_link_libraries(${MODULE_TARGET} PRIVATE cpprestsdk::cpprest)
    target_link_libraries(${MODULE_TARGET} PUBLIC OpenSSL::SSL)
  endif()
  
endif()
