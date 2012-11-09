set(MOC_H_FILES
  src/internal/berryPluginActivator.h
)

set(SRC_CPP_FILES
  berryAbstractHandler.cpp
  berryCommand.cpp
  berryCommandCategory.cpp
  berryCommandCategoryEvent.cpp
  berryCommandEvent.cpp
  berryCommandManager.cpp
  berryCommandManagerEvent.cpp
  berryExecutionEvent.cpp
  berryHandlerEvent.cpp
  berryICommandCategoryListener.cpp
  berryICommandListener.cpp
  berryICommandManagerListener.cpp
  berryIExecutionListener.cpp
  berryIExecutionListenerWithChecks.cpp
  berryIHandlerListener.cpp
  berryINamedHandleStateIds.cpp
  berryIParameterTypeListener.cpp
  berryIStateListener.cpp
  berryNamedHandleObjectWithState.cpp
  berryParameterization.cpp
  berryParameterizedCommand.cpp
  berryParameterType.cpp
  berryParameterTypeEvent.cpp
  berryState.cpp

  util/berryCommandTracing.cpp
)

set(COMMON_CPP_FILES
  berryAbstractBitSetEvent.cpp
  berryAbstractHandleObjectEvent.cpp
  berryAbstractNamedHandleEvent.cpp
  berryCommandExceptions.cpp
  berryHandleObject.cpp
  berryHandleObjectManager.cpp
  berryNamedHandleObject.cpp
)

set(INTERNAL_CPP_FILES
  berryCommandUtils.cpp
  berryPluginActivator.cpp
)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${COMMON_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/common/${file})
endforeach(file ${COMMON_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
