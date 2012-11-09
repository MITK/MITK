set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${MITK_SOURCE_DIR}/BlueBerry/CMake)
find_package(Poco)
list(APPEND ALL_INCLUDE_DIRECTORIES ${Poco_INCLUDE_DIRS})
list(APPEND ALL_LIBRARIES optimized PocoFoundation
                          debug PocoFoundationd
                          optimized PocoUtil
                          debug PocoUtild
                          optimized PocoXML
                          debug PocoXMLd)
link_directories(${Poco_LIBRARY_DIRS})


