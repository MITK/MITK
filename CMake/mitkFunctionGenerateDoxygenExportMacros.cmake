#! \brief Generate a Doxyfile PREDEFINED snippet for all MITK module export
#!        macros so Doxygen does not misparse function declarations.
#!
#! Background: each MITK module ships a generated header (e.g. MitkCoreExports.h)
#! that defines macros like MITKCORE_EXPORT to platform-specific compiler
#! attributes (__attribute__((visibility("default"))) on GCC/Clang,
#! __declspec(dllexport) on MSVC). When such a macro appears between the return
#! type and the function name (e.g. "void MITKCORE_EXPORT Foo();"), Doxygen
#! 1.10+'s parser can misattribute the declaration to the macro expansion
#! instead of the function, dropping or garbling the rendered documentation.
#!
#! The fix is to tell Doxygen's own preprocessor to treat these macros as
#! defined-to-empty via the PREDEFINED Doxyfile setting. This function
#! discovers the macros from the generated *Exports.h files in the build tree
#! and returns them as a multi-line string suitable for substitution into a
#! Doxyfile template via configure_file().
#!
#! Usage:
#!   mitkFunctionGenerateDoxygenExportMacros(MITK_DOXYGEN_MODULE_EXPORT_MACROS)
#!   # ...then in the Doxyfile template:
#!   #   PREDEFINED += @MITK_DOXYGEN_MODULE_EXPORT_MACROS@
#!
#! Requirements / caveats:
#!  * Call sites must be processed AFTER add_subdirectory(Modules), so the
#!    generated *Exports.h headers exist when this function runs. The
#!    top-level CMakeLists.txt orders Documentation/ after Modules/ already.
#!  * file(GLOB_RECURSE) is evaluated at configure time. Adding a new module
#!    requires re-running cmake so the new module's export header is included.
#!  * The regex matches both GCC/Clang (__attribute__) and MSVC (__declspec)
#!    expansions, so the generated PREDEFINED is correct on both toolchains.

function(mitkFunctionGenerateDoxygenExportMacros output_var)
  # Recursive glob picks up autoload submodules (e.g.
  # Modules/DICOMPM/autoload/DICOMPMIO/MitkDICOMPMIOExports.h) and any other
  # nested module that calls generate_export_header() below Modules/.
  file(GLOB_RECURSE _export_headers
    "${CMAKE_BINARY_DIR}/Modules/Mitk*Exports.h"
  )

  set(_macros)
  foreach(_header ${_export_headers})
    file(STRINGS "${_header}" _lines
         REGEX "^[ \t#]*define[ \t]+MITK[A-Z0-9_]+[ \t]+(__attribute__|__declspec)")
    foreach(_line ${_lines})
      if(_line MATCHES "define[ \t]+(MITK[A-Z0-9_]+)[ \t]+(__attribute__|__declspec)")
        list(APPEND _macros "${CMAKE_MATCH_1}")
      endif()
    endforeach()
  endforeach()

  list(REMOVE_DUPLICATES _macros)
  list(SORT _macros)

  set(_result "")
  foreach(_macro ${_macros})
    if(_result)
      string(APPEND _result " \\\n                         ${_macro}=")
    else()
      set(_result "${_macro}=")
    endif()
  endforeach()

  set(${output_var} "${_result}" PARENT_SCOPE)
endfunction()
