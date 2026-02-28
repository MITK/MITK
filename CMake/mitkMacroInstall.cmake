#
# MITK specific install macro
#
# On Mac everything is installed for each bundle listed in MACOSX_BUNDLE_NAMES
# by replacing the DESTINATION parameter. Everything else is passed to the CMake INSTALL command
#
# Usage: MITK_INSTALL( )
#
macro(MITK_INSTALL)

  set(ARGS ${ARGN})
  set(install_directories "")
  list(FIND ARGS DESTINATION _destination_index)
  if(_destination_index GREATER -1)
    message(SEND_ERROR "MITK_INSTALL macro must not be called with a DESTINATION parameter.")
  else()
    # If there is a FILES_MATCHING parameter we need to make sure that the
    # DESTINATION parameter is located in front of it.
    set(ARGS1 "")
    set(ARGS2 "")
    list(FIND ARGS FILES_MATCHING _files_matching_index)
    if(_files_matching_index GREATER -1)
      set(_index "0")
      list(LENGTH ARGS _count)
      while(_index LESS _files_matching_index )
        list(GET ARGS ${_index} _arg)
        list(APPEND ARGS1 ${_arg})
        math(EXPR _index "${_index}+1")
      endwhile()
      while(_index LESS _count)
        list(GET ARGS ${_index} _arg)
        list(APPEND ARGS2 ${_arg})
        math(EXPR _index "${_index}+1")
      endwhile()
    else()
      set(ARGS1 ${ARGS})
    endif()
    if(NOT MACOSX_BUNDLE_NAMES)
      install(${ARGS1} DESTINATION bin/${_install_DESTINATION} ${ARGS2})
    else()
      foreach(bundle_name ${MACOSX_BUNDLE_NAMES})
        install(${ARGS1} DESTINATION ${bundle_name}.app/Contents/MacOS/${_install_DESTINATION} ${ARGS2})
      endforeach()
    endif()
  endif()

endmacro()
