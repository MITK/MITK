set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  QmitkIGTTrackingLabView.cpp
  QmitkIGTTutorialView.cpp
  OpenIGTLinkExample.cpp
  OpenIGTLinkProviderExample.cpp
  OpenIGTLinkPlugin.cpp
  mitkPluginActivator.cpp
)

set(UI_FILES
  src/internal/QmitkIGTTrackingLabViewControls.ui
  src/internal/QmitkIGTTutorialViewControls.ui
  src/internal/OpenIGTLinkProviderExampleControls.ui
  src/internal/OpenIGTLinkExampleControls.ui
  src/internal/OpenIGTLinkPluginControls.ui
)

set(MOC_H_FILES
  src/internal/QmitkIGTTrackingLabView.h
  src/internal/QmitkIGTTutorialView.h
  src/internal/OpenIGTLinkProviderExample.h
  src/internal/OpenIGTLinkExample.h
  src/internal/mitkPluginActivator.h
  src/internal/OpenIGTLinkPlugin.h
)

set(CACHED_RESOURCE_FILES
  resources/start_rec.png
  resources/stop_rec.png
  resources/icon_igt_simple_example.svg
  resources/icon_igt_tracking_lab.svg
  resources/icon_openigtlink_client_example.svg
  resources/icon_openigtlink_extended_example.svg
  resources/icon_openigtlink_provider_example.svg
  plugin.xml
)

set(CPP_FILES

)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
