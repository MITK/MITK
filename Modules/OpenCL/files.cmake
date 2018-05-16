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
  mitkOclDataSet.cpp
  mitkOclFilter.cpp
  mitkOclImageFilter.cpp
  mitkOclDataSetFilter.cpp
  mitkOclImageToImageFilter.cpp
  mitkOclDataSetToDataSetFilter.cpp

# own filter implementations
  mitkOclBinaryThresholdImageFilter.cpp
)

set(RESOURCE_FILES
  BinaryThresholdFilter.cl
)
