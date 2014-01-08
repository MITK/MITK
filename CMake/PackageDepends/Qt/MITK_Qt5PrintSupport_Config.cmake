find_package(Qt5PrintSupport ${MITK_QT5_MINIMUM_VERSION} REQUIRED)
qt5_use_modules(${MODULE_NAME} PrintSupport)
