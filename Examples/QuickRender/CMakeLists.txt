project(QuickRender)
find_package(MITK REQUIRED)

mitk_create_executable(DEPENDS MitkQmlItems MitkLegacyIO)
