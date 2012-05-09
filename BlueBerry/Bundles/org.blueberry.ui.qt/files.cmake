set(SRC_CPP_FILES
  berryAbstractSourceProvider.cpp
  berryAbstractUICTKPlugin.cpp
  berryConstants.cpp
  berryDisplay.cpp
  berryEditorPart.cpp
  berryFileEditorInput.cpp
  berryGeometry.cpp

  berryIActionBars.h
  berryIContributionRoot.h
  berryIDropTargetListener.cpp
  berryIEditorDescriptor.cpp
  berryIEditorInput.cpp
  berryIEditorMatchingStrategy.cpp
  berryIEditorPart.cpp
  berryIEditorReference.cpp
  berryIEditorRegistry.cpp
  berryIEditorSite.cpp
  berryIFileEditorMapping.cpp
  berryIFolderLayout.cpp
  berryImageDescriptor.cpp
  berryIMemento.cpp
  berryINullSelectionListener.cpp
  berryIPageLayout.cpp
  berryIPartListener.cpp
  berryIPageService.cpp
  berryIPartService.cpp
  berryIPathEditorInput.cpp
  berryIPerspectiveDescriptor.cpp
  berryIPerspectiveFactory.cpp
  berryIPerspectiveListener.cpp
  berryIPerspectiveRegistry.cpp
  berryIPlaceholderFolderLayout.cpp
  berryIPluginContribution.h
  berryIPostSelectionProvider.cpp
  berryIPreferencePage.cpp
  berryIPropertyChangeListener.cpp
  berryIQtPreferencePage.cpp
  berryIQtStyleManager.cpp
  berryIReusableEditor.cpp
  berryISaveablePart.cpp
  berryISaveablesLifecycleListener.cpp
  berryISaveablesSource.cpp
  berryISelection.cpp
  berryISelectionChangedListener.cpp
  berryISelectionListener.cpp
  berryISelectionProvider.cpp
  berryISelectionService.cpp
  berryIShellListener.cpp
  berryIShellProvider.cpp
  berryISizeProvider.cpp
  berryISourceProvider.cpp
  berryISourceProviderListener.cpp
  berryISources.cpp
  berryIStickyViewDescriptor.cpp
  berryIStructuredSelection.cpp
  berryIViewCategory.cpp
  berryIViewDescriptor.cpp
  berryIViewLayout.cpp
  berryIViewPart.cpp
  berryIViewReference.cpp
  berryIViewRegistry.cpp
  berryIViewSite.cpp
  berryIWorkbenchCommandConstants.cpp
  berryIWindowListener.cpp
  berryIWorkbench.cpp
  berryIWorkbenchListener.cpp
  berryIWorkbenchPage.cpp
  berryIWorkbenchPart.cpp
  berryIWorkbenchPartConstants.cpp
  berryIWorkbenchPartDescriptor.cpp
  berryIWorkbenchPartReference.cpp
  berryIWorkbenchPartSite.cpp
  berryIWorkbenchSite.cpp
  berryIWorkbenchWindow.cpp
  berryMenuUtil.cpp
  berryPlatformUI.cpp
  berryPoint.cpp
  berryPropertyChangeEvent.cpp
  berryQCHPluginListener.cpp
  berryQModelIndexObject.cpp
  berryQtAssistantUtil.cpp
  berryQtEditorPart.cpp
  berryQtItemSelection.cpp
  berryQtIntroPart.cpp
  berryQtPreferences.cpp
  berryQtSelectionProvider.cpp
  berryQtViewPart.cpp
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
  berryWorkbenchActionConstants.cpp
  berryWorkbenchPart.cpp
  berryWorkbenchPreferenceConstants.cpp
  berryXMLMemento.cpp

  #actions
  actions/berryAbstractContributionFactory.cpp
  actions/berryAbstractGroupMarker.cpp
  #actions/berryAction.cpp
  #actions/berryActionContributionItem.cpp
  actions/berryCommandContributionItem.h
  actions/berryCommandContributionItem.cpp
  actions/berryContributionItem.cpp
  actions/berryContributionItemFactory.cpp
  actions/berryContributionManager.cpp
  actions/berryIContributionItem.h
  actions/berryIContributionManager.h
  actions/berryIContributionManagerOverrides.cpp
  actions/berryIMenuManager.h
  #actions/berryManuBarManager.cpp
  actions/berryMenuManager.cpp
  actions/berrySeparator.cpp
  actions/berrySubContributionItem.cpp


  #application
  application/berryActionBarAdvisor.cpp
  application/berryIActionBarConfigurer.cpp
  application/berryIWorkbenchConfigurer.cpp
  application/berryIWorkbenchWindowConfigurer.cpp
  application/berryWorkbenchAdvisor.cpp
  application/berryWorkbenchWindowAdvisor.cpp

  #commands
  commands/berryICommandImageService.cpp
  commands/berryICommandService.cpp
  commands/berryIElementReference.h
  commands/berryIElementUpdater.h
  commands/berryIMenuService.h
  commands/berryUIElement.cpp

  #dialogs
  dialogs/berryIDialog.cpp
  dialogs/berryIShowViewDialog.cpp
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
  handlers/berryIHandlerService.cpp
  handlers/berryShowViewHandler.cpp

  #intro
  intro/berryIIntroManager.cpp
  intro/berryIIntroPart.cpp
  intro/berryIIntroSite.cpp
  intro/berryIntroPart.cpp

  #tweaklets
  tweaklets/berryDnDTweaklet.cpp
  tweaklets/berryGuiWidgetsTweaklet.cpp
  tweaklets/berryImageTweaklet.cpp
  tweaklets/berryMessageDialogTweaklet.cpp
  tweaklets/berryITracker.cpp
  tweaklets/berryWorkbenchPageTweaklet.cpp
  tweaklets/berryWorkbenchTweaklet.cpp

  #presentations
  presentations/berryIPresentablePart.cpp
  presentations/berryIPresentationFactory.cpp
  presentations/berryIPresentationSerializer.cpp
  presentations/berryIStackPresentationSite.cpp
  presentations/berryStackDropResult.cpp
  presentations/berryStackPresentation.cpp

  #services
  services/berryIDisposable.cpp
  services/berryIEvaluationReference.h
  services/berryIEvaluationService.cpp
  services/berryINestable.cpp
  services/berryIServiceFactory.cpp
  services/berryIServiceLocator.cpp
  services/berryIServiceWithSources.cpp

  #testing
  testing/berryTestableObject.cpp

  #util
  util/berryISafeRunnableRunner.cpp
  util/berrySafeRunnable.cpp

  # application
  application/berryQtWorkbenchAdvisor.cpp
)

