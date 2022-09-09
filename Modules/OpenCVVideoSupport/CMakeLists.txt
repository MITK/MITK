set(dependencies OpenCV|videoio+calib3d)

# adding option for videoinput library on windows (for directshow based frame grabbing)
if(WIN32 AND MITK_USE_OpenCV)
  option(MITK_USE_videoInput "Use videoInput (DirectShow wrapper) library" OFF)
endif()

if(MITK_USE_videoInput)
  list(APPEND dependencies videoInput)
endif()

mitk_create_module(
  INCLUDE_DIRS Commands
  DEPENDS MitkAlgorithmsExt
  PACKAGE_DEPENDS PUBLIC ${dependencies}
)

if(MODULE_IS_ENABLED)
  if(MITK_USE_Qt5)
    add_subdirectory(UI)
  endif()
endif()

add_subdirectory(Testing)
