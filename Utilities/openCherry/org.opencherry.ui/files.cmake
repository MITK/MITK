SET(SRC_CPP_FILES
  cherryAbstractSourceProvider.cpp
  cherryAbstractUIPlugin.cpp
  cherryConstants.cpp
  cherryEditorPart.cpp
  cherryIEditorRegistry.cpp
  cherryIPageLayout.cpp
  cherryIPartListener.cpp
  cherryIPerspectiveListener.cpp
  cherryIPropertyChangeListener.cpp
  cherryISaveablePart.cpp
  cherryISaveablesLifecycleListener.cpp
  cherryISelectionChangedListener.cpp
  cherryISelectionListener.cpp
  cherryIShellListener.cpp
  cherryISizeProvider.cpp
  cherryISourceProviderListener.cpp
  cherryISources.cpp
  cherryIWindowListener.cpp
  cherryIWorkbenchListener.cpp
  cherryIWorkbenchPage.cpp
  cherryIWorkbenchPartConstants.cpp
  cherryPartPane.cpp
  cherryPlatformUI.cpp
  cherryPoint.cpp
  cherryPropertyChangeEvent.cpp
  cherryRectangle.cpp
  cherrySameShellProvider.cpp
  cherrySaveable.cpp
  cherrySaveablesLifecycleEvent.cpp
  cherrySelectionChangedEvent.cpp
  cherryShell.cpp
  cherryShellEvent.cpp
  cherryUIException.cpp
  cherryViewPart.cpp
  cherryWindow.cpp
  cherryWorkbenchPart.cpp
  cherryWorkbenchWindow.cpp
  
  #application
  application/cherryActionBarAdvisor.cpp
  application/cherryWorkbenchAdvisor.cpp
  application/cherryWorkbenchWindowAdvisor.cpp
  
  #dialogs
  dialogs/cherryIDialog.cpp
  dialogs/cherryMessageDialog.cpp
  
  #guitk
  guitk/cherryGuiTkSelectionEvent.cpp
  
  #tweaklets
  tweaklets/cherryGuiWidgetsTweaklet.cpp
  tweaklets/cherryMessageDialogTweaklet.cpp
  tweaklets/cherryWorkbenchPageTweaklet.cpp
  tweaklets/cherryWorkbenchTweaklet.cpp
  
  #presentations
  presentations/cherryIPresentablePart.cpp
  presentations/cherryIPresentationFactory.cpp
  presentations/cherryIStackPresentationSite.cpp
  presentations/cherryStackPresentation.cpp
)

SET(HANDLERS_CPP_FILES
  cherryHandlerUtil.cpp
  cherryShowViewHandler.cpp
)

SET(INTERNAL_CPP_FILES
  cherryContainerPlaceholder.cpp
  cherryDetachedPlaceHolder.cpp
  cherryDefaultSaveable.cpp
  cherryDefaultStackPresentationSite.cpp
  cherryDetachedWindow.cpp
  cherryDragUtil.cpp
  cherryEditorAreaHelper.cpp
  cherryEditorDescriptor.cpp
  cherryEditorManager.cpp
  cherryEditorReference.cpp
  cherryEditorRegistry.cpp
  cherryEditorRegistryReader.cpp
  cherryEditorSashContainer.cpp
  cherryEditorSite.cpp
  cherryFileEditorMapping.cpp
  cherryFolderLayout.cpp
  cherryLayoutHelper.cpp
  cherryLayoutPart.cpp
  cherryLayoutPartSash.cpp
  cherryLayoutTree.cpp
  cherryLayoutTreeNode.cpp
  cherryNullEditorInput.cpp
  cherryPageLayout.cpp
  cherryPagePartSelectionTracker.cpp
  cherryPageSelectionService.cpp
  cherryPartList.cpp
  cherryPartPlaceholder.cpp
  cherryPartSashContainer.cpp
  cherryPartService.cpp
  cherryPartSite.cpp
  cherryPartStack.cpp
  cherryPartTester.cpp
  cherryPerspective.cpp
  cherryPerspectiveDescriptor.cpp
  cherryPerspectiveExtensionReader.cpp
  cherryPerspectiveHelper.cpp
  cherryPerspectiveRegistry.cpp
  cherryPerspectiveRegistryReader.cpp
  cherryPlaceholderFolderLayout.cpp
  cherryPresentablePart.cpp
  cherryPresentationFactoryUtil.cpp
  cherryPresentationSerializer.cpp
  cherryRegistryReader.cpp
  cherrySaveablesList.cpp
  cherryServiceLocator.cpp
  cherryServiceLocatorCreator.cpp
  cherrySourcePriorityNameMapping.cpp
  cherryStackablePart.cpp
  cherryTweaklets.cpp
  cherryViewDescriptor.cpp
  cherryViewFactory.cpp
  cherryViewLayout.cpp
  cherryViewReference.cpp
  cherryViewRegistry.cpp
  cherryViewRegistryReader.cpp
  cherryViewSashContainer.cpp
  cherryViewSite.cpp
  cherryWorkbenchPage.cpp
  #cherryDummyWorkbenchPage.cpp
  cherryWindowManager.cpp
  cherryWorkbench.cpp
  cherryWorkbenchConfigurer.cpp
  cherryWorkbenchConstants.cpp
  cherryWorkbenchPagePartList.cpp
  cherryWorkbenchPartReference.cpp
  cherryWorkbenchPlugin.cpp
  cherryWorkbenchRegistryConstants.cpp
  cherryWorkbenchServiceRegistry.cpp
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