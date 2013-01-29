set(CPP_FILES
# helper classes
  mitkOclUtils.cpp
  mitkOclResourceServiceImpl_Private.cpp
  mitkOclImageFormats.cpp

# module activator
  mitkOpenCLActivator.cpp

# base data and filter objects
  mitkOclBaseData.cpp
  mitkOclImage.cpp
  mitkOclFilter.cpp
  mitkOclImageFilter.cpp
  mitkOclImageToImageFilter.cpp

# own filter implementations
  mitkOclBinaryThresholdImageFilter.cpp
)
