IF(MITK_USE_Boost)

FIND_PACKAGE(Boost REQUIRED)

LIST(APPEND ALL_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
# this is only necessary if we use more than just the headers
#LIST(APPEND ALL_LIBRARIES ${Boost_LIBRARIES})
#link_directories(${Boost_LIBRARY_DIRS})

ENDIF(MITK_USE_Boost)

