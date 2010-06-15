SET(CPP_BASE_FILENAMES
  CompressTest
  Driver
  PartialStreamTest
  ZipTest
  ZipTestSuite
)

IF(WIN32)
  SET(CPP_BASE_FILENAMES
    WinDriver
    ${CPP_BASE_FILENAMES}
  )
ENDIF(WIN32)

SET(CPP_FILES "")
FOREACH(basename ${CPP_BASE_FILENAMES})
  SET(CPP_FILES ${CPP_FILES} src/${basename})
ENDFOREACH(basename ${CPP_BASE_FILENAMES})
