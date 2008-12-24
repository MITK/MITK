SET(SRC_CPP_FILES
  cherryAbstractHandler.cpp
  cherryExecutionEvent.cpp
  cherryState.cpp
)

SET(COMMON_CPP_FILES
  cherryCommandExceptions.cpp
  cherryHandleObject.cpp
  cherryNamedHandleObject.cpp
)

SET(INTERNAL_CPP_FILES
  
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