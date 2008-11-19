SET(CPP_FILES
  mitkBaseController.cpp
  mitkCameraController.cpp
  mitkCameraRotationController.cpp
  mitkFocusManager.cpp
  mitkLimitedLinearUndo.cpp
  mitkMovieGenerator.cpp
  mitkMultiStepper.cpp
  mitkOperationEvent.cpp
  mitkProgressBar.cpp
  mitkRenderingManager.cpp
  mitkSegmentationInterpolationController.cpp
  mitkNavigationController.cpp
  mitkPointNavigationController.cpp
  mitkSliceNavigationController.cpp
  mitkSlicesCoordinator.cpp
  mitkSlicesRotator.cpp
  mitkSlicesSwiveller.cpp
  mitkStatusBar.cpp
  mitkStepper.cpp
  mitkTestManager.cpp
  mitkToolManager.cpp
  mitkUndoController.cpp
  mitkVerboseLimitedLinearUndo.cpp
  mitkVtkInteractorCameraController.cpp
  mitkVtkLayerController.cpp
  # VtkQRenderWindowInteractor.cpp
)
IF(WIN32)
  SET(CPP_FILES ${CPP_FILES} mitkMovieGeneratorWin32.cpp)
ENDIF(WIN32)

IF(MITK_USE_TD_MOUSE)
  SET(CPP_FILES ${CPP_FILES}  mitkTDMouseVtkCameraController.cpp)
ENDIF(MITK_USE_TD_MOUSE)
