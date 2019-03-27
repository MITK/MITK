if(MITK_USE_CppRestSdk)

  MITK_CREATE_MODULE(
    DEPENDS MitkCore MitkCppRestSdk
    PACKAGE_DEPENDS Qt5|Core
    AUTOLOAD_WITH MitkQtWidgets
  )

  if(TARGET ${MODULE_TARGET})
    target_link_libraries(${MODULE_TARGET} PUBLIC cpprestsdk::cpprest OpenSSL::SSL)
  endif()

endif()
