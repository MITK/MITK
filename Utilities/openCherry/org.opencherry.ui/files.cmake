SET(CPP_FILES
  manifest.cpp
  cherryAbstractUIPlugin.cpp
  cherryEditorPart.cpp
  cherryIEditorRegistry.cpp
  cherryISources.cpp
  cherryIWorkbenchPage.cpp
  cherryPlatformUI.cpp
  cherryUIException.cpp
  cherryViewPart.cpp
  cherryWorkbench.cpp
  cherryWorkbenchPart.cpp
  cherryWorkbenchWindow.cpp
)

SET(HANDLERS_CPP_FILES
  cherryHandlerUtil.cpp
  cherryShowViewHandler.cpp
)

SET(INTERNAL_CPP_FILES
  cherryEditorDescriptor.cpp
  #cherryEditorManager.cpp
  #cherryEditorRegistry.cpp
  cherryEditorRegistryReader.cpp
  cherryEditorSite.cpp
  cherryFileEditorMapping.cpp
  #cherryPartSite.cpp
  cherryPartTester.cpp
  cherryRegistryReader.cpp
  cherryViewDescriptor.cpp
  cherryViewFactory.cpp
  cherryViewReference.cpp
  cherryViewRegistry.cpp
  cherryViewRegistryReader.cpp
  cherryViewSite.cpp
  #cherryWorkbenchPage.cpp
  cherryWorkbenchPartReference.cpp
  cherryWorkbenchPlugin.cpp
  cherryWorkbenchRegistryConstants.cpp
)

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

foreach(file ${HANDLERS_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} handlers/${file})
endforeach(file ${HANDLERS_CPP_FILES})