project(DumpDir)

if(TARGET MitkDump)
  mitk_create_executable(PACKAGE_DEPENDS Qt4|QtCore WARNINGS_AS_ERRORS)
endif()
