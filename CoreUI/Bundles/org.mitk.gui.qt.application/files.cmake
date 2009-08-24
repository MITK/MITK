SET(SRC_CPP_FILES
  QmitkApplication.cpp
  QmitkDefaultPerspective.cpp
  QmitkGeneralPreferencePage.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkActionBarAdvisor.cpp
  QmitkWorkbenchAdvisor.cpp
  QmitkWorkbenchWindowAdvisor.cpp
  QmitkPreferencesDialog.cpp
)

SET(INTERNAL_MOC_H_FILES
)

SET(MOC_H_FILES
  src/QmitkGeneralPreferencePage.h
  src/internal/QmitkWorkbenchWindowAdvisorHack.h
  src/internal/QmitkPreferencesDialog.h
)

# todo: add some qt style sheet resources
SET(RES_FILES
  resources/resources.qrc
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
