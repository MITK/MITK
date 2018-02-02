SET (CHART_QT_MINIMUM_VERSION 5.10.0)
IF (Qt5WebEngineWidgets_VERSION VERSION_LESS CHART_QT_MINIMUM_VERSION)
  MESSAGE(AUTHOR_WARNING "MitkChart module requires at least ${CHART_QT_MINIMUM_VERSION}. Install this Qt version or above if you want to use MitkChart with full functionality.")
ENDIF()

MITK_CREATE_MODULE(
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC Qt5|WebEngineWidgets
)

add_subdirectory(test)