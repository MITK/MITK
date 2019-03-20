mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=deprecated" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
mitkFunctionCheckCAndCXXCompilerFlags("-Wno-error=deprecated-declarations" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)

MITK_CREATE_MODULE(
  DEPENDS MitkImageStatistics MitkQtWidgets
  PACKAGE_DEPENDS
    PUBLIC Qwt CTK|CTKWidgets
    PRIVATE Qt5|Concurrent+Svg+Xml
)
