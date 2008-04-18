SET(CPP_FILES
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

foreach(file ${COMMON_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} common/${file})
endforeach(file ${COMMON_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})