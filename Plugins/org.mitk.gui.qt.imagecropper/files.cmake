set(SRC_CPP_FILES

)

set(INTERNAL_CPP_FILES
  org_mitk_gui_qt_imagecropper_Activator.cpp
  QmitkImageCropper.cpp
)

set(UI_FILES
  src/internal/ImageCropperControls.ui
)

set(MOC_H_FILES
  src/internal/org_mitk_gui_qt_imagecropper_Activator.h
  src/internal/QmitkImageCropper.h
)

set(CACHED_RESOURCE_FILES
  resources/crop.svg
  plugin.xml
)

set(QRC_FILES
  resources/imagecropper.qrc
)

set(CPP_FILES)

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
