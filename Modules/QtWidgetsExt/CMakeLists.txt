MITK_CREATE_MODULE(
  DEPENDS MitkAlgorithmsExt MitkQtWidgets PRIVATE MitkSceneSerializationBase
  PACKAGE_DEPENDS
    PUBLIC Qwt CTK|CTKWidgets
    PRIVATE Qt5|Concurrent+Svg+Xml VTK|IOImage
)

if (TARGET ${MODULE_TARGET} AND MSVC)
  #[[ Compiler warnings/errors because of QList, resp. Qwt on Visual Studio 2022 version 17.8:

      'stdext::checked_array_iterator<const T *>': warning STL4043: stdext::checked_array_iterator,
      stdext::unchecked_array_iterator, and related factory functions are non-Standard extensions
      and will be removed in the future. std::span (since C++20) and gsl::span can be used instead.
      You can define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING or _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
      to suppress this warning.
   ]]
  target_compile_definitions(${MODULE_TARGET} PUBLIC _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
endif()
