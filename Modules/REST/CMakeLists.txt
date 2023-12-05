set(boost_depends "Boost|date_time+regex+system")

if(UNIX)
  set(boost_depends "${boost_depends}+atomic+chrono+filesystem+random+thread")
endif()

mitk_create_module(
  DEPENDS MitkCore
  PACKAGE_DEPENDS PUBLIC cpprestsdk OpenSSL|SSL ${boost_depends}
)

if(TARGET ${MODULE_TARGET})
  if(MSVC)
    #[[ Compiler warnings/errors because of C++ REST SDK's http_msg.h on Visual Studio 2022 version 17.8:

        'stdext::checked_array_iterator<const T *>': warning STL4043: stdext::checked_array_iterator,
        stdext::unchecked_array_iterator, and related factory functions are non-Standard extensions
        and will be removed in the future. std::span (since C++20) and gsl::span can be used instead.
        You can define _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING or _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
        to suppress this warning.
     ]]
    target_compile_definitions(${MODULE_TARGET} PUBLIC _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING)
  endif()

  add_subdirectory(test)
endif()
