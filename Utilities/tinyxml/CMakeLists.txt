project(TINY_XML)

set(TINY_XML_HEADERS
  tinystr.h
  tinyxml.h
)
  
set(TINY_XML_SOURCES
  tinyxml.cpp
  tinyxmlparser.cpp
  tinyxmlerror.cpp
  tinystr.cpp
)

add_definitions(-DTIXML_USE_STL)
add_library(tinyxml${LIBPOSTFIX} ${MITK_WIN32_FORCE_STATIC} ${TINY_XML_HEADERS} ${TINY_XML_SOURCES})
MITK_CREATE_MODULE_CONF( tinyxml )

