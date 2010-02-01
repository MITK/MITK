
# Call this macro after calling MACRO_CREATE_PLUGIN in your CMakeLists.txt
#
# Variables:
#
# - BLUEBERRY_TEST_APP contains the name of the executable which will start the OSGi framework
#
MACRO(MACRO_TEST_PLUGIN)

  SET(_cla_switch "--")
  IF(WIN32)
    IF(MSYS)
      SET(_cla_switch "//")
    ELSE()
      SET(_cla_switch "/")
    ENDIF()
  ENDIF()

  ADD_TEST(${BUNDLE-SYMBOLICNAME} ${BLUEBERRY_TEST_APP} ${_cla_switch}BlueBerry.application=coretestapplication ${_cla_switch}BlueBerry.testplugin=${BUNDLE-SYMBOLICNAME})
  
ENDMACRO(MACRO_TEST_PLUGIN)

# Variables:
#
# - BLUEBERRY_TEST_APP contains the name of the executable which will start the OSGi framework
# - BLUEBERRY_TEST_APP_ID contains the application id of the application to test. If empty,
#                          a minimalistic default application will be started
MACRO(MACRO_TEST_UIPLUGIN)

  SET(_cla_switch "--")
  IF(WIN32)
    IF(MSYS)
      SET(_cla_switch "//")
    ELSE()
      SET(_cla_switch "/")
    ENDIF()
  ENDIF()

  IF(BLUEBERRY_ENABLE_GUI_TESTING)
    IF(BLUEBERRY_TEST_APP_ID)
      SET(_app_id_arg "${_cla_switch}BlueBerry.testapplication=${BLUEBERRY_TEST_APP_ID}")
        
      ADD_TEST(${BUNDLE-SYMBOLICNAME} ${BLUEBERRY_TEST_APP} ${_cla_switch}BlueBerry.application=uitestapplication ${_app_id_arg} ${_cla_switch}BlueBerry.testplugin=${BUNDLE-SYMBOLICNAME})
    ENDIF()
  ENDIF()
  
ENDMACRO(MACRO_TEST_UIPLUGIN)
