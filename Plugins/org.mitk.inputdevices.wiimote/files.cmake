SET(SRC_CPP_FILES
  
)

SET(INTERNAL_CPP_FILES
  mitkWiiMoteActivator.cpp
)

SET(UI_FILES
  
)

SET(MOC_H_FILES
  
)

SET(RESOURCE_FILES
  
)

SET(RES_FILES
  
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

