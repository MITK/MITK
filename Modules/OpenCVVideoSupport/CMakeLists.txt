set(dependencies OpenCV)
if(MITK_USE_videoInput)
  set(dependencies ${dependencies} videoInput)
endif(MITK_USE_videoInput)

mitk_create_module(
  INCLUDE_DIRS Commands
  DEPENDS MitkAlgorithmsExt
  PACKAGE_DEPENDS PUBLIC ${dependencies}
  ADDITIONAL_LIBS ${OPENCVVIDEOSUPPORT_ADDITIONAL_LIBS}
)

if(MODULE_IS_ENABLED)
  if(MITK_USE_Qt5)
    add_subdirectory(UI)
  endif()
endif()

add_subdirectory(Testing)
