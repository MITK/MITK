#-----------------------------------------------------------------------------
# MITK Data
#-----------------------------------------------------------------------------

# Sanity checks
if(DEFINED MITK_DATA_DIR AND NOT EXISTS ${MITK_DATA_DIR})
  message(FATAL_ERROR "MITK_DATA_DIR variable is defined but corresponds to non-existing directory")
endif()

set(proj MITK-Data)
set(proj_DEPENDENCIES)
set(MITK-Data_DEPENDS ${proj})

if(BUILD_TESTING)

  set(revision_tag 9cb5f967fde0dece1835e51bf173510bea58da18)
  if(${proj}_REVISION_TAG)
    set(revision_tag ${${proj}_REVISION_TAG})
  endif()

  ExternalProject_Add(${proj}
    GIT_REPOSITORY http://git.mitk.org/MITK-Data.git
    GIT_TAG ${revision_tag}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS ${proj_DEPENDENCIES}
  )
  
  set(MITK_DATA_DIR ${ep_source_dir}/${proj})
  
else()

  mitkMacroEmptyExternalProject(${proj} "${proj_DEPENDENCIES}")
  
endif(BUILD_TESTING)

 
