project(org_mitk_gui_qt_ultrasound)

include_directories(${CTK_INCLUDE_DIRS})

IF(WIN32)
  IF(MITK_USE_US_DiPhAS_SDK)
    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/Framework.IBMT.US.Core.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/Framework.IBMT.US.CWrapper.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/Framework.IBMT.US.OpenCL.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/Framework.IBMT.US.PCIeInterface.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )
    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/openclext.dll ${MITK_BINARY_DIR}/bin/Debug/ COPYONLY )

    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/Framework.IBMT.US.Core.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/Framework.IBMT.US.CWrapper.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/Framework.IBMT.US.OpenCL.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/Framework.IBMT.US.PCIeInterface.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
    configure_file( ${MITK_US_DiPhAS_SDK_PATH}/openclext.dll ${MITK_BINARY_DIR}/bin/Release/ COPYONLY )
  ENDIF(MITK_USE_US_DiPhAS_SDK)
ENDIF(WIN32)

mitk_create_plugin(
  EXPORT_DIRECTIVE ULTRASOUND_EXPORT
  EXPORTED_INCLUDE_SUFFIXES src
  MODULE_DEPENDS MitkQtWidgetsExt MitkUS MitkUSUI
)
