SET(SRC_CPP_FILES
  cherryAbstractHandler.cpp
  cherryCommand.cpp
  cherryCommandCategory.cpp
  cherryCommandCategoryEvent.cpp
  cherryCommandEvent.cpp
  cherryCommandManager.cpp
  cherryCommandManagerEvent.cpp
  cherryExecutionEvent.cpp
  cherryHandlerEvent.cpp
  cherryICommandCategoryListener.cpp
  cherryICommandListener.cpp
  cherryICommandManagerListener.cpp
  cherryIExecutionListener.cpp
  cherryIExecutionListenerWithChecks.cpp
  cherryIHandlerListener.cpp
  cherryINamedHandleStateIds.cpp
  cherryIParameterTypeListener.cpp
  cherryIStateListener.cpp
  cherryNamedHandleObjectWithState.cpp
  cherryParameterization.cpp
  cherryParameterizedCommand.cpp
  cherryParameterType.cpp
  cherryParameterTypeEvent.cpp
  cherryState.cpp
  
  util/cherryCommandTracing.cpp
)

SET(COMMON_CPP_FILES
  cherryAbstractBitSetEvent.cpp
  cherryAbstractHandleObjectEvent.cpp
  cherryAbstractNamedHandleEvent.cpp
  cherryCommandExceptions.cpp
  cherryHandleObject.cpp
  cherryHandleObjectManager.cpp
  cherryNamedHandleObject.cpp
)

SET(INTERNAL_CPP_FILES
  cherryCommandUtils.cpp
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${COMMON_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/common/${file})
endforeach(file ${COMMON_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})