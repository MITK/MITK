file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  QmlMitkRenderWindowItem.cpp
  QmlMitkRenderingManager.cpp
  QmlMitkRenderingManagerFactory.cpp
  QmlMitkBigRenderLock.cpp
  QmlMitkFourRenderWindowWidget.cpp
  QmlAxisOverlay.cpp
  QmlMitkStdMultiItem.cpp
  QmlMitkDatamanager.cpp
  QmlMitkProperties.cpp
)

set(MOC_H_FILES
  include/QmlMitkRenderWindowItem.h
  include/QmlMitkRenderingManager.h
  include/QmlMitkBigRenderLock.h
  include/QmlMitkFourRenderWindowWidget.h
  include/QmlMitkStdMultiItem.h
  include/QmlMitkDatamanager.h
  include/QmlMitkProperties.h
)

set(UI_FILES
)

set(QRC_FILES
  resource/MitkItems.qrc
)
