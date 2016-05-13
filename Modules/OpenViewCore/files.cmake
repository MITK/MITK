file(GLOB_RECURSE H_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/include/*")

set(CPP_FILES
  vtkEventQtSlotConnect.cxx
  vtkQtConnection.cxx
  QVTKInteractorAdapter.cxx
  QVTKQuickItem.cxx
  QVTKFramebufferObjectRenderer.cxx
  QVTKInternalOpenglRenderWindow.cxx
)

set(MOC_H_FILES
  include/QVTKInteractorAdapter.h
  include/QVTKQuickItem.h
  include/vtkQtConnection.h
)

set(UI_FILES
)

set(QRC_FILES
)
