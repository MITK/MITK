#! Matches CMAKE_OSX_DEPLOYMENT_TARGET to the minimum macOS version of Qt.
#!
#! A binary does not load on a macOS older than any library it links, so a
#! package built for an older macOS than Qt supports fails the checks of
#! FixMacOSInstaller.cmake. Without an explicit target, Qt's minimum is used.
#! A target chosen this way follows a later change of Qt in the same build
#! tree, while an explicit target is kept and only warned about if it is lower
#! than Qt's minimum. Without Qt, an empty target defaults to the major
#! version of the SDK.
#!
#! Call it after find_package(Qt6).
#!
#! \param SPECIFIED Whether CMAKE_OSX_DEPLOYMENT_TARGET was set before
#!        project(), which otherwise may default it to the running macOS.

include(mitkMachOTools)

function(mitkFunctionCheckMacOSDeploymentTarget)
  cmake_parse_arguments(PARSE_ARGV 0 _arg "" "SPECIFIED" "")

  set(_qt_minos "")
  if(TARGET Qt6::Core)
    get_target_property(_qt_core Qt6::Core LOCATION)
    mitk_macho_read("${_qt_core}" MINOS _qt_minos)
  endif()

  set(_target "${CMAKE_OSX_DEPLOYMENT_TARGET}")

  if(_arg_SPECIFIED AND _target AND NOT _target STREQUAL MITK_OSX_DEPLOYMENT_TARGET_FROM_QT)
    if(_qt_minos AND _target VERSION_LESS _qt_minos)
      message(WARNING
        "CMAKE_OSX_DEPLOYMENT_TARGET (${_target}) is lower than the minimum "
        "macOS version of Qt ${Qt6_VERSION} (${_qt_minos}). Qt does not load "
        "on macOS ${_target}, so packaging fails.")
    endif()
  elseif(_qt_minos)
    if(NOT _target STREQUAL _qt_minos)
      message(STATUS
        "Setting CMAKE_OSX_DEPLOYMENT_TARGET to ${_qt_minos}, the minimum "
        "macOS version of Qt ${Qt6_VERSION}, as no target was specified. Other "
        "external dependencies may require a newer macOS, which packaging "
        "reports as \"requires macOS\" errors for their binaries. Set "
        "CMAKE_OSX_DEPLOYMENT_TARGET explicitly to override.")
      set(CMAKE_OSX_DEPLOYMENT_TARGET "${_qt_minos}" CACHE STRING "Deployment target version for macOS" FORCE)
    endif()
    set(MITK_OSX_DEPLOYMENT_TARGET_FROM_QT "${_qt_minos}" CACHE INTERNAL "")
  elseif(NOT _target AND CMAKE_OSX_SYSROOT)
    get_filename_component(_sdk_name "${CMAKE_OSX_SYSROOT}" NAME)
    string(REGEX REPLACE "^MacOSX([0-9]+)\\..*" "\\1" _sdk_major "${_sdk_name}")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "${_sdk_major}.0" CACHE STRING "Deployment target version for macOS" FORCE)
  endif()
endfunction()
