/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryWorkbench.h"

#include "../tweaklets/cherryWorkbenchTweaklet.h"

#include "cherrySaveablesList.h"
#include "cherryViewRegistry.h"
#include "cherryEditorRegistry.h"
#include "cherryServiceLocatorCreator.h"
#include "../dialogs/cherryMessageDialog.h"
#include "../cherryWorkbenchWindow.h"

#include "cherryWorkbenchPlugin.h"
#include "cherryWorkbenchConstants.h"

#include <Poco/Bugcheck.h>

namespace cherry
{

Workbench* Workbench::instance = 0;

int Workbench::CreateAndRunWorkbench(WorkbenchAdvisor* advisor)
{
  // create the workbench instance
  Workbench workbench(advisor);
  // run the workbench event loop
  int returnCode = workbench.RunUI();

  return returnCode;
}

Workbench::ServiceLocatorOwner::ServiceLocatorOwner(Workbench* wb)
 : workbench(wb)
 {

 }

void Workbench::ServiceLocatorOwner::Dispose() {
  MessageDialog::OpenInformation(
              0,
              "Restart needed",
              "A required plug-in is no longer available and the Workbench needs to be restarted. You will be prompted to save if there is any unsaved work.");
  workbench->Close(PlatformUI::RETURN_RESTART, true);
}


Workbench::Workbench(WorkbenchAdvisor* advisor)
 : serviceLocatorOwner(this), largeUpdates(0), isStarting(true), isClosing(false)
{
  poco_check_ptr(advisor);

  this->advisor = advisor;
  Workbench::instance = this;

  IServiceLocatorCreator::Pointer slc = new ServiceLocatorCreator();
  this->serviceLocator = dynamic_cast<ServiceLocator*>(slc->CreateServiceLocator(0, 0, &serviceLocatorOwner));
  serviceLocator->RegisterService(IServiceLocatorCreator::GetManifestName(), slc);
  returnCode = PlatformUI::RETURN_UNSTARTABLE;
}

Workbench* Workbench::GetInstance()
{
  return instance;
}

Workbench::~Workbench()
{
  delete serviceLocator;
}

Object::Pointer Workbench::GetService(const std::string& key) const
{
return serviceLocator->GetService(key);
}

bool Workbench::HasService(const std::string& key) const
{
  return serviceLocator->HasService(key);
}

bool Workbench::Init()
{
  bool bail = false;

  // create workbench window manager
  //windowManager = new WindowManager();

  // TODO Correctly order service initialization
  // there needs to be some serious consideration given to
  // the services, and hooking them up in the correct order
  //final EvaluationService restrictionService = new EvaluationService();
  //final EvaluationService evaluationService = new EvaluationService();

  //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //      public void runWithException() {
  //        serviceLocator.registerService(IRestrictionService.class,
  //            restrictionService);
  //        serviceLocator.registerService(IEvaluationService.class,
  //            evaluationService);
  //      }
  //    });

  // Initialize the activity support.
  //workbenchActivitySupport = new WorkbenchActivitySupport();
  //activityHelper = ActivityPersistanceHelper.getInstance();

  this->InitializeDefaultServices();
  //    initializeFonts();
  //    initializeColors();
  //    initializeApplicationColors();

  // now that the workbench is sufficiently initialized, let the advisor
  // have a turn.

  advisor->InternalBasicInitialize(this->GetWorkbenchConfigurer());

  // attempt to restore a previous workbench state

  advisor->PreStartup();

  if (!advisor->OpenWindows())
  {
    bail = true;
  }

  if (bail)
    return false;

  //forceOpenPerspective();

  return true;
}

void Workbench::InitializeDefaultServices()
{

//    final IContributionService contributionService = new ContributionService(
//        getAdvisor());
//    serviceLocator.registerService(IContributionService.class,
//        contributionService);
//
//    // TODO Correctly order service initialization
//    // there needs to be some serious consideration given to
//    // the services, and hooking them up in the correct order
//    final IEvaluationService evaluationService =
//      (IEvaluationService) serviceLocator.getService(IEvaluationService.class);
//
//
//
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
        serviceLocator->RegisterService(ISaveablesLifecycleListener::GetManifestName(),
            new SaveablesList());
//      }});
//
//    /*
//     * Phase 1 of the initialization of commands. When this phase completes,
//     * all the services and managers will exist, and be accessible via the
//     * getService(Object) method.
//     */
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        Command.DEBUG_COMMAND_EXECUTION = Policy.DEBUG_COMMANDS;
//        commandManager = new CommandManager();
//      }});
//
//    final CommandService [] commandService = new CommandService[1];
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        commandService[0] = new CommandService(commandManager);
//        commandService[0].readRegistry();
//        serviceLocator.registerService(ICommandService.class, commandService[0]);
//
//      }});
//
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        ContextManager.DEBUG = Policy.DEBUG_CONTEXTS;
//        contextManager = new ContextManager();
//        }});
//
//    final IContextService contextService = new ContextService(
//        contextManager);
//
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        contextService.readRegistry();
//        }});
//
//    serviceLocator.registerService(IContextService.class, contextService);
//
//
//    final IBindingService [] bindingService = new BindingService[1];
//
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        BindingManager.DEBUG = Policy.DEBUG_KEY_BINDINGS;
//        bindingManager = new BindingManager(contextManager, commandManager);
//        bindingService[0] = new BindingService(
//            bindingManager, commandService[0], Workbench.this);
//
//      }});
//
//    bindingService[0].readRegistryAndPreferences(commandService[0]);
//    serviceLocator.registerService(IBindingService.class, bindingService[0]);
//
//    final CommandImageManager commandImageManager = new CommandImageManager();
//    final CommandImageService commandImageService = new CommandImageService(
//        commandImageManager, commandService[0]);
//    commandImageService.readRegistry();
//    serviceLocator.registerService(ICommandImageService.class,
//        commandImageService);
//
//    final WorkbenchMenuService menuService = new WorkbenchMenuService(serviceLocator);
//
//    serviceLocator.registerService(IMenuService.class, menuService);
//    // the service must be registered before it is initialized - its
//    // initialization uses the service locator to address a dependency on
//    // the menu service
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        menuService.readRegistry();
//      }});
//
//    /*
//     * Phase 2 of the initialization of commands. The source providers that
//     * the workbench provides are creating and registered with the above
//     * services. These source providers notify the services when particular
//     * pieces of workbench state change.
//     */
//    final SourceProviderService sourceProviderService = new SourceProviderService(serviceLocator);
//    serviceLocator.registerService(ISourceProviderService.class,
//        sourceProviderService);
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        // this currently instantiates all players ... sigh
//        sourceProviderService.readRegistry();
//        ISourceProvider[] sp = sourceProviderService.getSourceProviders();
//        for (int i = 0; i < sp.length; i++) {
//          evaluationService.addSourceProvider(sp[i]);
//          if (!(sp[i] instanceof ActiveContextSourceProvider)) {
//            contextService.addSourceProvider(sp[i]);
//          }
//        }
//      }});
//
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        // these guys are need to provide the variables they say
//        // they source
//        actionSetSourceProvider = (ActionSetSourceProvider) sourceProviderService
//            .getSourceProvider(ISources.ACTIVE_ACTION_SETS_NAME);
//
//        FocusControlSourceProvider focusControl = (FocusControlSourceProvider) sourceProviderService
//            .getSourceProvider(ISources.ACTIVE_FOCUS_CONTROL_ID_NAME);
//        serviceLocator.registerService(IFocusService.class,
//            focusControl);
//
//        menuSourceProvider = (MenuSourceProvider) sourceProviderService
//            .getSourceProvider(ISources.ACTIVE_MENU_NAME);
//      }});
//
//    /*
//     * Phase 3 of the initialization of commands. This handles the creation
//     * of wrappers for legacy APIs. By the time this phase completes, any
//     * code trying to access commands through legacy APIs should work.
//     */
//    final IHandlerService[] handlerService = new IHandlerService[1];
//    StartupThreading.runWithoutExceptions(new StartupRunnable() {
//
//      public void runWithException() {
//        handlerService[0] = (IHandlerService) serviceLocator
//            .getService(IHandlerService.class);
//      }
//    });
//    workbenchContextSupport = new WorkbenchContextSupport(this,
//        contextManager);
//    workbenchCommandSupport = new WorkbenchCommandSupport(bindingManager,
//        commandManager, contextManager, handlerService[0]);
//    initializeCommandResolver();
//
//    addWindowListener(windowListener);
//    bindingManager.addBindingManagerListener(bindingManagerListener);
//
//    serviceLocator.registerService(ISelectionConversionService.class,
//        new SelectionConversionService());
  }

int Workbench::RunUI()
{

  // initialize workbench and restore or open one window
  bool initOK = this->Init();

  // let the advisor run its start up code
  if (initOK)
  {
    advisor->PostStartup(); // may trigger a close/restart
  }

  //TODO start eager plug-ins
  //startPlugins();

  //addStartupRegistryListener();

  // spin event loop
  return Tweaklets::Get(WorkbenchTweaklet::KEY)->RunEventLoop();
}

std::string Workbench::GetDefaultPerspectiveId()
{
  return this->GetAdvisor()->GetInitialWindowPerspectiveId();
}

IAdaptable* Workbench::GetDefaultPageInput()
{
  return this->GetAdvisor()->GetDefaultPageInput();
}

std::string Workbench::GetPresentationId() {
    if (factoryID != "") {
      return factoryID;
    }

    //factoryID = PrefUtil.getAPIPreferenceStore().getString(
    //    IWorkbenchPreferenceConstants.PRESENTATION_FACTORY_ID);

    // Workaround for bug 58975 - New preference mechanism does not properly
    // initialize defaults
    // Ensure that the UI plugin has started too.
    factoryID = WorkbenchConstants::DEFAULT_PRESENTATION_ID;

    return factoryID;
  }

void Workbench::LargeUpdateStart() {
  if (largeUpdates++ == 0) {
    // TODO Consider whether these lines still need to be here.
    // workbenchCommandSupport.setProcessing(false);
    // workbenchContextSupport.setProcessing(false);

    std::vector<IWorkbenchWindow::Pointer> windows = this->GetWorkbenchWindows();
    for (unsigned int i = 0; i < windows.size(); i++) {
      IWorkbenchWindow::Pointer window = windows[i];
      if (window.Cast<WorkbenchWindow>() != 0) {
        window.Cast<WorkbenchWindow>()->LargeUpdateStart();
      }
    }
  }
}


void Workbench::LargeUpdateEnd() {
  if (--largeUpdates == 0) {
    // TODO Consider whether these lines still need to be here.
    // workbenchCommandSupport.setProcessing(true);
    // workbenchContextSupport.setProcessing(true);

    // Perform window-specific blocking.
    std::vector<IWorkbenchWindow::Pointer> windows = this->GetWorkbenchWindows();
    for (unsigned int i = 0; i < windows.size(); i++) {
      IWorkbenchWindow::Pointer window = windows[i];
      if (window.Cast<WorkbenchWindow>() != 0) {
        window.Cast<WorkbenchWindow>()->LargeUpdateEnd();
      }
    }
  }
}

void Workbench::OpenFirstTimeWindow()
{
  try
  {
    IAdaptable* input;
    //StartupThreading.runWithoutExceptions(new StartupRunnable() {

    //  public void runWithException() throws Throwable {
    input = this->GetDefaultPageInput();
    //  }});

    this->BusyOpenWorkbenchWindow(this->GetPerspectiveRegistry()->GetDefaultPerspective(), input);
  } catch (WorkbenchException& e)
  {
    // Don't use the window's shell as the dialog parent,
    // as the window is not open yet (bug 76724).
    //StartupThreading.runWithoutExceptions(new StartupRunnable() {

    //  public void runWithException() throws Throwable {
    //    ErrorDialog.openError(null,
    //        WorkbenchMessages.Problems_Opening_Page, e.getMessage(), e
    //            .getStatus());
    //  }});
    std::cout << "Error: Problems opening page. " << e.displayText() << std::endl;
  }
}

WorkbenchConfigurer::Pointer Workbench::GetWorkbenchConfigurer()
{
  if (workbenchConfigurer.IsNull())
  {
    workbenchConfigurer = new WorkbenchConfigurer();
  }
  return workbenchConfigurer;
}

WorkbenchAdvisor* Workbench::GetAdvisor()
{
  return advisor;
}

IViewRegistry* Workbench::GetViewRegistry()
{
  return WorkbenchPlugin::GetDefault()->GetViewRegistry();
}

IEditorRegistry* Workbench::GetEditorRegistry()
{
  return WorkbenchPlugin::GetDefault()->GetEditorRegistry();
}

IPerspectiveRegistry* Workbench::GetPerspectiveRegistry()
{
  return WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry();
}

bool Workbench::Close()
{
  return this->Close(PlatformUI::RETURN_OK, false);
}

bool Workbench::Close(int returnCode, bool force)
{
  this->returnCode = returnCode;
  bool ret;
  //BusyIndicator.showWhile(null, new Runnable() {
  //  public void run() {
      ret = this->BusyClose(force);
  //  }
  //});
  return ret;
}

/**
 * Closes the workbench. Assumes that the busy cursor is active.
 *
 * @param force
 *            true if the close is mandatory, and false if the close is
 *            allowed to fail
 * @return true if the close succeeded, and false otherwise
 */
bool Workbench::BusyClose(bool force) {

  // notify the advisor of preShutdown and allow it to veto if not forced
  isClosing = advisor->PreShutdown();
  if (!force && !isClosing) {
    return false;
  }

  // notify regular workbench clients of preShutdown and allow them to
  // veto if not forced
  isClosing = this->FirePreShutdown(force);
  if (!force && !isClosing) {
    return false;
  }

  // save any open editors if they are dirty
  isClosing = this->SaveAllEditors(!force);
  if (!force && !isClosing) {
    return false;
  }

  bool closeEditors = !force;
     // && PrefUtil.getAPIPreferenceStore().getBoolean(
     //     IWorkbenchPreferenceConstants.CLOSE_EDITORS_ON_EXIT);
  if (closeEditors) {
    // SafeRunner.run(new SafeRunnable() {
    //  public void run() {
        std::vector<IWorkbenchWindow::Pointer> windows = this->GetWorkbenchWindows();
        for (unsigned int i = 0; i < windows.size(); i++) {
          IWorkbenchPage::Pointer page = windows[i]->GetActivePage();
          isClosing = isClosing && page->CloseAllEditors(false);
        }
     // }
    //});
    if (!force && !isClosing) {
      return false;
    }
  }

  if (this->GetWorkbenchConfigurer()->GetSaveAndRestore()) {
//    SafeRunner.run(new SafeRunnable() {
//      public void run() {
//        XMLMemento mem = recordWorkbenchState();
//        // Save the IMemento to a file.
//        saveMementoToFile(mem);
//      }
//
//      public void handleException(Throwable e) {
//        String message;
//        if (e.getMessage() == null) {
//          message = WorkbenchMessages.ErrorClosingNoArg;
//        } else {
//          message = NLS.bind(
//              WorkbenchMessages.ErrorClosingOneArg, e
//                  .getMessage());
//        }
//
//        if (!MessageDialog.openQuestion(null,
//            WorkbenchMessages.Error, message)) {
//          isClosing = false;
//        }
//      }
//    });
  }
  if (!force && !isClosing) {
    return false;
  }

  //SafeRunner.run(new SafeRunnable(WorkbenchMessages.ErrorClosing) {
  //  public void run() {
      if (isClosing || force) {
        isClosing = windowManager.Close();
      }
  //  }
  //});

  if (!force && !isClosing) {
    return false;
  }

  this->Shutdown();

  //runEventLoop = false;
  return true;
}

IWorkbenchWindow::Pointer Workbench::GetActiveWorkbenchWindow()
{
  // Look for the window that was last known being
  // the active one
  WorkbenchWindow::Pointer win = this->GetActivatedWindow();
  return win;
}

int Workbench::GetWorkbenchWindowCount()
{
  return windowManager.GetWindowCount();
}

std::vector<IWorkbenchWindow::Pointer> Workbench::GetWorkbenchWindows()
{
  std::vector<Window::Pointer> windows = windowManager.GetWindows();
  std::vector<IWorkbenchWindow::Pointer> result;
  for (std::vector<Window::Pointer>::iterator iter = windows.begin();
       iter != windows.end(); ++iter)
  {
    result.push_back(iter->Cast<WorkbenchWindow>());
  }

  return result;
}

IWorkbenchWindow::Pointer Workbench::OpenWorkbenchWindow(
    const std::string& perspID, IAdaptable* input)
{
  // Run op in busy cursor.
  //final Object[] result = new Object[1];
  //BusyIndicator.showWhile(null, new Runnable() {
  //  public void run() {
  //    try {
  return this->BusyOpenWorkbenchWindow(perspID, input);
  //    } catch (WorkbenchException e) {
  //      result[0] = e;
  //    }
  //  }
  //});

}

IWorkbenchWindow::Pointer Workbench::OpenWorkbenchWindow(IAdaptable* input)
{
  //TODO perspective work
  //    return this->OpenWorkbenchWindow(this->GetPerspectiveRegistry()
  //            ->GetDefaultPerspective(), input);
  return this->OpenWorkbenchWindow("", input);
}

IWorkbenchPage::Pointer Workbench::ShowPerspective(
    const std::string& perspectiveId, IWorkbenchWindow::Pointer window)
{
  // If the specified window has the requested perspective open, then the
  // window
  // is given focus and the perspective is shown. The page's input is
  // ignored.
  WorkbenchWindow::Pointer win = window.Cast<WorkbenchWindow> ();
  if (win.IsNotNull())
  {
    IWorkbenchPage::Pointer page = win->GetActivePage(); //GetActiveWorkbenchPage();
    if (page.IsNotNull())
    {
      //        IPerspectiveDescriptor perspectives[] = page
      //            .getOpenPerspectives();
      //        for (int i = 0; i < perspectives.length; i++) {
      //          IPerspectiveDescriptor persp = perspectives[i];
      //          if (perspectiveId.equals(persp.getId())) {
      //            win.makeVisible();
      //            page.setPerspective(persp);
      return page;
      //          }
      //        }
    }
  }

  return 0;

  // If another window that has the workspace root as input and the
  // requested
  // perpective open and active, then the window is given focus.

  // TODO showPerspective
  //    IAdaptable input = getDefaultPageInput();
  //    IWorkbenchWindow[] windows = getWorkbenchWindows();
  //    for (int i = 0; i < windows.length; i++) {
  //      win = (WorkbenchWindow) windows[i];
  //      if (window != win) {
  //        WorkbenchPage page = win.getActiveWorkbenchPage();
  //        if (page != null) {
  //          boolean inputSame = false;
  //          if (input == null) {
  //            inputSame = (page.getInput() == null);
  //          } else {
  //            inputSame = input.equals(page.getInput());
  //          }
  //          if (inputSame) {
  //            Perspective persp = page.getActivePerspective();
  //            if (persp != null) {
  //              IPerspectiveDescriptor desc = persp.getDesc();
  //              if (desc != null) {
  //                if (perspectiveId.equals(desc.getId())) {
  //                  Shell shell = win.getShell();
  //                  shell.open();
  //                  if (shell.getMinimized()) {
  //                    shell.setMinimized(false);
  //                  }
  //                  return page;
  //                }
  //              }
  //            }
  //          }
  //        }
  //      }
  //    }

  // Otherwise the requested perspective is opened and shown in the
  // specified
  // window or in a new window depending on the current user preference
  // for opening
  // perspectives, and that window is given focus.

  //TODO showPerspective
  //    win = (WorkbenchWindow) window;
  //    if (win != null) {
  //      IPreferenceStore store = WorkbenchPlugin.getDefault()
  //          .getPreferenceStore();
  //      int mode = store.getInt(IPreferenceConstants.OPEN_PERSP_MODE);
  //      IWorkbenchPage page = win.getActiveWorkbenchPage();
  //      IPerspectiveDescriptor persp = null;
  //      if (page != null) {
  //        persp = page.getPerspective();
  //      }
  //
  //      // Only open a new window if user preference is set and the window
  //      // has an active perspective.
  //      if (IPreferenceConstants.OPM_NEW_WINDOW == mode && persp != null) {
  //        IWorkbenchWindow newWindow = openWorkbenchWindow(perspectiveId,
  //            input);
  //        return newWindow.getActivePage();
  //      }
  //
  //      IPerspectiveDescriptor desc = getPerspectiveRegistry()
  //          .findPerspectiveWithId(perspectiveId);
  //      if (desc == null) {
  //        throw new WorkbenchException(
  //            NLS
  //                .bind(
  //                    WorkbenchMessages.WorkbenchPage_ErrorCreatingPerspective,
  //                    perspectiveId));
  //      }
  //      win.getShell().open();
  //      if (page == null) {
  //        page = win.openPage(perspectiveId, input);
  //      } else {
  //        page.setPerspective(desc);
  //      }
  //      return page;
  //    }
  //
  //    // Just throw an exception....
  //    throw new WorkbenchException(NLS
  //        .bind(WorkbenchMessages.Workbench_showPerspectiveError,
  //            perspectiveId));
}

IWorkbenchPage::Pointer Workbench::ShowPerspective(
    const std::string& perspectiveId, IWorkbenchWindow::Pointer window,
    IAdaptable* input)
{
  return 0;
  //    // If the specified window has the requested perspective open and the
  //    // same requested
  //    // input, then the window is given focus and the perspective is shown.
  //    bool inputSameAsWindow = false;
  //    WorkbenchWindow::Pointer win = window.Cast<WorkbenchWindow>();
  //    if (win.IsNotNull()) {
  //      WorkbenchPage::Pointer page = win->GetActiveWorkbenchPage();
  //      if (page.IsNotNull()) {
  //        bool inputSame = false;
  //        if (input == 0) {
  //          inputSame = (page->GetInput() == 0);
  //        } else {
  //          inputSame = input.equals(page.getInput());
  //        }
  //        if (inputSame) {
  //          inputSameAsWindow = true;
  //          IPerspectiveDescriptor perspectives[] = page
  //              .getOpenPerspectives();
  //          for (int i = 0; i < perspectives.length; i++) {
  //            IPerspectiveDescriptor persp = perspectives[i];
  //            if (perspectiveId.equals(persp.getId())) {
  //              win.makeVisible();
  //              page.setPerspective(persp);
  //              return page;
  //            }
  //          }
  //        }
  //      }
  //    }
  //
  //    // If another window has the requested input and the requested
  //    // perpective open and active, then that window is given focus.
  //    IWorkbenchWindow[] windows = getWorkbenchWindows();
  //    for (int i = 0; i < windows.length; i++) {
  //      win = (WorkbenchWindow) windows[i];
  //      if (window != win) {
  //        WorkbenchPage page = win.getActiveWorkbenchPage();
  //        if (page != null) {
  //          boolean inputSame = false;
  //          if (input == null) {
  //            inputSame = (page.getInput() == null);
  //          } else {
  //            inputSame = input.equals(page.getInput());
  //          }
  //          if (inputSame) {
  //            Perspective persp = page.getActivePerspective();
  //            if (persp != null) {
  //              IPerspectiveDescriptor desc = persp.getDesc();
  //              if (desc != null) {
  //                if (perspectiveId.equals(desc.getId())) {
  //                  win.getShell().open();
  //                  return page;
  //                }
  //              }
  //            }
  //          }
  //        }
  //      }
  //    }
  //
  //    // If the specified window has the same requested input but not the
  //    // requested
  //    // perspective, then the window is given focus and the perspective is
  //    // opened and shown
  //    // on condition that the user preference is not to open perspectives in
  //    // a new window.
  //    win = (WorkbenchWindow) window;
  //    if (inputSameAsWindow && win != null) {
  //      IPreferenceStore store = WorkbenchPlugin.getDefault()
  //          .getPreferenceStore();
  //      int mode = store.getInt(IPreferenceConstants.OPEN_PERSP_MODE);
  //
  //      if (IPreferenceConstants.OPM_NEW_WINDOW != mode) {
  //        IWorkbenchPage page = win.getActiveWorkbenchPage();
  //        IPerspectiveDescriptor desc = getPerspectiveRegistry()
  //            .findPerspectiveWithId(perspectiveId);
  //        if (desc == null) {
  //          throw new WorkbenchException(
  //              NLS
  //                  .bind(
  //                      WorkbenchMessages.WorkbenchPage_ErrorCreatingPerspective,
  //                      perspectiveId));
  //        }
  //        win.getShell().open();
  //        if (page == null) {
  //          page = win.openPage(perspectiveId, input);
  //        } else {
  //          page.setPerspective(desc);
  //        }
  //        return page;
  //      }
  //    }
  //
  //    // If the specified window has no active perspective, then open the
  //    // requested perspective and show the specified window.
  //    if (win != null) {
  //      IWorkbenchPage page = win.getActiveWorkbenchPage();
  //      IPerspectiveDescriptor persp = null;
  //      if (page != null) {
  //        persp = page.getPerspective();
  //      }
  //      if (persp == null) {
  //        IPerspectiveDescriptor desc = getPerspectiveRegistry()
  //            .findPerspectiveWithId(perspectiveId);
  //        if (desc == null) {
  //          throw new WorkbenchException(
  //              NLS
  //                  .bind(
  //                      WorkbenchMessages.WorkbenchPage_ErrorCreatingPerspective,
  //                      perspectiveId));
  //        }
  //        win.getShell().open();
  //        if (page == null) {
  //          page = win.openPage(perspectiveId, input);
  //        } else {
  //          page.setPerspective(desc);
  //        }
  //        return page;
  //      }
  //    }
  //
  //    // Otherwise the requested perspective is opened and shown in a new
  //    // window, and the
  //    // window is given focus.
  //    IWorkbenchWindow newWindow = openWorkbenchWindow(perspectiveId, input);
  //    return newWindow.getActivePage();
}

bool Workbench::SaveAllEditors(bool confirm)
{
  return true;
}

bool Workbench::IsRunning()
{
  return Tweaklets::Get(WorkbenchTweaklet::KEY)->IsRunning();
}

bool Workbench::IsStarting()
{
  return isStarting;
}

bool Workbench::IsClosing()
{
  return isClosing;
}

WorkbenchWindow::Pointer Workbench::GetActivatedWindow()
{
  return activatedWindow;
}

/*
 * Sets the workbench window which was last known being the active one, or
 * <code> null </code> .
 */
void Workbench::SetActivatedWindow(WorkbenchWindow::Pointer window)
{
  activatedWindow = window;
}

WorkbenchWindow::Pointer Workbench::NewWorkbenchWindow()
{
  WorkbenchWindow::Pointer wbw = new WorkbenchWindow(this->GetNewWindowNumber());
  //wbw->Init();
  return wbw;
}

int Workbench::GetNewWindowNumber()
{
  // Get window list.
  std::vector<Window::Pointer> windows = windowManager.GetWindows();
  int count = windows.size();

  // Create an array of booleans (size = window count).
  // Cross off every number found in the window list.
  bool *checkArray = new bool[count];
  for (int nX = 0; nX < count; ++nX)
  {
    if (windows[nX].Cast<WorkbenchWindow> ().IsNotNull())
    {
      WorkbenchWindow::Pointer ww = windows[nX].Cast<WorkbenchWindow> ();
      int index = ww->GetNumber() - 1;
      if (index >= 0 && index < count)
      {
        checkArray[index] = true;
      }
    }
  }

  // Return first index which is not used.
  // If no empty index was found then every slot is full.
  // Return next index.
  for (int index = 0; index < count; index++)
  {
    if (!checkArray[index])
    {
      delete[] checkArray;
      return index + 1;
    }
  }

  delete[] checkArray;
  return count + 1;
}

IWorkbenchWindow::Pointer Workbench::BusyOpenWorkbenchWindow(
    const std::string& perspID, IAdaptable* input)
{
  // Create a workbench window (becomes active window)
  //final WorkbenchWindow newWindowArray[] = new WorkbenchWindow[1];
  //StartupThreading.runWithWorkbenchExceptions(new StartupRunnable() {
  //  public void runWithException() {
  //    newWindowArray[0] = newWorkbenchWindow();
  WorkbenchWindow::Pointer newWindow = this->NewWorkbenchWindow();
  //  }
  //});

  //final WorkbenchWindow newWindow = newWindowArray[0];

  //StartupThreading.runWithoutExceptions(new StartupRunnable() {

  //  public void runWithException() {
      newWindow->Create(); // must be created before adding to window
  // manager
  //  }
  //});
  windowManager.Add(newWindow);

  //final WorkbenchException [] exceptions = new WorkbenchException[1];
  // Create the initial page.
  if (perspID != "")
  {
    //StartupThreading.runWithWorkbenchExceptions(new StartupRunnable() {

    try
    {
      newWindow->BusyOpenPage(perspID, input);
    } catch (WorkbenchException& e)
    {
      windowManager.Remove(newWindow);
      throw e;
    }
  }

  // Open window after opening page, to avoid flicker.
  //StartupThreading.runWithWorkbenchExceptions(new StartupRunnable() {

  //  public void runWithException() {
  newWindow->Open();
  //  }
  //});

  return newWindow;
}

void Workbench::AddWorkbenchListener(IWorkbenchListener::Pointer listener)
{
  workbenchEvents.AddListener(listener);
}

void Workbench::RemoveWorkbenchListener(IWorkbenchListener::Pointer listener)
{
  workbenchEvents.RemoveListener(listener);
}

IWorkbenchListener::Events& Workbench::GetWorkbenchEvents()
{
  return workbenchEvents;
}

void Workbench::AddWindowListener(IWindowListener::Pointer l)
{
  windowEvents.AddListener(l);
}


void Workbench::RemoveWindowListener(IWindowListener::Pointer l)
{
  windowEvents.RemoveListener(l);
}

IWindowListener::Events& Workbench::GetWindowEvents()
{
  return windowEvents;
}

bool Workbench::FirePreShutdown(bool forced) {

  //SafeRunnable.run(new SafeRunnable() {
  //  public void run() {
  typedef IWorkbenchListener::Events::PreShutdownEvent::ListenerList ListenerList;
  const ListenerList& listeners = workbenchEvents.preShutdown.GetListeners();
  for ( ListenerList::const_iterator iter = listeners.begin();
              iter != listeners.end(); ++iter )
  {
    // notify each listener
    if (! (*iter)->Execute(dynamic_cast<IWorkbench*>(this), forced))
      return false;
  }
  //  }

  return true;
}

/**
 * Fire workbench postShutdown event.
 *
 * @since 3.2
 */
void Workbench::FirePostShutdown() {

  //  SafeRunnable.run(new SafeRunnable() {
  //    public void run() {
        workbenchEvents.postShutdown(this);
  //    }

}

void Workbench::FireWindowOpened(IWorkbenchWindow::Pointer window) {

   // SafeRunner.run(new SafeRunnable() {
   //   public void run() {
        windowEvents.windowOpened(window);
   //   }

}

void Workbench::FireWindowClosed(IWorkbenchWindow::Pointer window) {
  if (activatedWindow == window) {
    // Do not hang onto it so it can be GC'ed
    activatedWindow = 0;
  }


  //  SafeRunner.run(new SafeRunnable() {
  //    public void run() {
        windowEvents.windowClosed(window);
  //    }

}

void Workbench::FireWindowActivated(IWorkbenchWindow::Pointer window) {

  //  SafeRunner.run(new SafeRunnable() {
  //    public void run() {
        windowEvents.windowActivated(window);
  //    }

}

void Workbench::FireWindowDeactivated(IWorkbenchWindow::Pointer window) {

  //  SafeRunner.run(new SafeRunnable() {
  //    public void run() {
        windowEvents.windowDeactivated(window);
  //    }

}

IWorkbenchWindow::Pointer Workbench::RestoreWorkbenchWindow(IMemento::Pointer memento)
{
  WorkbenchWindow::Pointer newWindow = this->NewWorkbenchWindow();
  //newWindow.create();

  windowManager.Add(newWindow);

  // whether the window was opened
  bool opened = false;

  try
  {
    newWindow->RestoreState(memento, 0);
    newWindow->FireWindowRestored();
    newWindow->Open();
    opened = true;
  } catch (...)
  {
    if (!opened)
    {
      newWindow->Close();
    }
  }

  return newWindow;
}

void Workbench::Shutdown() {
  // shutdown application-specific portions first
  advisor->PostShutdown();

  // notify regular workbench clients of shutdown, and clear the list when
  // done
  this->FirePostShutdown();
  //workbenchListeners.clear();

  //cancelEarlyStartup();

  // for dynamic UI
//  Platform.getExtensionRegistry().removeRegistryChangeListener(
//      extensionEventHandler);
//  Platform.getExtensionRegistry().removeRegistryChangeListener(
//      startupRegistryListener);

 // ((GrabFocus) Tweaklets.get(GrabFocus.KEY)).dispose();

  // Bring down all of the services.
 // serviceLocator.dispose();

 // workbenchActivitySupport.dispose();
 // WorkbenchHelpSystem.disposeIfNecessary();

  // shutdown the rest of the workbench
 // WorkbenchColors.shutdown();
 // activityHelper.shutdown();
 // uninitializeImages();
//  if (WorkbenchPlugin.getDefault() != null) {
//    WorkbenchPlugin.getDefault().reset();
//  }
//  WorkbenchThemeManager.getInstance().dispose();
//  PropertyPageContributorManager.getManager().dispose();
//  ObjectActionContributorManager.getManager().dispose();
//  if (tracker != null) {
//    tracker.close();
//  }
}

}
