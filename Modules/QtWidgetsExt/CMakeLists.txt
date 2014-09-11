MITK_CREATE_MODULE(
  INCLUDE_DIRS QmitkApplicationBase QmitkPropertyObservers QmitkFunctionalityComponents
  DEPENDS MitkImageStatistics MitkQtWidgets
  PACKAGE_DEPENDS Qt4|QtWebKit Qwt Qxt
  WARNINGS_AS_ERRORS
)
