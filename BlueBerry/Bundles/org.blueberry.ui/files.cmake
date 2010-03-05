SET(SRC_CPP_FILES
  berryAbstractSourceProvider.cpp
  berryAbstractUIPlugin.cpp
  berryConstants.cpp
  berryDisplay.cpp
  berryEditorPart.cpp
  berryFileEditorInput.cpp
  berryGeometry.cpp
  berryIEditorRegistry.cpp
  berryImageDescriptor.cpp
  berryIMemento.cpp
  berryIPageLayout.cpp
  berryIPartListener.cpp
  berryIPerspectiveListener.cpp
  berryIPropertyChangeListener.cpp
  berryISaveablePart.cpp
  berryISaveablesLifecycleListener.cpp
  berryISelectionChangedListener.cpp
  berryISelectionListener.cpp
  berryIShellListener.cpp
  berryISizeProvider.cpp
  berryISourceProviderListener.cpp
  berryISources.cpp
  berryIWindowListener.cpp
  berryIWorkbenchListener.cpp
  berryIWorkbenchPage.cpp
  berryIWorkbenchPartConstants.cpp
  berryPlatformUI.cpp
  berryPoint.cpp
  berryPropertyChangeEvent.cpp
  berryRectangle.cpp
  berrySameShellProvider.cpp
  berrySaveable.cpp
  berrySaveablesLifecycleEvent.cpp
  berrySelectionChangedEvent.cpp
  berryShell.cpp
  berryShellEvent.cpp
  berryUIException.cpp
  berryViewPart.cpp
  berryWindow.cpp
  berryWorkbenchPart.cpp
  berryWorkbenchPreferenceConstants.cpp
  berryXMLMemento.cpp
  
  #application
  application/berryActionBarAdvisor.cpp
  application/berryWorkbenchAdvisor.cpp
  application/berryWorkbenchWindowAdvisor.cpp
  
  #commands
  #commands/berryAbstractContributionFactory.cpp
  #commands/berryCommandContributionItem.cpp
  #commands/berryCommandContributionItemParameter.cpp
  #commands/berryContributionItem.cpp
  #commands/berryContributionManager.cpp
  #commands/berryICommandImageService.cpp
  #commands/berryICommandService.cpp
  #commands/berryIContributionManagerOverrides.cpp
  #commands/berryIMenuItem.cpp
  #commands/berryIMenuItemListener.cpp
  #commands/berryIMenuListener.cpp
  #commands/berryIToolItemListener.cpp
  #commands/berryIUIElementListener.cpp
  #commands/berryMenuManager.cpp
  #commands/berrySubContributionItem.cpp
  #commands/berryUIElement.cpp
  
  #dialogs
  dialogs/berryIDialog.cpp
  dialogs/berryMessageDialog.cpp
  
  #guitk
  guitk/berryGuiTkControlEvent.cpp
  guitk/berryGuiTkEvent.cpp
  guitk/berryGuiTkIControlListener.cpp
  guitk/berryGuiTkIMenuListener.cpp
  guitk/berryGuiTkISelectionListener.cpp
  guitk/berryGuiTkSelectionEvent.cpp
  
  #handlers
  handlers/berryHandlerUtil.cpp
  handlers/berryIHandlerActivation.cpp
  handlers/berryShowViewHandler.cpp
  
  #src
  intro/berryIntroPart.cpp
  
  #tweaklets
  tweaklets/berryDnDTweaklet.cpp
  tweaklets/berryGuiWidgetsTweaklet.cpp
  tweaklets/berryImageTweaklet.cpp
  tweaklets/berryMessageDialogTweaklet.cpp
  tweaklets/berryWorkbenchPageTweaklet.cpp
  tweaklets/berryWorkbenchTweaklet.cpp
  
  #presentations
  presentations/berryIPresentablePart.cpp
  presentations/berryIPresentationFactory.cpp
  presentations/berryIStackPresentationSite.cpp
  presentations/berryStackDropResult.cpp
  presentations/berryStackPresentation.cpp
  
  #testing
  testing/berryTestableObject.cpp
  
  #util
  util/berrySafeRunnable.cpp
  
)


