SET(CPP_FILES
  manifest.cpp
  cherryAbstractUIPlugin.cpp
  cherryISources.cpp
  cherryIWorkbench.cpp
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
  cherryRegistryReader.cpp
  cherryViewDescriptor.cpp
  cherryViewRegistry.cpp
  cherryViewRegistryReader.cpp
  cherryWorkbenchPlugin.cpp
  cherryWorkbenchRegistryConstants.cpp
)

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

foreach(file ${HANDLERS_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} handlers/${file})
endforeach(file ${HANDLERS_CPP_FILES})