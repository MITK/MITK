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

  IF(NOT DEFINED BOOST_ROOT AND NOT MITK_USE_SYSTEM_Boost)
  
    SET(_boost_libs )
    
    IF(MITK_USE_Boost_LIBRARIES)
      # We need binary boost libraries
      FOREACH(_boost_lib ${MITK_USE_Boost_LIBRARIES})
        SET(_boost_libs ${_boost_libs} --with-${_boost_lib})
      ENDFOREACH()

      IF(WIN32)
        SET(_boost_variant "")
        SET(_shell_extension .bat)
      ELSE()
        IF(CMAKE_BUILD_TYPE STREQUAL "Debug")
          SET(_boost_variant "variant=debug")
        ELSE()
          SET(_boost_variant "variant=release")
        ENDIF()
        SET(_shell_extension .sh)
      ENDIF()  
      
      SET(_boost_cfg_cmd ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src/bootstrap${_shell_extension})
      SET(_boost_build_cmd ${CMAKE_CURRENT_BINARY_DIR}/${proj}-src/bjam --build-dir=${CMAKE_CURRENT_BINARY_DIR}/${proj}-build ${_boost_variant} ${_boost_libs} link=shared,static threading=multi runtime-link=shared -q)
    ELSE()
      SET(_boost_cfg_cmd )
      SET(_boost_build_cmd )
    ENDIF()
    
    ExternalProject_Add(${proj}
      URL http://mitk.org/download/thirdparty/boost_1_45_0.tar.bz2
      SOURCE_DIR ${proj}-src
      BINARY_DIR ${proj}-src
      CONFIGURE_COMMAND "${_boost_cfg_cmd}"
      BUILD_COMMAND "${_boost_build_cmd}"
      INSTALL_COMMAND ""
      DEPENDS ${proj_DEPENDENCIES}
      )
      
    SET(BOOST_ROOT "${CMAKE_CURRENT_BINARY_DIR}/${proj}-src")
    
    SET(BOOST_INCLUDEDIR "${CMAKE_CURRENT_BINARY_DIR}/${proj}-src")
    SET(BOOST_LIBRARYDIR "${CMAKE_CURRENT_BINARY_DIR}/${proj}-src/stage/lib")

  ELSE()

    mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
    
  ENDIF()
  
ENDIF()
