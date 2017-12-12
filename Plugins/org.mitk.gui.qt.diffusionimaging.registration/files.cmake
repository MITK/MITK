set(SRC_CPP_FILES


)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp

  QmitkDiffusionRegistrationView.cpp
  QmitkSimpleRigidRegistrationView.cpp
  QmitkDwiRegistrationPerspective.cpp
)

set(UI_FILES
  src/internal/QmitkDiffusionRegistrationViewControls.ui
  src/internal/QmitkSimpleRigidRegistrationViewControls.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h

  src/internal/QmitkDiffusionRegistrationView.h
  src/internal/QmitkSimpleRigidRegistrationView.h
  src/internal/QmitkDwiRegistrationPerspective.h
)

set(CACHED_RESOURCE_FILES
  plugin.xml

  resources/diffusionregistration.png
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
