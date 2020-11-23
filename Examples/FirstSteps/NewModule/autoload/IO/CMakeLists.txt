MITK_CREATE_MODULE( NewModuleIO
  INCLUDE_DIRS
    PRIVATE src/IO
  DEPENDS PUBLIC MitkNewModule MitkSceneSerialization
  PACKAGE_DEPENDS
    PRIVATE tinyxml2|tinyxml2
  AUTOLOAD_WITH MitkCore
)

if(TARGET ${MODULE_TARGET})
  target_link_libraries(${MODULE_TARGET}
    PRIVATE tinyxml2::tinyxml2
  )
endif()
