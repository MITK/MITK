SET(SRC_CPP_FILES
  cherryAbstractUIPlugin.cpp
  cherryEditorPart.cpp
  cherryIEditorRegistry.cpp
  cherryIPageLayout.cpp
  cherryISources.cpp
  cherryIWorkbenchPage.cpp
  cherryIWorkbenchPartConstants.cpp
  cherryPartPane.cpp
  cherryPlatformUI.cpp
  cherryPoint.cpp
  cherryRectangle.cpp
  cherryUIException.cpp
  cherryViewPart.cpp
  cherryWorkbench.cpp
  cherryWorkbenchPart.cpp
  cherryWorkbenchWindow.cpp
  
  #application
  application/cherryActionBarAdvisor.cpp
  application/cherryWorkbenchAdvisor.cpp
  application/cherryWorkbenchWindowAdvisor.cpp
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
  #cherryFolderLayout.cpp
  #cherryLayoutPart.cpp
  #cherryLayoutTree.cpp
  #cherryLayoutTreeNode.cpp
  cherryNullEditorInput.cpp
  #cherryPageLayout.cpp
  cherryPartList.cpp
  #cherryPartPlaceholder.cpp
  cherryPartService.cpp
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
  cherryDummyWorkbenchPage.cpp
  cherryWorkbenchConfigurer.cpp
  cherryWorkbenchPagePartList.cpp
  cherryWorkbenchPartReference.cpp
  cherryWorkbenchPlugin.cpp
  cherryWorkbenchRegistryConstants.cpp
  cherryWorkbenchWindowConfigurer.cpp
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