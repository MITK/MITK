include_directories(${CTK_INCLUDE_DIRS})

MITK_CREATE_MODULE( QmitkExt
  INCLUDE_DIRS QmitkApplicationBase QmitkPropertyObservers QmitkFunctionalityComponents
  DEPENDS MitkExt IpPicSupport Qmitk PlanarFigure SceneSerialization
  PACKAGE_DEPENDS Qwt Qxt
  QT4_MODULES QtCore QtGui
)
