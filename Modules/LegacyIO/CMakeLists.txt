mitkFunctionCheckCompilerFlags("/wd4996" CMAKE_CXX_FLAGS)
mitkFunctionCheckCompilerFlags("-Wno-deprecated-declarations" CMAKE_CXX_FLAGS)

MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS PRIVATE ITK|ITKIOGDCM+ITKIORAW tinyxml2|tinyxml2
  DEPRECATED_SINCE 2014.10
)

if(TARGET ${MODULE_TARGET})
  target_link_libraries(${MODULE_TARGET}
    PRIVATE tinyxml2::tinyxml2
  )
endif()
