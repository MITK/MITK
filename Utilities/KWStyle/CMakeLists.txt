if(UNIX)
  configure_file(${MITK_SOURCE_DIR}/Utilities/KWStyle/doAllStyle.csh.in
                 ${MITK_BINARY_DIR}/doAllStyle.csh
                 @ONLY IMMEDIATE)
  exec_program(chmod ARGS 755 \"${MITK_BINARY_DIR}/doAllStyle.csh\"
                     OUTPUT_VARIABLE MITK_EXEC_PLEASE_BE_QUIET)
endif(UNIX)
include(KWStyle.cmake)

