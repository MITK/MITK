project(MbiLogExample)

find_package(MITK REQUIRED)

mitk_create_executable(LogExample DEPENDS MitkCore)
