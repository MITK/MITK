set(CPP_FILES
  vtkEventQtSlotConnect.cxx
  QVTKInteractor.cxx
  QVTKInteractorAdapter.cxx
  QVTKQuickItem.cxx
  vtkQtConnection.cxx
)

set(MOC_H_FILES
  QVTKInteractorAdapter.h
  QVTKInteractorInternal.h
  QVTKQuickItem.h
  vtkQtConnection.h
)

# THIS is normally done by MITK_CREATE_MODULE.
# BUT we do not use the MITK definition of a "Qt module"
# because we need Qt 5 instead of Qt 4 (the MITK default)
qt5_wrap_cpp(moc_SRC ${MOC_H_FILES})
set(CPP_FILES ${CPP_FILES} ${moc_SRC})

set(UI_FILES
)

set(QRC_FILES
)