set(INTERNAL_CPP_FILES

  defaultpresentation/berryEmptyTabFolder.cpp
  defaultpresentation/berryEmptyTabItem.cpp
  defaultpresentation/berryNativeTabFolder.cpp
  defaultpresentation/berryNativeTabItem.cpp
  defaultpresentation/berryQCTabBar.cpp
  defaultpresentation/berryQtWorkbenchPresentationFactory.cpp

  util/berryAbstractTabFolder.cpp
  util/berryAbstractTabItem.cpp
  util/berryIPresentablePartList.cpp
  util/berryLeftToRightTabOrder.cpp
  util/berryPartInfo.cpp
  util/berryPresentablePartFolder.cpp
  util/berryReplaceDragHandler.cpp
  util/berryTabbedStackPresentation.cpp
  util/berryTabDragHandler.cpp
  util/berryTabFolderEvent.cpp
  util/berryTabOrder.cpp

  #intro
  intro/berryEditorIntroAdapterPart.cpp
  intro/berryIIntroDescriptor.cpp
  intro/berryIIntroRegistry.cpp
  intro/berryIntroConstants.cpp
  intro/berryIntroDescriptor.cpp
  intro/berryIntroPartAdapterSite.cpp
  intro/berryIntroRegistry.cpp
  intro/berryViewIntroAdapterPart.cpp
  intro/berryWorkbenchIntroManager.cpp

  berryAbstractMenuAdditionCacheEntry.cpp
  berryAbstractPartSelectionTracker.cpp
  berryAbstractSelectionService.cpp
  berryAlwaysEnabledExpression.cpp
  berryBundleUtility.cpp
  berryCommandContributionItemParameter.cpp
  berryCommandService.cpp
  berryContainerPlaceholder.cpp
  berryContributionRoot.cpp
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
  berryElementReference.cpp
  berryErrorViewPart.cpp
  berryEvaluationAuthority.cpp
  berryEvaluationReference.cpp
  berryEvaluationResultCache.cpp
  berryEvaluationService.cpp
  berryExpressionAuthority.cpp
  berryFileEditorMapping.cpp
  berryFolderLayout.cpp
  berryIDragOverListener.cpp
  berryIDropTarget.cpp
  berryIEvaluationResultCache.cpp
  berryILayoutContainer.cpp
  berryIServiceLocatorCreator.cpp
  berryIStickyViewManager.cpp
  berryIWorkbenchLocationService.cpp
  berryLayoutHelper.cpp
  berryLayoutPart.cpp
  berryLayoutPartSash.cpp
  berryLayoutTree.cpp
  berryLayoutTreeNode.cpp
  berryMMMenuListener.cpp
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
  berryQtControlWidget.cpp
  berryQtDnDControlWidget.cpp
  berryQtDisplay.cpp
  berryQtDnDTweaklet.cpp
  berryQtFileImageDescriptor.cpp
  berryQtGlobalEventFilter.cpp
  berryQtIconImageDescriptor.cpp
  berryQtImageTweaklet.cpp
  berryQtMainWindowControl.cpp
  berryQtMessageDialogTweaklet.cpp
  berryQtMissingImageDescriptor.cpp
  berryQtOpenPerspectiveAction.cpp
  berryQtPerspectiveSwitcher.cpp
  berryQtSafeApplication.cpp
  berryQtSash.cpp
  berryQtShell.cpp
  berryQtShowViewAction.cpp
  berryQtShowViewDialog.cpp
  berryQtStyleManager.cpp
  berryQtStylePreferencePage.cpp
  berryQtTracker.cpp
  berryQtWidgetController.cpp
  berryQtWidgetsTweaklet.cpp
  berryQtWidgetsTweakletImpl.cpp
  berryQtWorkbenchPageTweaklet.cpp
  berryQtWorkbenchTweaklet.cpp
  berryRegistryReader.cpp
  berrySaveablesList.cpp
  berryShowViewMenu.cpp
  berryServiceLocator.cpp
  berryServiceLocatorCreator.cpp
  berryShellPool.cpp
  berrySourcePriorityNameMapping.cpp
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
  berryWorkbenchMenuService.cpp
  berryWorkbenchPagePartList.cpp
  berryWorkbenchPartReference.cpp
  berryWorkbenchPlugin.cpp
  berryWorkbenchRegistryConstants.cpp
  berryWorkbenchServiceRegistry.cpp
  berryWorkbenchTestable.cpp
  berryWorkbenchWindow.cpp
  berryWorkbenchWindowConfigurer.cpp
  berryWWinActionBars.cpp
  berryWWinPartService.cpp

)

