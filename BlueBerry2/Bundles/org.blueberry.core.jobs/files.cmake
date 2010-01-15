SET(SRC_CPP_FILES
  cherryIJobChangeListener.cpp
  cherryIJobManager.cpp
  cherryJob.cpp
  cherryJobExceptions.cpp
  cherryJobStatus.cpp
  cherryMultiRule.cpp
  cherryNullProgressMonitor.cpp
  cherryProgressProvider.cpp
  cherryQualifiedName.cpp
)

SET(INTERNAL_CPP_FILES
  cherryInternalJob.cpp
  cherryJobChangeEvent.cpp
  cherryJobListeners.cpp
  cherryJobManager.cpp
  cherryJobQueue.cpp
  cherryWorker.cpp
  cherryWorkerPool.cpp
)

SET(H_FILES
 src/cherryJobsDll.h
 src/cherryLockListener.h
 src/cherryISchedulingRule.h
 src/cherryIProgressMonitor.h
 src/cherryISchedulingRule.h
 src/cherryILock.h
 )

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
