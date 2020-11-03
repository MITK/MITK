if(MITK_USE_cpprestsdk)

  mitk_create_module(
    DEPENDS MitkCore
  )

  if(TARGET ${MODULE_TARGET})
    target_link_libraries(${MODULE_TARGET} PUBLIC cpprestsdk::cpprest OpenSSL::SSL)
  endif()

  add_subdirectory(test)

endif()
