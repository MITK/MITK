SET(QT_USE_QT3SUPPORT 1)

MITK_GENERATE_TOOLS_LIBRARY(QmitkExt "NO")
MITK_CREATE_MODULE( QmitkExt
  INCLUDE_DIRS QmitkApplicationBase QmitkPropertyObservers QmitkFunctionalityComponents vtkQtChartHeaders RigidRegistrationMetrics RigidRegistrationOptimizer RigidRegistrationTransforms
  DEPENDS MitkExt Qmitk qwt qxt SceneSerialization
  QT_MODULE 
  GENERATED_CPP ${TOOL_GUI_CPPS} ${TOOL_CPPS}
)

