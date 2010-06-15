IF(UNIX)
  CONFIGURE_FILE(${MITK_SOURCE_DIR}/Utilities/KWStyle/doAllStyle.csh.in
                 ${MITK_BINARY_DIR}/doAllStyle.csh
                 @ONLY IMMEDIATE)
  EXEC_PROGRAM(chmod ARGS 755 \"${MITK_BINARY_DIR}/doAllStyle.csh\"
                     OUTPUT_VARIABLE MITK_EXEC_PLEASE_BE_QUIET)
ENDIF(UNIX)
INCLUDE(KWStyle.cmake)

