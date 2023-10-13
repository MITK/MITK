option(BUILD_SegmentationCmdApps "Build command-line apps of the MitkSegmentation module" OFF)

if(BUILD_SegmentationCmdApps OR MITK_BUILD_ALL_APPS)
  mitkFunctionCreateCommandLineApp(
    NAME ContoursToImage
    DEPENDS MitkSegmentation
  )
endif()
