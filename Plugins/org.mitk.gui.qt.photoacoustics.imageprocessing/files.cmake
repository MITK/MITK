set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_photoacoustics_imageprocessing_Activator.cpp
  PAImageProcessing.cpp
)

set(UI_FILES
  src/internal/PAImageProcessingControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_photoacoustics_imageprocessing_Activator.h
  src/internal/PAImageProcessing.h
)

set(CACHED_RESOURCE_FILES
  resources/pai.svg
  resources/photoacousticsIcon.png
  plugin.xml
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
