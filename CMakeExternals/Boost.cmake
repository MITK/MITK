#-----------------------------------------------------------------------------
# Boost
#-----------------------------------------------------------------------------

IF(MITK_USE_Boost)

  # Sanity checks
  IF(DEFINED BOOST_ROOT AND NOT EXISTS ${BOOST_ROOT})
    MESSAGE(FATAL_ERROR "BOOST_ROOT variable is defined but corresponds to non-existing directory")
  ENDIF()

  SET(proj Boost)
  SET(proj_DEPENDENCIES )
  SET(Boost_DEPENDS ${proj})

  IF(NOT DEFINED BOOST_ROOT)
    ExternalProject_Add(${proj}
      URL http://mitk.org/download/thirdparty/boost_1_45_0.tar.bz2
      SOURCE_DIR ${proj}-src
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ""
      DEPENDS ${proj_DEPENDENCIES}
      )
    SET(BOOST_ROOT ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src)

  ELSE()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
    
  ENDIF()
  
ENDIF()
