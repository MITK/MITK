MITK_CREATE_MODULE(
  INCLUDE_DIRS USControlInterfaces USFilters USModel
  INTERNAL_INCLUDE_DIRS ${INCLUDE_DIRS_INTERNAL}
  PACKAGE_DEPENDS Poco PRIVATE tinyxml2|tinyxml2
  DEPENDS MitkOpenCVVideoSupport MitkQtWidgetsExt MitkIGTBase MitkOpenIGTLink
)

if(TARGET ${MODULE_TARGET})
  target_link_libraries(${MODULE_TARGET}
    PRIVATE tinyxml2::tinyxml2
  )
endif()

## create US config
#CONFIGURE_FILE(mitkUSConfig.h.in ${PROJECT_BINARY_DIR}/mitkUSConfig.h @ONLY)

ADD_SUBDIRECTORY(USHardwareTelemed)
ADD_SUBDIRECTORY(USNavigation)

ADD_SUBDIRECTORY(Testing)
