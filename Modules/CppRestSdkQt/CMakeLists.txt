if(MITK_USE_CppRestSdk)

  MITK_CREATE_MODULE(
    DEPENDS MitkCore MitkQtWidgetsExt CppMicroServices MitkCppRestSdk
    PACKAGE_DEPENDS Qt5|Core
    #WARNINGS_NO_ERRORS 
	AUTOLOAD_WITH MitkQtWidgets
  )
  find_package(OpenSSL REQUIRED)

  target_link_libraries(${MODULE_TARGET} PRIVATE cpprestsdk::cpprest)
  target_link_libraries(${MODULE_TARGET} PUBLIC OpenSSL::SSL)
  

endif()
