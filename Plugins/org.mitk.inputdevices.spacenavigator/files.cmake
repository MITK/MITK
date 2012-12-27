set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
 mitkSpaceNavigatorActivator.cpp

)

set(UI_FILES

)

set(MOC_H_FILES

)

set(RESOURCE_FILES

)

set(RES_FILES

)

set(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

