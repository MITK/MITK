#MESSAGE(STATUS "Searching for OpenCV")
FIND_PACKAGE(OpenCV REQUIRED)
LIST(APPEND ALL_LIBRARIES ${OpenCV_LIBS})
# adding release directory for fixing error with RelWitgDebInfo (debug dlls have the suffix "d". thus release dlls wont be loaded accidentally)
LINK_DIRECTORIES(${OpenCV_LIB_DIR} "${OpenCV_LIB_DIR}/Release")

#find_path(OPENCV_DIR OpenCVConfig.cmake HINTS $ENV{PATH} $ENV{OPENCV_DIR} $ENV{OPENCVDIR} $ENV{OpenCVDIR} $ENV{OPENCV_DIR} $ENV{OpenCV_DIR})

#IF(OPENCV_DIR)
#INCLUDE("${OPENCV_DIR}/OpenCVConfig.cmake")
#LOAD_CACHE(${THIS_OPENCV_CONFIG_PATH} READ_WITH_PREFIX OpenCV_ OpenCV_SOURCE_DIR)
#SET(OpenCV_INCLUDE_DIRS "${OpenCV_OpenCV_SOURCE_DIR}/include" "${OpenCV_OpenCV_SOURCE_DIR}/include/opencv" "${OpenCV_OpenCV_SOURCE_DIR}/3rdparty/include")
	
#LIST(APPEND ALL_INCLUDE_DIRECTORIES ${OpenCV_INCLUDE_DIRS})
#LIST(APPEND ALL_LIBRARIES ${OpenCV_LIBS})
# adding release directory for fixing error with RelWitgDebInfo
#link_directories(${OpenCV_LIB_DIR} "${OpenCV_LIB_DIR}/Release")
#ELSE(OPENCV_DIR)
#MESSAGE(STATUS "Warning: OpenCVConfig.cmake not found. Please set OPENCV_DIR manually to guide search.")
#SET(OPENCV_DIR CACHE PATH "Set the directory containing OpenCVConfig.cmake")
#ENDIF(OPENCV_DIR)
