SET(SRC_CPP_FILES
  #QmitkDataTreeNodeTableModel.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkDataManagerView.cpp 
)

SET(MOC_H_FILES
  src/internal/QmitkDataManagerView.h
  #src/QmitkDataTreeNodeTableModel.h  
)

SET(CPP_FILES manifest.cpp)


SET(RES_FILES
  resources/datamanager.qrc
)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
