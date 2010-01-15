
# Call this macro after calling MACRO_CREATE_PLUGIN in your CMakeLists.txt
#
# Variables:
#
# - OPENCHERRY_TEST_APP contains the name of the executable which will start the OSGi framework
#
MACRO(MACRO_TEST_PLUGIN)

  IF(WIN32)
    ADD_TEST(${BUNDLE-SYMBOLICNAME} ${OPENCHERRY_TEST_APP} /openCherry.application=coretestapplication /openCherry.testplugin=${BUNDLE-SYMBOLICNAME})
  ELSE()
    ADD_TEST(${BUNDLE-SYMBOLICNAME} ${OPENCHERRY_TEST_APP} --openCherry.application=coretestapplication --openCherry.testplugin=${BUNDLE-SYMBOLICNAME})
  ENDIF()
  
ENDMACRO(MACRO_TEST_PLUGIN)

# Variables:
#
# - OPENCHERRY_TEST_APP contains the name of the executable which will start the OSGi framework
# - OPENCHERRY_TEST_APP_ID contains the application id of the application to test. If empty,
#                          a minimalistic default application will be started
MACRO(MACRO_TEST_UIPLUGIN)

  IF(OPENCHERRY_ENABLE_GUI_TESTING)
      IF(OPENCHERRY_TEST_APP_ID)
        SET(_app_id_arg "openCherry.testapplication=${OPENCHERRY_TEST_APP_ID}")
        IF(WIN32)
          SET(_app_id_arg "/${_app_id_arg}")
        ELSE()
          SET(_app_id_arg "--${_app_id_arg}")
        ENDIF()
      ENDIF()
    
      IF(WIN32)
        ADD_TEST(${BUNDLE-SYMBOLICNAME} ${OPENCHERRY_TEST_APP} /openCherry.application=uitestapplication ${_app_id_arg} /openCherry.testplugin=${BUNDLE-SYMBOLICNAME})
      ELSE()
        ADD_TEST(${BUNDLE-SYMBOLICNAME} ${OPENCHERRY_TEST_APP} --openCherry.application=uitestapplication ${_app_id_arg} --openCherry.testplugin=${BUNDLE-SYMBOLICNAME})
      ENDIF()
  ENDIF()
  
ENDMACRO(MACRO_TEST_UIPLUGIN)