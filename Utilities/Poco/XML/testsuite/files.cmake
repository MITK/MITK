set(CPP_BASE_FILENAMES
 AttributesImplTest ChildNodesTest DOMTestSuite DocumentTest
 DocumentTypeTest Driver ElementTest EventTest NamePoolTest NameTest
 NamespaceSupportTest NodeIteratorTest NodeTest ParserWriterTest
 SAXParserTest SAXTestSuite TextTest TreeWalkerTest
 XMLTestSuite XMLWriterTest NodeAppenderTest
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
