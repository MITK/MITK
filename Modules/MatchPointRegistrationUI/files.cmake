file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  Qmitk/QmitkRegistrationJob.cpp
  Qmitk/QmitkMappingJob.cpp
  Qmitk/QmitkFramesRegistrationJob.cpp
  Qmitk/QmitkAlgorithmProfileViewer.cpp
  Qmitk/QmitkAlgorithmSettingsConfig.cpp
  Qmitk/QmitkMAPAlgorithmModel.cpp
  Qmitk/QmitkAlgorithmListModel.cpp
  Qmitk/QmitkMapPropertyDelegate.cpp
  Qmitk/QmitkMapperSettingsWidget.cpp
  Qmitk/QmitkRegEvalSettingsWidget.cpp
  Qmitk/QmitkRegistrationManipulationWidget.cpp
)

set(UI_FILES
  src/Qmitk/QmitkAlgorithmProfileViewer.ui
  src/Qmitk/QmitkAlgorithmSettingsConfig.ui
  src/Qmitk/QmitkMapperSettingsWidget.ui
  src/Qmitk/QmitkRegEvalSettingsWidget.ui
  src/Qmitk/QmitkRegistrationManipulationWidget.ui
)
