project(org_blueberry_ui_qt)

set(PLUGIN_exported_include_suffixes
  src
  src/actions
  src/application
  src/commands
  src/guitk
  src/handlers
  src/intro
  src/model
  src/presentations
  src/services
  src/testing
  src/tweaklets
  src/util
)

if(MITK_USE_Qt5)
  set(PLUGIN_package_depends
    PUBLIC Qt5|Widgets+PrintSupport+Svg
  )
endif()

mitk_create_plugin(
  EXPORT_DIRECTIVE BERRY_UI_QT
  EXPORTED_INCLUDE_SUFFIXES ${PLUGIN_exported_include_suffixes}
  MODULE_DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS ${PLUGIN_package_depends}
)

if (TARGET ${PLUGIN_TARGET} AND MSVC)
  #[[ Compiler warnings/errors because of QList on Visual Studio 2022 version 17.8:

      'stdext::checked_array_iterator<const T *>': warning STL4043: stdext::checked_array_iterator,
      stdext::unchecked_array_iterator, and related factory functions are non-Standard extensions
      and will be removed in the future. std::span (since C++20) and gsl::span can be used instead.
      You can define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING or _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
      to suppress this warning.
   ]]
  target_compile_definitions(${PLUGIN_TARGET} PRIVATE _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
endif()
