include_directories(${CTK_INCLUDE_DIRS})

MITK_CREATE_MODULE(mitkDicomUI
  DEPENDS QmitkExt
  PACKAGE_DEPENDS CTK
  QT4_MODULES QtGui QtSql
  EXPORT_DEFINE MITK_DICOMUI_EXPORT
  )

# Mitk MitkExt to make linux happy
