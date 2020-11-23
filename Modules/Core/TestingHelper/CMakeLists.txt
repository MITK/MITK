mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS
    PUBLIC CppUnit tinyxml2|tinyxml2
    PRIVATE VTK|IOImage+TestingRendering OpenGL
)

if(TARGET ${MODULE_TARGET})
  target_link_libraries(${MODULE_TARGET}
    PUBLIC tinyxml2::tinyxml2
  )
endif()
