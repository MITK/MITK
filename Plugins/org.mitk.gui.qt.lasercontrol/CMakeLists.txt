project(org_mitk_gui_qt_lasercontrol)

IF(WIN32)
  configure_file( ${MITK_GALIL_API_PATH}/dll/x64/gclib.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
  configure_file( ${MITK_GALIL_API_PATH}/dll/x64/gclibo.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
  configure_file( ${MITK_GALIL_API_PATH}/dll/x64/gclib.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
  configure_file( ${MITK_GALIL_API_PATH}/dll/x64/gclibo.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
  configure_file( ${MITK_OPHIR_API_PATH}/Debug/OphirPyroWrapper.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
  configure_file( ${MITK_OPHIR_API_PATH}/Release/OphirPyroWrapper.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
  configure_file( ${MITK_OPHIR_API_PATH}/Debug/OphirLMMeasurement.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
  configure_file( ${MITK_OPHIR_API_PATH}/Release/OphirLMMeasurement.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
  configure_file( ${MITK_OPHIR_API_PATH}/Debug/Interop.OphirLMMeasurementLib.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
  configure_file( ${MITK_OPHIR_API_PATH}/Release/Interop.OphirLMMeasurementLib.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
ENDIF(WIN32)

mitk_create_plugin(
  EXPORT_DIRECTIVE LASERCONTROL_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkPhotoacousticsHardware
)
