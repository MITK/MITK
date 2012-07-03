set(QT_USE_QTSQL 1)

include_directories(${CTK_INCLUDE_DIRS})

MITK_CREATE_MODULE(mitkDicomUI
  DEPENDS QmitkExt
  PACKAGE_DEPENDS CTK
  QT_MODULE
  EXPORT_DEFINE MITK_DICOMUI_EXPORT
  )

# Mitk MitkExt to make linux happy
