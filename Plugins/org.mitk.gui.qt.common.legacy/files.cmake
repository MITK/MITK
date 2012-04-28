SET(SRC_CPP_FILES
  QmitkFunctionality.cpp
  QmitkFunctionality2.cpp
  QmitkFunctionalityCoordinator.cpp
)

SET(INTERNAL_CPP_FILES
  QmitkCommonLegacyActivator.cpp
  QmitkFunctionalityUtil.cpp
)

SET(MOC_H_FILES
  src/internal/QmitkCommonLegacyActivator.h
)

SET(UI_FILES
)

SET(CACHED_RESOURCE_FILES
)

SET(QRC_FILES
)

SET(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