SET(INTERNAL_CPP_FILES

  #intro
  intro/berryEditorIntroAdapterPart.cpp
  intro/berryIntroConstants.cpp
  intro/berryIntroDescriptor.cpp
  intro/berryIntroPartAdapterSite.cpp
  intro/berryIntroRegistry.cpp
  intro/berryViewIntroAdapterPart.cpp
  intro/berryWorkbenchIntroManager.cpp
  
  berryAbstractPartSelectionTracker.cpp
  berryAbstractSelectionService.cpp
  berryBundleUtility.cpp
  berryContainerPlaceholder.cpp
  berryDetachedPlaceHolder.cpp
  berryDefaultSaveable.cpp
  berryDefaultStackPresentationSite.cpp
  berryDetachedWindow.cpp
  berryDragUtil.cpp
  berryEditorAreaHelper.cpp
  berryEditorDescriptor.cpp
  berryEditorManager.cpp
  berryEditorReference.cpp
  berryEditorRegistry.cpp
  berryEditorRegistryReader.cpp
  berryEditorSashContainer.cpp
  berryEditorSite.cpp
  berryErrorViewPart.cpp
  berryFileEditorMapping.cpp
  berryFolderLayout.cpp
  berryIDragOverListener.cpp
  berryLayoutHelper.cpp
  berryLayoutPart.cpp
  berryLayoutPartSash.cpp
  berryLayoutTree.cpp
  berryLayoutTreeNode.cpp
  berryNullEditorInput.cpp
  berryPageLayout.cpp
  berryPagePartSelectionTracker.cpp
  berryPageSelectionService.cpp
  berryPartList.cpp
  berryPartPane.cpp
  berryPartPlaceholder.cpp
  berryPartSashContainer.cpp
  berryPartService.cpp
  berryPartSite.cpp
  berryPartStack.cpp
  berryPartTester.cpp
  berryPerspective.cpp
  berryPerspectiveDescriptor.cpp
  berryPerspectiveExtensionReader.cpp
  berryPerspectiveHelper.cpp
  berryPerspectiveRegistry.cpp
  berryPerspectiveRegistryReader.cpp
  berryPlaceholderFolderLayout.cpp
  berryPreferenceConstants.cpp
  berryPresentablePart.cpp
  berryPresentationFactoryUtil.cpp
  berryPresentationSerializer.cpp
  berryRegistryReader.cpp
  berrySaveablesList.cpp
  berryServiceLocator.cpp
  berryServiceLocatorCreator.cpp
  berryShellPool.cpp
  berrySourcePriorityNameMapping.cpp
  berryStackablePart.cpp
  berryStickyViewDescriptor.cpp
  berryStickyViewManager.cpp
  berryTweaklets.cpp
  berryViewDescriptor.cpp
  berryViewFactory.cpp
  berryViewLayout.cpp
  berryViewReference.cpp
  berryViewRegistry.cpp
  berryViewRegistryReader.cpp
  berryViewSashContainer.cpp
  berryViewSite.cpp
  berryWorkbenchPage.cpp
  berryWindowManager.cpp
  berryWindowPartSelectionTracker.cpp
  berryWindowSelectionService.cpp
  berryWorkbench.cpp
  berryWorkbenchConfigurer.cpp
  berryWorkbenchConstants.cpp
  berryWorkbenchPagePartList.cpp
  berryWorkbenchPartReference.cpp
  berryWorkbenchPlugin.cpp
  berryWorkbenchRegistryConstants.cpp
  berryWorkbenchServiceRegistry.cpp
  berryWorkbenchTestable.cpp
  berryWorkbenchWindow.cpp
  berryWorkbenchWindowConfigurer.cpp
  berryWWinPartService.cpp
)

SET(CPP_FILES manifest.cpp)

foreach(file ${SRC_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  SET(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