set(MOC_H_FILES

  src/berryEditorPart.h
  src/berryViewPart.h
  src/berryWorkbenchPart.h
  src/intro/berryIntroPart.h


  src/berryQCHPluginListener.h
  src/berryQtSelectionProvider.h

  src/internal/berryMMMenuListener.h

  src/internal/defaultpresentation/berryNativeTabFolder.h
  src/internal/defaultpresentation/berryNativeTabItem.h
  src/internal/defaultpresentation/berryQCTabBar.h
  src/internal/defaultpresentation/berryQtWorkbenchPresentationFactory.h

  src/internal/berryWorkbenchPlugin.h
  src/internal/intro/berryEditorIntroAdapterPart.h

  src/internal/berryQtDisplay.h
  src/internal/berryQtDnDTweaklet.h
  src/internal/berryQtGlobalEventFilter.h
  src/internal/berryQtImageTweaklet.h
  src/internal/berryQtMainWindowControl.h
  src/internal/berryQtMessageDialogTweaklet.h
  src/internal/berryQtOpenPerspectiveAction.h
  src/internal/berryQtPerspectiveSwitcher.h
  src/internal/berryQtSash.h
  src/internal/berryQtShowViewAction.h
  src/internal/berryQtStyleManager.h
  src/internal/berryQtStylePreferencePage.h
  src/internal/berryQtTracker.h
  src/internal/berryQtWidgetsTweaklet.h
  src/internal/berryQtWidgetsTweakletImpl.h
  src/internal/berryQtWorkbenchTweaklet.h
  src/internal/berryQtWorkbenchPageTweaklet.h
)

set(UI_FILES
  src/internal/berryQtShowViewDialog.ui
  src/internal/berryQtStylePreferencePage.ui
  src/internal/berryQtStatusPart.ui
)

set(QRC_FILES
  resources/org_blueberry_ui_qt.qrc
)

set(CACHED_RESOURCE_FILES
  plugin.xml
)

set(CPP_FILES )

foreach(file ${SRC_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/${file})
endforeach(file ${SRC_CPP_FILES})

foreach(file ${INTERNAL_CPP_FILES})
  set(CPP_FILES ${CPP_FILES} src/internal/${file})
endforeach(file ${INTERNAL_CPP_FILES})
