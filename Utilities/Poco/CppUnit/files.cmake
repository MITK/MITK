SET(CPP_BASE_FILENAMES
 CppUnitException
 TestDecorator
 TestResult
 TestSuite
 TestCase
 TestFailure
 TestRunner
 TextTestResult
)

FOREACH(basename ${CPP_BASE_FILENAMES})
  SET(CPP_FILES ${CPP_FILES} src/${basename})
ENDFOREACH(basename ${CPP_BASE_FILENAMES})
