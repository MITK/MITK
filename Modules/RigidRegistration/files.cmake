SET(CPP_FILES
  mitkImageRegistrationMethod.cpp
  mitkMetricParameters.cpp
  mitkOptimizerFactory.cpp
  mitkOptimizerParameters.cpp
  mitkRigidRegistrationObserver.cpp
  mitkRigidRegistrationPreset.cpp
  mitkRigidRegistrationTestPreset.cpp
  mitkTransformParameters.cpp
  mitkPyramidalRegistrationMethod.cpp
)

MITK_MULTIPLEX_PICTYPE( mitkImageRegistrationMethod-TYPE.cpp ) 
MITK_MULTIPLEX_PICTYPE( mitkPyramidalRegistrationMethod-TYPE.cpp ) 