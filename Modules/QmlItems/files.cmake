file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  QmlMitkRenderWindowItem.cpp
  QmlMitkRenderingManager.cpp
  QmlMitkRenderingManagerFactory.cpp
  QmlMitkBigRenderLock.cpp
  QmlAxisOverlay.cpp
  QmlMitkStdMultiItem.cpp
  QmlMitkDatamanager.cpp
  QmlMitkProperties.cpp
  QmlMitkSliderLevelWindowItem.cpp
  QmlMitkTransferFunctionCanvas.cpp
  QmlMitkPiecewiseFunctionCanvas.cpp
  QmlMitkColorTransferFunctionCanvas.cpp
  QmlMitkTransferFunctionItem.cpp
  QmlMitkImageNavigator.cpp
  QmlMitkSliderNavigatorItem.cpp
)

set(MOC_H_FILES
  include/QmlMitkRenderWindowItem.h
  include/QmlMitkRenderingManager.h
  include/QmlMitkBigRenderLock.h
  include/QmlMitkStdMultiItem.h
  include/QmlMitkDatamanager.h
  include/QmlMitkProperties.h
  include/QmlMitkSliderLevelWindowItem.h
  include/QmlMitkTransferFunctionCanvas.h
  include/QmlMitkPiecewiseFunctionCanvas.h
  include/QmlMitkColorTransferFunctionCanvas.h
  include/QmlMitkTransferFunctionItem.h
  include/QmlMitkImageNavigator.h
  include/QmlMitkSliderNavigatorItem.h
)

set(UI_FILES
)

set(QRC_FILES
  resource/MitkItems.qrc
)
