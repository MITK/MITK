project(Dump)
find_package(MITK REQUIRED)

mitk_create_executable(DEPENDS MitkDICOMReader WARNINGS_AS_ERRORS)