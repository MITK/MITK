mitk_create_module(
  DEPENDS PUBLIC MitkCore
  PACKAGE_DEPENDS
    PRIVATE Qt6|Core Poco|Zip
)

add_subdirectory(test)
