MITK_CREATE_MODULE(IGTIO
                   DEPENDS MitkIGTBase
                   PACKAGE_DEPENDS tinyxml2|tinyxml2
                   AUTOLOAD_WITH MitkCore
                  )

if(TARGET ${MODULE_TARGET})
  target_link_libraries(${MODULE_TARGET}
    PRIVATE tinyxml2::tinyxml2
  )
endif()
