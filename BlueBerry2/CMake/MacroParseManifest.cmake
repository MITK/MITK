#
# MACRO_PARSE_MANIFEST(manifest [GENERATE_CMAKE_FILE])
#
# parses the file given as an absolute path in <manifest> and
# optionally writes the key/value pairs as CMake variables in
# the current binary directory with filename <bundle-id>_MF.cmake.
#
# The keys in the manifest file are available as upper case CMake
# variables after this macro completes.
#
MACRO(MACRO_PARSE_MANIFEST _manifest_)

    MACRO_PARSE_ARGUMENTS(_PARSE "OUTPUT" "" ${ARGN})

    # clear all supported manifest headers
    SET(BUNDLE-ACTIVATOR "")
    SET(BUNDLE-COPYRIGHT "")
    SET(BUNDLE-ACTIVATIONPOLICY "")
    SET(BUNDLE-NAME "")
    SET(BUNDLE-SYMBOLICNAME "")
    SET(BUNDLE-VENDOR "")
    SET(BUNDLE-VERSION "")
    SET(MANIFEST-VERSION "")
    SET(CHERRY-SYSTEMBUNDLE "")
    SET(REQUIRE-BUNDLE "")

    SET(_file "${_manifest_}")
    
    FILE(STRINGS "${_file}" _mflines LENGTH_MINIMUM 1 REGEX "[^ ]+.*:[]*[^ ]+")
     
    SET(_mfcontent "SET(BUNDLE-ACTIVATOR )
SET(BUNDLE-COPYRIGHT )
SET(BUNDLE-ACTIVATIONPOLICY )
SET(BUNDLE-NAME )
SET(BUNDLE-SYMBOLICNAME )
SET(BUNDLE-VENDOR )
SET(BUNDLE-VERSION )
SET(MANIFEST-VERSION )
SET(CHERRY-SYSTEMBUNDLE )
SET(REQUIRE-BUNDLE )")

    SET(_rbcontent "SET(REQUIRE-BUNDLE )")
    
    FOREACH(_line ${_mflines})
      STRING(REGEX MATCH "[^ ^:]+" _key "${_line}")
      STRING(REGEX REPLACE "^[^:]*[:](.*)" "\\1" _value "${_line}")
      
	  STRING(STRIP "${_value}" _value)
      
      #MESSAGE(SEND_ERROR "(${_key}, ${_value})")
      
      STRING(TOUPPER "${_key}" _ukey)
      
      IF(${_ukey} STREQUAL "REQUIRE-BUNDLE")
        # special handling for Require-Bundle
        # we generate a proper CMake list out of the comma-separated bundle-ids
        STRING(REPLACE "," ";" _value ${_value})
      
        FOREACH(_dep ${_value})
          # remove leading and trailing whitespace
          STRING(STRIP ${_dep} _trimeddep)
          LIST(APPEND ${_ukey} ${_trimeddep})
        ENDFOREACH()
        
        SET(_rbcontent "SET(REQUIRE-BUNDLE \"${${_ukey}}\")")
      ELSE()
        SET(${_ukey} "${_value}")
      ENDIF()
      
      SET(_mfcontent "${_mfcontent}
      
SET(${_ukey} \"${${_ukey}}\")")
      
    ENDFOREACH(_line ${_mflines})
    
    IF(_PARSE_OUTPUT)
      CONFIGURE_FILE("${OPENCHERRY_SOURCE_DIR}/CMake/cherryPluginManifest.cmake.in" "${_PARSE_OUTPUT}/Manifest.cmake" @ONLY)
      CONFIGURE_FILE("${OPENCHERRY_SOURCE_DIR}/CMake/cherryRequireBundle.cmake.in" "${_PARSE_OUTPUT}/RequireBundle.cmake" @ONLY)
    ENDIF()

ENDMACRO(MACRO_PARSE_MANIFEST)