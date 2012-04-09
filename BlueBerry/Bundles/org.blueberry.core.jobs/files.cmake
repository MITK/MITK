set(MOC_H_FILES
  src/internal/berryPluginActivator.h
)

set(SRC_CPP_FILES
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

set(INTERNAL_CPP_FILES
  berryInternalJob.cpp
  berryJobChangeEvent.cpp
  berryJobListeners.cpp
  berryJobManager.cpp
  berryJobQueue.cpp
  berryPluginActivator.cpp
  berryWorker.cpp
  berryWorkerPool.cpp
)

set(H_FILES
 src/berryJobsDll.h
 src/berryLockListener.h
 src/berryISchedulingRule.h
 src/berryIProgressMonitor.h
 src/berryISchedulingRule.h
 src/berryILock.h
 )

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
