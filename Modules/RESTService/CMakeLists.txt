if(MITK_USE_cpprestsdk)

  mitk_create_module(
    DEPENDS MitkREST
    AUTOLOAD_WITH MitkCore
  )

endif()
