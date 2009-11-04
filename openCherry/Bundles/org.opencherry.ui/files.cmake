SET(SRC_CPP_FILES
  cherryAbstractSourceProvider.cpp
  cherryAbstractUIPlugin.cpp
  cherryConstants.cpp
  cherryDisplay.cpp
  cherryEditorPart.cpp
  cherryFileEditorInput.cpp
  cherryGeometry.cpp
  cherryIEditorRegistry.cpp
  cherryImageDescriptor.cpp
  cherryIMemento.cpp
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
  cherryWorkbenchPreferenceConstants.cpp
  cherryWorkbenchWindow.cpp
  cherryXMLMemento.cpp
  
  #application
  application/cherryActionBarAdvisor.cpp
  application/cherryWorkbenchAdvisor.cpp
  application/cherryWorkbenchWindowAdvisor.cpp
  
  #commands
  #commands/cherryAbstractContributionFactory.cpp
  #commands/cherryCommandContributionItem.cpp
  #commands/cherryCommandContributionItemParameter.cpp
  #commands/cherryContributionItem.cpp
  #commands/cherryContributionManager.cpp
  #commands/cherryICommandImageService.cpp
  #commands/cherryICommandService.cpp
  #commands/cherryIContributionManagerOverrides.cpp
  #commands/cherryIMenuItem.cpp
  #commands/cherryIMenuItemListener.cpp
  #commands/cherryIMenuListener.cpp
  #commands/cherryIToolItemListener.cpp
  #commands/cherryIUIElementListener.cpp
  #commands/cherryMenuManager.cpp
  #commands/cherrySubContributionItem.cpp
  #commands/cherryUIElement.cpp
  
  #dialogs
  dialogs/cherryIDialog.cpp
  dialogs/cherryMessageDialog.cpp
  
  #guitk
  guitk/cherryGuiTkControlEvent.cpp
  guitk/cherryGuiTkEvent.cpp
  guitk/cherryGuiTkIControlListener.cpp
  guitk/cherryGuiTkIMenuListener.cpp
  guitk/cherryGuiTkISelectionListener.cpp
  guitk/cherryGuiTkSelectionEvent.cpp
  
  #handlers
  handlers/cherryHandlerUtil.cpp
  handlers/cherryIHandlerActivation.cpp
  handlers/cherryShowViewHandler.cpp
  
  #src
  intro/cherryIntroPart.cpp
  
  #tweaklets
  tweaklets/cherryDnDTweaklet.cpp
  tweaklets/cherryGuiWidgetsTweaklet.cpp
  tweaklets/cherryImageTweaklet.cpp
  tweaklets/cherryMessageDialogTweaklet.cpp
  tweaklets/cherryWorkbenchPageTweaklet.cpp
  tweaklets/cherryWorkbenchTweaklet.cpp
  
  #presentations
  presentations/cherryIPresentablePart.cpp
  presentations/cherryIPresentationFactory.cpp
  presentations/cherryIStackPresentationSite.cpp
  presentations/cherryStackDropResult.cpp
  presentations/cherryStackPresentation.cpp
  
  #testing
  testing/cherryTestableObject.cpp
  
  #util
  util/cherrySafeRunnable.cpp
  
)


SET(INTERNAL_CPP_FILES

  #intro
  intro/cherryEditorIntroAdapterPart.cpp
  intro/cherryIntroConstants.cpp
  intro/cherryIntroDescriptor.cpp
  intro/cherryIntroPartAdapterSite.cpp
  intro/cherryIntroRegistry.cpp
  intro/cherryViewIntroAdapterPart.cpp
  intro/cherryWorkbenchIntroManager.cpp
  
  cherryAbstractPartSelectionTracker.cpp
  cherryAbstractSelectionService.cpp
  cherryBundleUtility.cpp
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
  cherryErrorViewPart.cpp
  cherryFileEditorMapping.cpp
  cherryFolderLayout.cpp
  cherryIDragOverListener.cpp
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
  cherryPreferenceConstants.cpp
  cherryPresentablePart.cpp
  cherryPresentationFactoryUtil.cpp
  cherryPresentationSerializer.cpp
  cherryRegistryReader.cpp
  cherrySaveablesList.cpp
  cherryServiceLocator.cpp
  cherryServiceLocatorCreator.cpp
  cherryShellPool.cpp
  cherrySourcePriorityNameMapping.cpp
  cherryStackablePart.cpp
  cherryStickyViewDescriptor.cpp
  cherryStickyViewManager.cpp
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
  cherryWindowManager.cpp
  cherryWindowPartSelectionTracker.cpp
  cherryWindowSelectionService.cpp
  cherryWorkbench.cpp
  cherryWorkbenchConfigurer.cpp
  cherryWorkbenchConstants.cpp
  cherryWorkbenchPagePartList.cpp
  cherryWorkbenchPartReference.cpp
  cherryWorkbenchPlugin.cpp
  cherryWorkbenchRegistryConstants.cpp
  cherryWorkbenchServiceRegistry.cpp
  cherryWorkbenchTestable.cpp
  cherryWorkbenchWindowConfigurer.cpp
  cherryWWinPartService.cpp
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
