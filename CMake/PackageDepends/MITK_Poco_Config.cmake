set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${MITK_SOURCE_DIR}/BlueBerry/CMake)
find_package(Poco)

list(APPEND ALL_INCLUDE_DIRECTORIES ${Poco_INCLUDE_DIRS})

# Only add the following Poco libraries due to possible name clashes
# in PocoPDF with libpng when also linking QtGui.
foreach(lib Foundation Util XML Zip)
  list(APPEND ALL_LIBRARIES ${Poco_${lib}_LIBRARY})
endforeach()
link_directories(${Poco_LIBRARY_DIRS})

