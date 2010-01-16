SET(SRC_CPP_FILES
  berryIJobChangeListener.cpp
  berryIJobManager.cpp
  berryJob.cpp
  berryJobExceptions.cpp
  berryJobStatus.cpp
  berryMultiRule.cpp
  berryNullProgressMonitor.cpp
  berryProgressProvider.cpp
  berryQualifiedName.cpp
)

SET(INTERNAL_CPP_FILES
  berryInternalJob.cpp
  berryJobChangeEvent.cpp
  berryJobListeners.cpp
  berryJobManager.cpp
  berryJobQueue.cpp
  berryWorker.cpp
  berryWorkerPool.cpp
)

SET(H_FILES
 src/berryJobsDll.h
 src/berryLockListener.h
 src/berryISchedulingRule.h
 src/berryIProgressMonitor.h
 src/berryISchedulingRule.h
 src/berryILock.h
 )

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
