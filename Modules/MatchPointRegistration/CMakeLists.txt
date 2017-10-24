MITK_CREATE_MODULE(
  INCLUDE_DIRS PUBLIC Rendering Helper algorithms
  DEPENDS MitkCore MitkSceneSerializationBase
  PACKAGE_DEPENDS
   PUBLIC MatchPoint
)

if(TARGET ${MODULE_TARGET})
  set(ALG_PROFILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/algorithms)

  include(${MatchPoint_SOURCE_DIR}/CMake/mapFunctionCreateAlgorithmProfile.cmake)
  file(GLOB ALG_PROFILE_FILES LIST_DIRECTORIES false RELATIVE ${ALG_PROFILE_DIR} "${ALG_PROFILE_DIR}/*.profile")

  foreach(profile_file ${ALG_PROFILE_FILES})
    get_filename_component(profile_name ${profile_file} NAME_WE)
    MESSAGE(STATUS "... generate MDRA profile ${profile_name} (from ${profile_file})...")
    CREATE_ALGORITHM_PROFILE(${profile_name} ${ALG_PROFILE_DIR}/${profile_file})
  endforeach(profile_file)

  ADD_SUBDIRECTORY(autoload/IO)
  ADD_SUBDIRECTORY(deployment)
  if(BUILD_TESTING)
    ADD_SUBDIRECTORY(Testing)
  endif(BUILD_TESTING)
endif()
