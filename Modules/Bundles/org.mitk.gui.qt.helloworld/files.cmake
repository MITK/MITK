SET(SRC_CPP_FILES
  QmitkHelloWorldView.cpp
  HelloWorldDialog.cpp
)
SET(MOC_H_FILES
  src/QmitkHelloWorldView.h
)

SET(INTERNAL_CPP_FILES
 
)

SET(RESOURCE_FILES
  resources/HelloWorld.png
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
