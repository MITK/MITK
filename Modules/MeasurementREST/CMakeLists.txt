mitk_create_module(
  DEPENDS
    PUBLIC MitkRESTAPI MitkPlanarFigure MitkImageStatistics
  PACKAGE_DEPENDS
    PUBLIC Qt6|Core nlohmann_json
    PRIVATE Qt6|Gui
)

if(TARGET ${MODULE_TARGET})
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
