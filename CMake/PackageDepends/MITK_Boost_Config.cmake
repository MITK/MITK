if(MITK_USE_Boost)

  if(NOT MITK_USE_SYSTEM_Boost)
    set(Boost_NO_SYSTEM_PATHS 1)
  endif()

  set(Boost_USE_MULTITHREADED 1)
  set(Boost_USE_STATIC_LIBS 0)
  set(Boost_USE_STATIC_RUNTIME 0)

  if(MITK_USE_Boost_LIBRARIES)
    find_package(Boost 1.40.0 REQUIRED COMPONENTS ${MITK_USE_Boost_LIBRARIES})
  else()
    find_package(Boost 1.40.0 REQUIRED)
  endif()

  list(APPEND ALL_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIRS})
  if(Boost_LIBRARIES)
    list(APPEND ALL_LIBRARIES ${Boost_LIBRARIES})
    link_directories(${Boost_LIBRARY_DIRS})
  endif()

endif(MITK_USE_Boost)

