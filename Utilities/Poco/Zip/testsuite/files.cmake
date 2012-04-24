set(CPP_BASE_FILENAMES
  CompressTest
  Driver
  PartialStreamTest
  ZipTest
  ZipTestSuite
)

if(WIN32)
  set(CPP_BASE_FILENAMES
    WinDriver
    ${CPP_BASE_FILENAMES}
  )
endif(WIN32)

set(CPP_FILES "")
foreach(basename ${CPP_BASE_FILENAMES})
  set(CPP_FILES ${CPP_FILES} src/${basename})
endforeach(basename ${CPP_BASE_FILENAMES})
