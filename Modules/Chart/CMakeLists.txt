SET (CHART_QT_MINIMUM_VERSION 5.9.0)
IF (Qt5WebEngineWidgets_VERSION LESS CHART_QT_MINIMUM_VERSION)
  MESSAGE(WARNING "MitkChart module requires at least ${CHART_QT_MINIMUM_VERSION}. Install this Qt version or above if you want to use MitkChart.")
ELSE()
  MITK_CREATE_MODULE(
    DEPENDS MitkCore
    PACKAGE_DEPENDS PRIVATE Qt5|WebEngineWidgets
  )
ENDIF()
