
MACRO(MACRO_PARSE_MANIFEST _manifest_)

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

    SET(_file ${_manifest_})
    
    #FILE(STRINGS ${_file} _mflines LENGTH_MINIMUM 1 REGEX .*:.+)
    
    FILE(READ ${_file} _mfcontent)
    STRING(REGEX MATCHALL [^\n\r]+ _mflines "${_mfcontent}")
    
    FOREACH(_line ${_mflines})
      STRING(REGEX MATCH " *[^ ^:]+" _key "${_line}")
      STRING(REGEX REPLACE "^[^:]*[:][ ]*(.*)[ ]*" "\\1" _value "${_line}")
      
      IF(_key STREQUAL "Bundle-SymbolicName")
        STRING(REGEX MATCH "[^ ]+" _value "${_value}")
      ENDIF(_key STREQUAL "Bundle-SymbolicName")
      
      #MESSAGE(SEND_ERROR "(${_key}, ${_value})")
      
      STRING(TOUPPER "${_key}" _ukey)
      
      SET(${_ukey} "${_value}")
    ENDFOREACH(_line ${_mflines})

ENDMACRO(MACRO_PARSE_MANIFEST)