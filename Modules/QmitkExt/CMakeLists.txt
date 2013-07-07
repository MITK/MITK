include_directories(${CTK_INCLUDE_DIRS})

MITK_CREATE_MODULE( QmitkExt
  INCLUDE_DIRS QmitkApplicationBase QmitkPropertyObservers QmitkFunctionalityComponents
  DEPENDS MitkExt IpPicSupport Qmitk qwt qxt PlanarFigure SceneSerialization
  QT_MODULE
)

