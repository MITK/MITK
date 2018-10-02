#!
#! Create a Command Line App.
#!
#! \brief This function will create a command line executable and the scripts required to run it
#!
#! \param NAME (required) Name of the algorithm / cmake target
#! \param DEPENDS (optional) Required MITK modules beyond MitkCommandLine
#! \param PACKAGE_DEPENDS (optional) list of "packages" this command line app depends on (e.g. ITK, VTK, etc.)
#! \param CPP_FILES (optional) list of cpp files, if it is not given NAME.cpp is assumed
#! \param INCLUDE_DIRS (optional): All directories that should be added as include dirs to the project
#! \param PROFILE (optional): The profile file that should be used for the algorithm. If not set it is "./<algname>.profile".
#! \param NO_PROFILE_GEN (optional): Flag. If set no profile resource will be generated.
#! \param ADDITIONAL_LIBS (optional) List of additional private libraries linked to this module.
#!        The folder containing the library will be added to the global list of library search paths.
#! \param H_FILES (optional) List of public header files for this module.
#! Assuming that there exists a file called <code>MyApp.cpp</code>, an example call looks like:
#! \code
#! mitkFunctionCreateCommandLineApp(
#!   NAME MyApp
#!   DEPENDS MitkCore MitkPlanarFigure
#!   PACKAGE_DEPENDS ITK VTK
#! )
#! \endcode
#!

function(mitkFunctionCreateMatchPointDeployedAlgorithm)

  set(_function_params
      NAME                # Name of the algorithm/target
      PROFILE             # Profile of the algorithm that should be used
     )

  set(_function_multiparams
      DEPENDS                # list of modules this command line app depends on
      PACKAGE_DEPENDS        # list of "packages" this command line app depends on (e.g. ITK, VTK, etc.)
      CPP_FILES              # (optional) list of cpp files, if it is not given NAME.cpp is assumed
      INCLUDE_DIRS           # include directories: [PUBLIC|PRIVATE|INTERFACE] <list>
      ADDITIONAL_LIBS        # list of addidtional private libraries linked to this module.
      H_FILES                # list of header files: [PUBLIC|PRIVATE] <list>
     )

  set(_function_options
       NO_PROFILE_GEN #Flag that indicates that no profile resource should be generated.
     )

  cmake_parse_arguments(ALG "${_function_options}" "${_function_params}" "${_function_multiparams}" ${ARGN})

  if( NOT (DEFINED MITK_USE_MatchPoint) OR NOT (${MITK_USE_MatchPoint}))
    message(FATAL_ERROR "Need package Matchpoint to deploy MatchPoint Algorithms.")
  endif()

  if(NOT ALG_NAME)
    message(FATAL_ERROR "NAME argument cannot be empty.")
  endif()

  SET(ALG_TARGET "MDRA_${ALG_NAME}")

  if(NOT ALG_CPP_FILES)
    set(ALG_CPP_FILES "${ALG_NAME}.cpp")
  endif()

  IF(NOT ALG_PROFILE)
    set(ALG_PROFILE "${ALG_NAME}.profile")
  ENDIF(NOT ALG_PROFILE)

  IF(NOT ALG_NO_PROFILE_GEN)
      MESSAGE(STATUS "... generate MDRA profile for ${ALG_NAME} (from ${ALG_PROFILE})...")

      include(${MatchPoint_SOURCE_DIR}/CMake/mapFunctionCreateAlgorithmProfile.cmake)
      CREATE_ALGORITHM_PROFILE(${ALG_NAME} ${ALG_PROFILE})

      MESSAGE(STATUS "... algorithm UID: ${ALGORITHM_PROFILE_UID}")
  ENDIF(NOT ALG_NO_PROFILE_GEN)

  MESSAGE(STATUS "... deploy MDRA algorithm ${ALG_NAME}...")
  ADD_LIBRARY(${ALG_TARGET} SHARED ${ALG_CPP_FILES} ${ALGORITHM_PROFILE_FILE})

  SET_TARGET_PROPERTIES(${ALG_TARGET} PROPERTIES
    OUTPUT_NAME "mdra-${MatchPoint_VERSION_MAJOR}-${MatchPoint_VERSION_MINOR}_${ALG_NAME}"
    OUTPUT_NAME_DEBUG "mdra-D-${MatchPoint_VERSION_MAJOR}-${MatchPoint_VERSION_MINOR}_${ALG_NAME}"
    PREFIX "" )

  mitk_use_modules(TARGET ${ALG_TARGET}
                   MODULES ${ALG_DEPENDS}
                   PACKAGES PRIVATE ITK MatchPoint ${ALG_PACKAGE_DEPENDS}
                   )

  target_include_directories(${ALG_TARGET} PRIVATE ${ALG_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR})

  if(ALG_ADDITIONAL_LIBS)
    target_link_libraries(${ALG_TARGET} PRIVATE ${ALG_ADDITIONAL_LIBS})
    get_property(_mitk_additional_library_search_paths GLOBAL PROPERTY MITK_ADDITIONAL_LIBRARY_SEARCH_PATHS)
    foreach(_lib_filepath ${ALG_ADDITIONAL_LIBS})
      get_filename_component(_search_path "${_lib_filepath}" PATH)
      if(_search_path)
        list(APPEND _mitk_additional_library_search_paths "${_search_path}")
      endif()
    endforeach()
    if(_mitk_additional_library_search_paths)
      list(REMOVE_DUPLICATES _mitk_additional_library_search_paths)
      set_property(GLOBAL PROPERTY MITK_ADDITIONAL_LIBRARY_SEARCH_PATHS ${_mitk_additional_library_search_paths})
    endif()
  endif()

  MITK_INSTALL(TARGETS ${ALG_TARGET})

endfunction()
