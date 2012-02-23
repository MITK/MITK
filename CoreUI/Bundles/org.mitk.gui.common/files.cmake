SET(MOC_H_FILES
  src/internal/org_mitk_gui_common_Activator.h
)

SET(SRC_CPP_FILES
  mitkDataNodeObject.cpp
  mitkDataNodeSelection.cpp
  mitkDataStorageEditorInput.cpp
  mitkILifecycleAwarePart.h
  mitkILinkedRenderWindowPart.h
  mitkIRenderingManager.cpp
  mitkIRenderWindowPart.cpp
  mitkIRenderWindowPartListener.h
  mitkIZombieViewPart.h
  mitkWorkbenchUtil.cpp
)

SET(INTERNAL_CPP_FILES
  org_mitk_gui_common_Activator.cpp
)


#SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
