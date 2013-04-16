
# Call this macro after calling MACRO_CREATE_PLUGIN in your CMakeLists.txt
#
# Variables:
#
# - BLUEBERRY_TEST_APP contains the name of the executable which will start the OSGi framework
#
macro(MACRO_TEST_PLUGIN)

  set(_cla_switch "--")
  if(WIN32)
    if(MSYS)
      set(_cla_switch "//")
    else()
      set(_cla_switch "/")
    endif()
  endif()

  if(NOT BUNDLE-SYMBOLICNAME)
    string(REPLACE "_" "." BUNDLE-SYMBOLICNAME ${PROJECT_NAME})
  endif()
  add_test(${BUNDLE-SYMBOLICNAME} ${BLUEBERRY_TEST_APP} ${_cla_switch}BlueBerry.application=coretestapplication ${_cla_switch}BlueBerry.testplugin=${BUNDLE-SYMBOLICNAME})

  set(_labels ${ARGN})
  if(NOT _labels)
    set(_labels BlueBerry)
  endif()
  set_property(TEST ${BUNDLE-SYMBOLICNAME} PROPERTY LABELS ${_labels})

endmacro(MACRO_TEST_PLUGIN)

# Variables:
#
# - BLUEBERRY_UI_TEST_APP contains the name of the executable which will start the OSGi framework
# - BLUEBERRY_TEST_APP_ID contains the application id of the application to test. If empty,
#                          a minimalistic default application will be started
macro(MACRO_TEST_UIPLUGIN)

  set(_cla_switch "--")
  if(WIN32)
    if(MSYS)
      set(_cla_switch "//")
    else()
      set(_cla_switch "/")
    endif()
  endif()

  if(NOT BUNDLE-SYMBOLICNAME)
    string(REPLACE "_" "." BUNDLE-SYMBOLICNAME ${PROJECT_NAME})
  endif()

  if(BLUEBERRY_ENABLE_GUI_TESTING)
    if(BLUEBERRY_TEST_APP_ID)
      set(_app_id_arg "${_cla_switch}BlueBerry.testapplication=${BLUEBERRY_TEST_APP_ID}")
    else()
      set(_app_id_arg )
    endif()

    add_test(${BUNDLE-SYMBOLICNAME} ${BLUEBERRY_UI_TEST_APP} ${_cla_switch}BlueBerry.application=uitestapplication ${_app_id_arg} ${_cla_switch}BlueBerry.testplugin=${BUNDLE-SYMBOLICNAME})

    set(_labels ${ARGN})
    if(NOT _labels)
      set(_labels BlueBerry)
    endif()
    set_property(TEST ${BUNDLE-SYMBOLICNAME} PROPERTY LABELS ${_labels})
  endif()

endmacro(MACRO_TEST_UIPLUGIN)
