MITK_CREATE_MODULE(PET
  INCLUDE_DIRS
    PRIVATE src
  DEPENDS
    PUBLIC MitkCore MitkDICOM
  PACKAGE_DEPENDS
    PRIVATE DCMTK|ofstd+dcmdata
)

add_subdirectory(cmdapps)
add_subdirectory(test)

