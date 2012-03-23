set(SRC_CPP_FILES
  QmitkAbstractRenderEditor.cpp
  QmitkAbstractView.cpp
  QmitkDataNodeSelectionProvider.cpp
  QmitkDnDFrameWidget.cpp
)

set(INTERNAL_CPP_FILES
  QmitkCommonActivator.cpp
  QmitkDataNodeItemModel.cpp
  QmitkDataNodeSelection.cpp
  QmitkViewCoordinator.cpp
)

set(MOC_H_FILES
  src/QmitkAbstractRenderEditor.h
  src/QmitkDnDFrameWidget.h

  src/internal/QmitkCommonActivator.h
)

set(CACHED_RESOURCE_FILES
)

set(QRC_FILES
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
