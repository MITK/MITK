mitk_create_module(
  INCLUDE_DIRS
    PUBLIC algorithms
    PRIVATE src/Helper src/Rendering
  DEPENDS MitkCore MitkSceneSerializationBase MitkMultilabel
  PACKAGE_DEPENDS
   PUBLIC MatchPoint
   PRIVATE VTK|ImagingGeneral+ImagingHybrid
)

if(TARGET ${MODULE_TARGET})
  set(ALG_PROFILE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/algorithms)

  include(${MatchPoint_SOURCE_DIR}/CMake/mapFunctionCreateAlgorithmProfile.cmake)
  file(GLOB ALG_PROFILE_FILES LIST_DIRECTORIES false RELATIVE ${ALG_PROFILE_DIR} "${ALG_PROFILE_DIR}/*.profile")

  foreach(profile_file ${ALG_PROFILE_FILES})
    get_filename_component(profile_name ${profile_file} NAME_WE)
    message(STATUS "Generate MDRA profile ${profile_name} (from ${profile_file})")
    CREATE_ALGORITHM_PROFILE(${profile_name} ${ALG_PROFILE_DIR}/${profile_file})
  endforeach()

  add_subdirectory(autoload/IO)
  add_subdirectory(deployment)
  if(BUILD_TESTING)
    add_subdirectory(Testing)
  endif()
  add_subdirectory(cmdapps)
endif()
