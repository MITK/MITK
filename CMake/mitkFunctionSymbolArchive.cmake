# Defines the "package-symbols" target, which archives MITK's own Release
# debug symbols (see MITK_RELEASE_DEBUG_SYMBOLS) into
# <CPACK_PACKAGE_FILE_NAME>-symbols.zip next to the other CPack artifacts.
#
# The actual globbing and archiving run at build time via mitkPackageSymbols.cmake,
# because the symbol files only exist once the binaries have been linked.
# Included from mitkSetupCPack.cmake, so CPACK_PACKAGE_FILE_NAME is available.

if(NOT TARGET package-symbols)
  add_custom_target(package-symbols
    COMMAND "${CMAKE_COMMAND}"
      "-DMITK_SYMBOL_RUNTIME_DIR=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
      "-DMITK_SYMBOL_OUTPUT=${MITK_BINARY_DIR}/${CPACK_PACKAGE_FILE_NAME}-symbols.zip"
      "-DMITK_SYMBOL_STAGING_DIR=${MITK_BINARY_DIR}/mitk-symbols-staging"
      "-DMITK_SYMBOL_CONFIG=$<CONFIG>"
      -P "${MITK_SOURCE_DIR}/CMake/mitkPackageSymbols.cmake"
    VERBATIM
    USES_TERMINAL
    COMMENT "Archiving MITK debug symbols")
endif()
