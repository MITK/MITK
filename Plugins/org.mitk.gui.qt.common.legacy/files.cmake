set(SRC_CPP_FILES
  QmitkFunctionality.cpp
  QmitkFunctionality2.cpp
  QmitkFunctionalityCoordinator.cpp
)

set(INTERNAL_CPP_FILES
  QmitkCommonLegacyActivator.cpp
  QmitkFunctionalityUtil.cpp
)

set(MOC_H_FILES
  src/internal/QmitkCommonLegacyActivator.h
)

set(UI_FILES
)

set(CACHED_RESOURCE_FILES
)

set(QRC_FILES
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
