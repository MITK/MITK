if(MITK_USE_CppRestSdk)

  MITK_CREATE_MODULE(
    DEPENDS MitkCore MitkQtWidgetsExt
    PACKAGE_DEPENDS Qt5|Core
  )

endif(MITK_USE_CppRestSdk)
