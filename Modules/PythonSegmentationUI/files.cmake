set(CPP_FILES
  mitkPythonPackageUpdatePrompt.cpp
  mitkPythonSegmentationUI.cpp
  mitknnInteractiveInstall.cpp
  mitknnInteractiveModel.cpp
  mitkTorchInstall.cpp
  mitkTotalSegmentatorInstall.cpp
  mitkVoxTellInstall.cpp
  QmitknnInteractiveInstallModeDialog.cpp
  QmitknnInteractiveToolGUI.cpp
  QmitkTotalSegmentatorToolGUI.cpp
  QmitkVenvProcess.cpp
  QmitkVoxTellToolGUI.cpp
)

set(UI_FILES
  QmitknnInteractiveInstallModeDialog.ui
  QmitknnInteractiveToolGUI.ui
  QmitkTotalSegmentatorToolGUI.ui
  QmitkVoxTellToolGUI.ui
)

set(QRC_FILES
  HuggingFace/HuggingFace.qrc
  nnInteractive/nnInteractive.qrc
)
