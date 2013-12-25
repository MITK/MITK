include_directories(${CTK_INCLUDE_DIRS})

MITK_CREATE_MODULE( QmitkExt
  INCLUDE_DIRS QmitkApplicationBase QmitkPropertyObservers QmitkFunctionalityComponents
  DEPENDS MitkAlgorithmsExt Qmitk
  PACKAGE_DEPENDS Qwt Qxt
  QT_MODULE
)

