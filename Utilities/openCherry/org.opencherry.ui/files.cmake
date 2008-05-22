SET(SRC_FILES
  cherryAbstractUIPlugin.cpp
  cherryEditorPart.cpp
  cherryIEditorRegistry.cpp
  cherryIPageLayout.cpp
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
  cherryEditorManager.cpp
  cherryEditorReference.cpp
  cherryEditorRegistry.cpp
  cherryEditorRegistryReader.cpp
  cherryEditorSite.cpp
  cherryFileEditorMapping.cpp
  cherryFolderLayout.cpp
  #cherryLayoutPart.cpp
  cherryNullEditorInput.cpp
  #cherryPageLayout.cpp
  #cherryPartPlaceholder.cpp
  cherryPartSite.cpp
  cherryPartTester.cpp
  #cherryPerspective.cpp
  #cherryPerspectiveDescriptor.cpp
  #cherryPlaceholderFolderLayout.cpp
  cherryRegistryReader.cpp
  cherryViewDescriptor.cpp
  cherryViewFactory.cpp
  cherryViewReference.cpp
  cherryViewRegistry.cpp
  cherryViewRegistryReader.cpp
  cherryViewSite.cpp
  #cherryWorkbenchPage.cpp
  #cherryWorkbenchPagePartList.cpp
  cherryWorkbenchPartReference.cpp
  cherryWorkbenchPlugin.cpp
  cherryWorkbenchRegistryConstants.cpp
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})

foreach(file ${HANDLERS_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/handlers/${file})
endforeach(file ${HANDLERS_CPP_FILES})