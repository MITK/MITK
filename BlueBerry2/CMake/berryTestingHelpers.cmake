
# Call this macro after calling MACRO_CREATE_PLUGIN in your CMakeLists.txt
#
# Variables:
#
# - BLUEBERRY_TEST_APP contains the name of the executable which will start the OSGi framework
#
MACRO(MACRO_TEST_PLUGIN)

  IF(WIN32)
    ADD_TEST(${BUNDLE-SYMBOLICNAME} ${BLUEBERRY_TEST_APP} /BlueBerry.application=coretestapplication /BlueBerry.testplugin=${BUNDLE-SYMBOLICNAME})
  ELSE()
    ADD_TEST(${BUNDLE-SYMBOLICNAME} ${BLUEBERRY_TEST_APP} --BlueBerry.application=coretestapplication --BlueBerry.testplugin=${BUNDLE-SYMBOLICNAME})
  ENDIF()
  
ENDMACRO(MACRO_TEST_PLUGIN)

# Variables:
#
# - BLUEBERRY_TEST_APP contains the name of the executable which will start the OSGi framework
# - BLUEBERRY_TEST_APP_ID contains the application id of the application to test. If empty,
#                          a minimalistic default application will be started
MACRO(MACRO_TEST_UIPLUGIN)

  IF(BLUEBERRY_ENABLE_GUI_TESTING)
      IF(BLUEBERRY_TEST_APP_ID)
        SET(_app_id_arg "BlueBerry.testapplication=${BLUEBERRY_TEST_APP_ID}")
        IF(WIN32)
          SET(_app_id_arg "/${_app_id_arg}")
        ELSE()
          SET(_app_id_arg "--${_app_id_arg}")
        ENDIF()
      ENDIF()
    
      IF(WIN32)
        ADD_TEST(${BUNDLE-SYMBOLICNAME} ${BLUEBERRY_TEST_APP} /BlueBerry.application=uitestapplication ${_app_id_arg} /BlueBerry.testplugin=${BUNDLE-SYMBOLICNAME})
      ELSE()
        ADD_TEST(${BUNDLE-SYMBOLICNAME} ${BLUEBERRY_TEST_APP} --BlueBerry.application=uitestapplication ${_app_id_arg} --BlueBerry.testplugin=${BUNDLE-SYMBOLICNAME})
      ENDIF()
  ENDIF()
  
ENDMACRO(MACRO_TEST_UIPLUGIN)