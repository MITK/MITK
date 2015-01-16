set(MOC_H_FILES
  src/internal/org_mitk_gui_common_Activator.h
)

set(SRC_CPP_FILES
  mitkDataNodeObject.cpp
  mitkDataNodeSelection.cpp
  mitkDataStorageEditorInput.cpp
  mitkILifecycleAwarePart.h
  mitkILifecycleAwarePart.cpp
  mitkILinkedRenderWindowPart.h
  mitkILinkedRenderWindowPart.cpp
  mitkIRenderingManager.cpp
  mitkIRenderWindowPart.cpp
  mitkIRenderWindowPartListener.h
  mitkIRenderWindowPartListener.cpp
  mitkIZombieViewPart.h
  mitkIZombieViewPart.cpp
  mitkWorkbenchUtil.cpp
  mitkPickedDataNodeSelectionProvider.cpp
)

set(INTERNAL_CPP_FILES
  org_mitk_gui_common_Activator.cpp
)


#set(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
