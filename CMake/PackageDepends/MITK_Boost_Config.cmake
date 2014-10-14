if(MITK_USE_Boost)

  if(NOT MITK_USE_SYSTEM_Boost)
    set(Boost_NO_SYSTEM_PATHS 1)
  endif()

  set(Boost_USE_MULTITHREADED 1)
  set(Boost_USE_STATIC_LIBS 0)
  set(Boost_USE_STATIC_RUNTIME 0)

  if(MITK_USE_Boost_LIBRARIES)
    if(NOT MITK_USE_SYSTEM_Boost)
      set(BOOST_INCLUDEDIR ${CMAKE_BINARY_DIR}/../Boost-install/include)
      set(BOOST_LIBRARYDIR ${CMAKE_BINARY_DIR}/../Boost-install/lib)
      set(Boost_ADDITIONAL_VERSIONS 1.56)
    endif()
    find_package(Boost 1.56.0 REQUIRED COMPONENTS ${MITK_USE_Boost_LIBRARIES} QUIET)
  else()
    find_package(Boost 1.56.0 REQUIRED QUIET)
  endif()

  list(APPEND ALL_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
  if(Boost_LIBRARIES)
    list(APPEND ALL_LIBRARIES ${Boost_LIBRARIES})
    link_directories(${Boost_LIBRARY_DIRS})
  endif()

endif(MITK_USE_Boost)

