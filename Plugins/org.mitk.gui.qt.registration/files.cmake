set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  mitkPluginActivator.cpp
  #DeformableRegistration
  QmitkDeformableRegistrationView.cpp
  # PointBasedRegistration
  QmitkPointBasedRegistrationView.cpp
  mitkLandmarkWarping.cpp
  # RigidRegistration
  QmitkRigidRegistrationView.cpp
  QmitkLoadPresetDialog.cpp
  QmitkRigidRegistrationSelectorView.cpp

)

set(UI_FILES
  #DeformableRegistration
  src/internal/QmitkDeformableRegistrationViewControls.ui
  # PointBasedRegistration
  src/internal/QmitkPointBasedRegistrationViewControls.ui
  # RigidRegistration
  src/internal/QmitkRigidRegistrationViewControls.ui
  src/internal/QmitkRigidRegistrationSelector.ui
)

set(MOC_H_FILES
  src/internal/mitkPluginActivator.h
  #DeformableRegistration
  src/internal/QmitkDeformableRegistrationView.h
  # PointBasedRegistration
  src/internal/QmitkPointBasedRegistrationView.h
  # RigidRegistration
  src/internal/QmitkRigidRegistrationView.h
  src/internal/QmitkLoadPresetDialog.h
  src/internal/QmitkRigidRegistrationSelectorView.h

)

set(CACHED_RESOURCE_FILES
# list of resource files which can be used by the plug-in
# system without loading the plug-ins shared library,
# for example the icon used in the menu and tabs for the
# plug-in views in the workbench
  plugin.xml
  #DeformableRegistration
  resources/DeformableRegistration.xpm
  # PointBasedRegistration
  resources/PointBasedRegistration.xpm
  # RigidRegistration
  resources/RigidRegistration.xpm
)

set(QRC_FILES
  #DeformableRegistration
  resources/QmitkDeformableRegistrationView.qrc
  # PointBasedRegistration
  resources/QmitkPointBasedRegistrationView.qrc
  # RigidRegistration
  resources/QmitkRigidRegistrationView.qrc
)

set(CPP_FILES

)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

