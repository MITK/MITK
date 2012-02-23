SET(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  src/internal/mitkDataStorageService.h
)

SET(SRC_CPP_FILES
  mitkIDataStorageService.cpp
)

SET(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  mitkDataStorageReference.cpp
  mitkDataStorageService.cpp
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
