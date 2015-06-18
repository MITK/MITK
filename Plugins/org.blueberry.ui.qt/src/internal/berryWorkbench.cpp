/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryLog.h"

#include "tweaklets/berryWorkbenchTweaklet.h"

#include "berryWorkbench.h"

#include <berrySafeRunner.h>

#include "berryPolicy.h"
#include "berrySaveablesList.h"
#include "berryViewRegistry.h"
#include "berryEditorRegistry.h"
#include "berryEditorHistory.h"
#include "berryEditorHistoryItem.h"
#include "berryServiceLocatorCreator.h"
#include "berryWorkbenchPage.h"
#include "berryPerspective.h"
#include "berryPreferenceConstants.h"
#include "berryUIExtensionTracker.h"
#include "berryWorkbenchWindow.h"
#include "berryDisplay.h"
#include "services/berryIServiceFactory.h"
#include "util/berrySafeRunnable.h"

#include "berryWorkbenchServiceRegistry.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchConstants.h"
#include "berryWorkbenchMenuService.h"
#include "berryEvaluationService.h"
#include "berryCommandService.h"
#include "berryCommandManager.h"
#include "berryMenuManager.h"
#include "berryParameterType.h"
#include "berryQActionProperties.h"
#include "berrySourceProviderService.h"
#include "berryWorkbenchLocationService.h"

#include <berryCommand.h>
#include <berryCommandCategory.h>
#include <berryIElementFactory.h>
#include <berryIHandler.h>
#include <berryIHandlerService.h>
#include <berryIPreferencesService.h>
#include <berryIPreferences.h>
#include <berryIProduct.h>
#include <berryISourceProvider.h>
#include <berryIServiceScopes.h>

#include <QDir>
#include <QApplication>
#include <QMessageBox>

#include <Poco/FileStream.h>

namespace berry
{

Workbench* Workbench::instance = nullptr;
WorkbenchTestable::Pointer Workbench::testableObject;

const unsigned int Workbench::VERSION_STRING_COUNT = 1;
const QString Workbench::VERSION_STRING[Workbench::VERSION_STRING_COUNT] =
{ "1.0" };

const QString Workbench::DEFAULT_WORKBENCH_STATE_FILENAME = "workbench.xml";

class RestoreStateRunnable: public SafeRunnable
{

private:

  Workbench* workbench;
  Poco::File stateFile;
  bool& result;

public:

  RestoreStateRunnable(Workbench* workbench, const QString& stateFile,
      bool& result) :
    SafeRunnable(
        "Unable to read workbench state. Workbench UI layout will be reset."),
        workbench(workbench), stateFile(stateFile.toStdString()), result(result)
  {

  }

  void Run() override
  {
    Poco::FileInputStream input(stateFile.path());
    IMemento::Pointer memento = XMLMemento::CreateReadRoot(input);

    // Validate known version format
    QString version;
    memento->GetString(WorkbenchConstants::TAG_VERSION, version);
    bool valid = false;
    for (auto & elem : Workbench::VERSION_STRING)
    {
      if (elem == version)
      {
        valid = true;
        break;
      }
    }
    if (!valid)
    {
      input.close();
      QString msg =
          "Invalid workbench state version. workbench.xml will be deleted";
      QMessageBox::critical(nullptr, "Restoring Problems", msg);
      stateFile.remove();
      //          result[0] = new Status(IStatus.ERROR,
      //              WorkbenchPlugin.PI_WORKBENCH,
      //              IWorkbenchConfigurer.RESTORE_CODE_RESET, msg, null);
      result = false;
      return;
    }

    //    // Validate compatible version format
    //    // We no longer support the release 1.0 format
    //    if (VERSION_STRING[0].equals(version))
    //    {
    //      reader.close();
    //      QString msg = "The saved user interface layout is in an "
    //          "obsolete format and cannot be preserved. Your projects and files "
    //          "will not be affected. Press OK to convert to the new format. Press "
    //          "Cancel to exit with no changes.";
    //      QList<QString> dlgLabels;
    //      dlgLabels.push_back("Ok");
    //      dlgLabels.push_back("Cancel");
    //      IDialog::Pointer dlg = MessageDialog::CreateDialog(Shell::Pointer(0),
    //          "Cannot Preserve Layout", 0, msg, IDialog::WARNING, dlgLabels, 0);
    //      IDialog::ReturnCode ignoreSavedState = dlg->Open();
    //      // OK is the default
    //      if (ignoreSavedState == IDialog::OK)
    //      {
    //        stateFile.remove();
    //        //            result[0] = new Status(IStatus.WARNING,
    //        //                WorkbenchPlugin.PI_WORKBENCH,
    //        //                IWorkbenchConfigurer.RESTORE_CODE_RESET, msg,
    //        //                null);
    //        result = false;
    //      }
    //      else
    //      {
    //        //            result[0] = new Status(IStatus.WARNING,
    //        //                WorkbenchPlugin.PI_WORKBENCH,
    //        //                IWorkbenchConfigurer.RESTORE_CODE_EXIT, msg,
    //        //                null);
    //        result = false;
    //      }
    //      return;
    //    }

    // Restore the saved state
    //final IStatus restoreResult = restoreState(memento);
    /*bool restoreResult =*/ workbench->RestoreState(memento);
    input.close();
    //        if (restoreResult.getSeverity() == IStatus.ERROR) {
    //          StartupThreading
    //              .runWithoutExceptions(new StartupRunnable() {
    //
    //                public void runWithException() throws Throwable {
    //                  StatusManager.getManager().handle(restoreResult, StatusManager.LOG);
    //                }
    //              });
    //
    //        }
  }

  void HandleException(const ctkException& e) override
  {
    //StartupThreading.runWithoutExceptions(new StartupRunnable() {

    //public void runWithException() {
    Handle(e);
    //            QString msg = e.getMessage() == null ? "" : e.getMessage(); //$NON-NLS-1$
    //            result[0] = new Status(IStatus.ERROR,
    //                WorkbenchPlugin.PI_WORKBENCH,
    //                IWorkbenchConfigurer.RESTORE_CODE_RESET, msg, e);
    result = false;
    stateFile.remove();
    //  }});
  }

private:

  void Handle(const ctkException& e)
  {
    SafeRunnable::HandleException(e);
  }
};


int Workbench::CreateAndRunWorkbench(Display* display,
    WorkbenchAdvisor* advisor)
{
  // create the workbench instance
  Workbench workbench(display, advisor);
  // run the workbench event loop
  int returnCode = workbench.RunUI();

  return returnCode;
}

Display* Workbench::CreateDisplay()
{

  // create the display
  Display* newDisplay = Tweaklets::Get(WorkbenchTweaklet::KEY)->CreateDisplay();

  // workaround for 1GEZ9UR and 1GF07HN
  //newDisplay.setWarnings(false);

  // Set the priority higher than normal so as to be higher
  // than the JobManager.
  //Poco::Thread::current()->setPriority(Poco::Thread::PRIO_HIGH);

  //initializeImages();

  return newDisplay;
}

Workbench::ServiceLocatorOwner::ServiceLocatorOwner(Workbench* wb) :
  workbench(wb)
{

}

void Workbench::ServiceLocatorOwner::Dispose()
{
  QMessageBox::information(
        nullptr,
        "Restart needed",
        "A required plug-in is no longer available and the Workbench needs "
        "to be restarted. You will be prompted to save if there is any unsaved work.");
  workbench->Close(PlatformUI::RETURN_RESTART, true);
}

Workbench::Workbench(Display* display, WorkbenchAdvisor* advisor)
  : commandManager(nullptr), progressCount(-1)
  , serviceLocatorOwner(new ServiceLocatorOwner(this))
  , largeUpdates(0), introManager(nullptr), isStarting(true), isClosing(false)
  , activeWorkbenchWindow(nullptr)
{
  poco_check_ptr(display)
;  poco_check_ptr(advisor);
  // the reference count to the one and only workbench instance
  // is increased, so that temporary smart pointer to the workbench
  // do not delete it
  this->Register();

  this->display = display;
  this->advisor = advisor;
  Workbench::instance = this;

  serviceLocatorCreator.reset(new ServiceLocatorCreator());
  serviceLocatorCreator->Register();
  this->serviceLocator = serviceLocatorCreator->CreateServiceLocator(
        nullptr,
        nullptr,
        IDisposable::WeakPtr(serviceLocatorOwner)).Cast<ServiceLocator>();
  serviceLocator->RegisterService(serviceLocatorCreator.data());

  workbenchLocationService.reset(
        new WorkbenchLocationService(IServiceScopes::WORKBENCH_SCOPE, this, nullptr, nullptr, 0));
  workbenchLocationService->Register();
  serviceLocator->RegisterService(workbenchLocationService.data());

  returnCode = PlatformUI::RETURN_UNSTARTABLE;
}

Display* Workbench::GetDisplay() const
{
  return display;
}

Workbench* Workbench::GetInstance()
{
  return instance;
}

WorkbenchTestable::Pointer Workbench::GetWorkbenchTestable()
{
  if (!testableObject)
  {
    testableObject = new WorkbenchTestable();
  }
  return testableObject;
}

Workbench::~Workbench()
{
  this->instance = nullptr;
  this->UnRegister(false);
}

Object* Workbench::GetService(const QString& key)
{
  return serviceLocator->GetService(key);
}

bool Workbench::HasService(const QString& key) const
{
  return serviceLocator->HasService(key);
}

bool Workbench::Init()
{

//  // setup debug mode if required.
//  if (WorkbenchPlugin.getDefault().isDebugging()) {
//    WorkbenchPlugin.DEBUG = true;
//    ModalContext.setDebugMode(true);
//  }

  bool bail = false;

  // create workbench window manager
  //windowManager = new WindowManager();

  IIntroRegistry* introRegistry = WorkbenchPlugin::GetDefault()->GetIntroRegistry();
  if (introRegistry->GetIntroCount() > 0)
  {
    IProduct::Pointer product = Platform::GetProduct();
    if (product.IsNotNull())
    {
      introDescriptor = introRegistry->GetIntroForProduct(product->GetId()).Cast<IntroDescriptor>();
    }
  }

  // TODO Correctly order service initialization
  // there needs to be some serious consideration given to
  // the services, and hooking them up in the correct order
  evaluationService.reset(new EvaluationService());
  evaluationService->Register();

  //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //      public void runWithException() {
  serviceLocator->RegisterService(evaluationService.data());
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

//  StartupThreading.runWithoutExceptions(new StartupRunnable() {

//        public void runWithException() {
  StartSourceProviders();
//        }
//      });

//      StartupThreading.runWithoutExceptions(new StartupRunnable() {

//        public void runWithException() {

//          activateWorkbenchContext();

//        }
//      });

//      StartupThreading.runWithoutExceptions(new StartupRunnable() {

//        public void runWithException() {

//          createApplicationMenu();
//        }
//      });

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

bool Workbench::RestoreState()
{
  //return false;
  if (!GetWorkbenchConfigurer()->GetSaveAndRestore())
  {
    //      QString msg = "This application does not save and restore previously saved state.";
    //      return new Status(IStatus.WARNING, WorkbenchPlugin.PI_WORKBENCH,
    //          IWorkbenchConfigurer.RESTORE_CODE_RESET, msg, null);
    return false;
  }
  // Read the workbench state file.
  QString stateFile = GetWorkbenchStateFile();
  // If there is no state file cause one to open.
  if (stateFile.isEmpty() || !QFile::exists(stateFile))
  {
    //      QString msg = "No previously saved state to restore.";
    //      return new Status(IStatus.WARNING, WorkbenchPlugin.PI_WORKBENCH,
    //          IWorkbenchConfigurer.RESTORE_CODE_RESET, msg, null);
    return false;
  }

  //    final IStatus result[] = { new Status(IStatus.OK,
  //        WorkbenchPlugin.PI_WORKBENCH, IStatus.OK, "", null) }; //$NON-NLS-1$
  bool result = true;
  ISafeRunnable::Pointer runnable(new RestoreStateRunnable(this, stateFile, result));
  SafeRunner::Run(runnable);
  // ensure at least one window was opened
  //if (result[0].isOK() && windowManager.getWindows().length == 0)
  if (result && windowManager.GetWindowCount() == 0)
  {
    QString msg = "No windows restored.";
    //    result[0] = new Status(IStatus.ERROR, WorkbenchPlugin.PI_WORKBENCH,
    //        IWorkbenchConfigurer.RESTORE_CODE_RESET, msg, null);
    result &= false;
  }
  return result;
}

bool Workbench::RestoreState(IMemento::Pointer memento)
{

  //    final MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID,
  //        IStatus.OK, WorkbenchMessages.Workbench_problemsRestoring, null);
  bool result = true;

  const bool showProgress = false;
  //TODO restore state progress
  //    final boolean showProgress = PrefUtil.getAPIPreferenceStore()
  //        .getBoolean(
  //            IWorkbenchPreferenceConstants.SHOW_PROGRESS_ON_STARTUP);

  try
  {
    /*
     * Restored windows will be set in the createdWindows field to be
     * used by the openWindowsAfterRestore() method
     */
    if (!showProgress)
    {
      DoRestoreState(memento, result);
    }
    else
    {
      // Retrieve how many plug-ins were loaded while restoring the
      // workbench
      int lastProgressCount = -1;
      memento->GetInteger(WorkbenchConstants::TAG_PROGRESS_COUNT, lastProgressCount);

      // If we don't know how many plug-ins were loaded last time,
      // assume we are loading half of the installed plug-ins.
      /*const std::size_t expectedProgressCount =*/
      std::max<std::size_t>(1,
          lastProgressCount == -1 ? WorkbenchPlugin::GetDefault()->GetBundleCount() / 2
          : lastProgressCount);

      //TODO restore state progress
      //        RunStartupWithProgress(expectedProgressCount, new Runnable() {
      //          public void Run() {
      //            DoRestoreState(memento, result);
      //          }
      //        });
    }
  }
  catch (...)
  {
    OpenWindowsAfterRestore();
    throw;
  }

  OpenWindowsAfterRestore();
  return result;
}

void Workbench::DoRestoreState(IMemento::Pointer memento, bool& status) // final MultiStatus status)

{
  IMemento::Pointer childMem;
  try
  {
    // UIStats.start(UIStats.RESTORE_WORKBENCH, "MRUList"); //$NON-NLS-1$
    IMemento::Pointer mruMemento = memento
    ->GetChild(WorkbenchConstants::TAG_MRU_LIST);
    if (mruMemento)
    {
      // TODO restore editor history
      //status.add(getEditorHistory().restoreState(mruMemento));
    }

    //UIStats.end(UIStats.RESTORE_WORKBENCH, this, "MRUList"); //$NON-NLS-1$
  }
  catch (...)
  {
    //UIStats.end(UIStats.RESTORE_WORKBENCH, this, "MRUList"); //$NON-NLS-1$
    throw;
  }

  // Restore advisor state.
  IMemento::Pointer advisorState = memento
  ->GetChild(WorkbenchConstants::TAG_WORKBENCH_ADVISOR);
  if (advisorState)
  {
    //status.add(getAdvisor().restoreState(advisorState));
    status &= GetAdvisor()->RestoreState(advisorState);
  }

  // Get the child windows.
  QList<IMemento::Pointer> children = memento
  ->GetChildren(WorkbenchConstants::TAG_WINDOW);

  createdWindows.clear();

  // Read the workbench windows.
  for (int i = 0; i < children.size(); i++)
  {
    childMem = children[i];
    WorkbenchWindow::Pointer newWindow;

    //StartupThreading.runWithoutExceptions(new StartupRunnable() {

    //  public void runWithException() {
    newWindow = NewWorkbenchWindow();
    newWindow->Create();
    //  }});
    createdWindows.push_back(newWindow);

    // allow the application to specify an initial perspective to open
    // @issue temporary workaround for ignoring initial perspective
    // String initialPerspectiveId =
    // getAdvisor().getInitialWindowPerspectiveId();
    // if (initialPerspectiveId != null) {
    // IPerspectiveDescriptor desc =
    // getPerspectiveRegistry().findPerspectiveWithId(initialPerspectiveId);
    // result.merge(newWindow.restoreState(childMem, desc));
    // }
    // add the window so that any work done in newWindow.restoreState
    // that relies on Workbench methods has windows to work with
    windowManager.Add(newWindow);

    // now that we've added it to the window manager we need to listen
    // for any exception that might hose us before we get a chance to
    // open it. If one occurs, remove the new window from the manager.
    // Assume that the new window is a phantom for now
    try
    {
      //status.merge(newWindow[0].restoreState(childMem, null));
      status &= newWindow->RestoreState(childMem, IPerspectiveDescriptor::Pointer(nullptr));
      try
      {
        newWindow->FireWindowRestored();
      }
      catch (const WorkbenchException& /*e*/)
      {
        //status.add(e.getStatus());
        status &= false;
      }
      // everything worked so far, don't close now
    }
    catch (...)
    {
      // null the window in newWindowHolder so that it won't be
      // opened later on
      createdWindows[i] = nullptr;
      //StartupThreading.runWithoutExceptions(new StartupRunnable() {

      //  public void runWithException() throws Throwable {
      newWindow->Close();
      //  }});

    }
  }
}

void Workbench::OpenWindowsAfterRestore()
{
  if (createdWindows.empty())
  {
    return;
  }
  // now open the windows (except the ones that were nulled because we
  // closed them above)
  for (int i = 0; i < createdWindows.size(); i++)
  {
    if (createdWindows[i])
    {
      WorkbenchWindow::Pointer myWindow = createdWindows[i];
      //StartupThreading.runWithoutExceptions(new StartupRunnable() {

      //  public void runWithException() throws Throwable {
      try
      {
        myWindow->Open();
      }
      catch (...)
      {
        myWindow->Close();
        throw;
      }
      //  }});
    }
  }
  createdWindows.clear();
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
  //
  //
  //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //      public void runWithException() {
  saveablesList.reset(new SaveablesList());
  saveablesList->Register();
  serviceLocator->RegisterService(saveablesList.data());
  //      }});
  //

  /*
   * Phase 1 of the initialization of commands. When this phase completes,
   * all the services and managers will exist, and be accessible via the
   * getService(Object) method.
   */
  //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //      public void runWithException() {
  Command::DEBUG_COMMAND_EXECUTION = Policy::DEBUG_COMMANDS();
  Command::DEBUG_HANDLERS = Policy::DEBUG_HANDLERS_VERBOSE();
  Command::DEBUG_HANDLERS_COMMAND_ID = Policy::DEBUG_HANDLERS_VERBOSE_COMMAND_ID();
  commandManager.reset(new CommandManager());
  //      }});
  //
  //    final CommandService [] commandService = new CommandService[1];
  //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //
  //      public void runWithException() {
  commandService.reset(new CommandService(commandManager.data()));
  commandService->Register();
  commandService->ReadRegistry();
  serviceLocator->RegisterService(commandService.data());
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

  auto   wms = new WorkbenchMenuService(serviceLocator.GetPointer());
  menuService.reset(wms);
  menuService->Register();

  serviceLocator->RegisterService(menuService.data());
  // the service must be registered before it is initialized - its
  // initialization uses the service locator to address a dependency on
  // the menu service
  //StartupThreading.runWithoutExceptions(new StartupRunnable() {
  //      public void runWithException() {
  wms->ReadRegistry();
  //      }});

  // the source providers are now initialized in phase 3, but source
  // priorities have to be set before handler initialization
  // StartupThreading.runWithoutExceptions(new StartupRunnable() {
  // public void runWithException() {
  InitializeSourcePriorities();
  // }
  // });

  /*
   * Phase 2 of the initialization of commands. This handles the creation
   * of wrappers for legacy APIs. By the time this phase completes, any
   * code trying to access commands through legacy APIs should work.
   */
  //IHandlerService* handlerService = NULL;
//      StartupThreading.runWithoutExceptions(new StartupRunnable() {

//        public void runWithException() {
  /*handlerService = */serviceLocator->GetService<IHandlerService>();
//        }
//      });
//      workbenchContextSupport = new WorkbenchContextSupport(this,
//          contextManager);
//      workbenchCommandSupport = new WorkbenchCommandSupport(bindingManager,
//          commandManager, contextManager, handlerService[0]);
//      initializeCommandResolver();

  this->AddWindowListener(this);
//      bindingManager.addBindingManagerListener(bindingManagerListener);

//      serviceLocator.registerService(ISelectionConversionService.class,
//          new SelectionConversionService());
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

  isStarting = false;

  BERRY_INFO << "BlueBerry Workbench ready";

  this->GetWorkbenchTestable()->Init(Display::GetDefault(), this);

  // spin event loop
  return display->RunEventLoop();
}

QString Workbench::GetDefaultPerspectiveId() const
{
  return this->GetAdvisor()->GetInitialWindowPerspectiveId();
}

IAdaptable* Workbench::GetDefaultPageInput() const
{
  return this->GetAdvisor()->GetDefaultPageInput();
}

QString Workbench::GetPresentationId() const
{
  if (factoryID != "")
  {
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

IElementFactory* Workbench::GetElementFactory(const QString& factoryId) const
{
  return WorkbenchPlugin::GetDefault()->GetElementFactory(factoryId);
}

void Workbench::UpdateTheme()
{
  WorkbenchPlugin::GetDefault()->GetPresentationFactory()->UpdateTheme();
}

void Workbench::LargeUpdateStart()
{
  if (largeUpdates++ == 0)
  {
    // TODO Consider whether these lines still need to be here.
    // workbenchCommandSupport.setProcessing(false);
    // workbenchContextSupport.setProcessing(false);

    QList<IWorkbenchWindow::Pointer> windows = this->GetWorkbenchWindows();
    for (int i = 0; i < windows.size(); i++)
    {
      IWorkbenchWindow::Pointer window = windows[i];
      if (window.Cast<WorkbenchWindow>() != 0)
      {
        window.Cast<WorkbenchWindow>()->LargeUpdateStart();
      }
    }
  }
}

void Workbench::LargeUpdateEnd()
{
  if (--largeUpdates == 0)
  {
    // TODO Consider whether these lines still need to be here.
    // workbenchCommandSupport.setProcessing(true);
    // workbenchContextSupport.setProcessing(true);

    // Perform window-specific blocking.
    QList<IWorkbenchWindow::Pointer> windows = this->GetWorkbenchWindows();
    for (int i = 0; i < windows.size(); i++)
    {
      IWorkbenchWindow::Pointer window = windows[i];
      if (window.Cast<WorkbenchWindow>() != 0)
      {
        window.Cast<WorkbenchWindow>()->LargeUpdateEnd();
      }
    }
  }
}

IExtensionTracker*Workbench::GetExtensionTracker() const
{
  if (tracker.isNull())
  {
    tracker.reset(new UIExtensionTracker(this->GetDisplay()));
  }
  return tracker.data();
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
  }
  catch (WorkbenchException& e)
  {
    // Don't use the window's shell as the dialog parent,
    // as the window is not open yet (bug 76724).
    //StartupThreading.runWithoutExceptions(new StartupRunnable() {

    //  public void runWithException() throws Throwable {
    //    ErrorDialog.openError(null,
    //        WorkbenchMessages.Problems_Opening_Page, e.getMessage(), e
    //            .getStatus());
    //  }});
    BERRY_ERROR << "Error: Problems opening page. " << e.what() << std::endl;
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

WorkbenchAdvisor* Workbench::GetAdvisor() const
{
  return advisor;
}

IViewRegistry* Workbench::GetViewRegistry() const
{
  return WorkbenchPlugin::GetDefault()->GetViewRegistry();
}

IEditorRegistry* Workbench::GetEditorRegistry() const
{
  return WorkbenchPlugin::GetDefault()->GetEditorRegistry();
}

EditorHistory* Workbench::GetEditorHistory() const
{
  if (editorHistory.isNull())
  {
    editorHistory.reset(new EditorHistory());
  }
  return editorHistory.data();
}

IPerspectiveRegistry* Workbench::GetPerspectiveRegistry() const
{
  return WorkbenchPlugin::GetDefault()->GetPerspectiveRegistry();
}

bool Workbench::Close()
{
  return this->Close(PlatformUI::RETURN_OK, false);
}

bool Workbench::Close(int returnCode, bool force)
{
  BERRY_INFO << "Closing workbench...";
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
bool Workbench::BusyClose(bool force)
{

  // notify the advisor of preShutdown and allow it to veto if not forced
  isClosing = advisor->PreShutdown();
  if (!force && !isClosing)
  {
    return false;
  }

  // notify regular workbench clients of preShutdown and allow them to
  // veto if not forced
  isClosing = this->FirePreShutdown(force);
  if (!force && !isClosing)
  {
    return false;
  }

  // save any open editors if they are dirty
  isClosing = this->SaveAllEditors(!force);
  if (!force && !isClosing)
  {
    return false;
  }

  bool closeEditors = !force && false; // false is the default for the not yet implemented preference below
  // && PrefUtil.getAPIPreferenceStore().getBoolean(
  //     IWorkbenchPreferenceConstants.CLOSE_EDITORS_ON_EXIT);
  if (closeEditors)
  {
    // SafeRunner.run(new SafeRunnable() {
    //  public void run() {
    QList<IWorkbenchWindow::Pointer> windows = this->GetWorkbenchWindows();
    for (int i = 0; i < windows.size(); i++)
    {
      IWorkbenchPage::Pointer page = windows[i]->GetActivePage();
      if (page)
      isClosing = isClosing && page->CloseAllEditors(false);
    }
    // }
    //});
    if (!force && !isClosing)
    {
      return false;
    }
  }

  if (this->GetWorkbenchConfigurer()->GetSaveAndRestore())
  {
    try
    {
      //    SafeRunner.run(new SafeRunnable() {
      //      public void run() {
      XMLMemento::Pointer mem = RecordWorkbenchState();
      // Save the IMemento to a file.
      SaveMementoToFile(mem);
      //      }
    }
    catch(const ctkException& e)
    {
      //      public void handleException(Throwable e) {
      QString message;
      if (e.what() == nullptr)
      {
        message = "An error has occurred. See error log for more details. Do you want to exit?";
      }
      else
      {
        message = QString("An error has occurred: ") + e.what() + ". See error log for more details. Do you want to exit?";
      }
      if (QMessageBox::question(nullptr, "Error", message) != QMessageBox::Yes)
      {
        isClosing = false;
      }
    }
    //      }
    //    });
  }
  if (!force && !isClosing)
  {
    return false;
  }

  //SafeRunner.run(new SafeRunnable(WorkbenchMessages.ErrorClosing) {
  //  public void run() {
  if (isClosing || force)
  {
    isClosing = windowManager.Close();
  }
  //  }
  //});

  if (!force && !isClosing)
  {
    return false;
  }

  this->Shutdown();

  display->ExitEventLoop(0);
  return true;
}

QString Workbench::GetWorkbenchStateFile() const
{
  QString path = WorkbenchPlugin::GetDefault()->GetDataLocation();
  if (path.isNull())
  {
    return QString();
  }
  return QDir::cleanPath(path + "/" + DEFAULT_WORKBENCH_STATE_FILENAME);
}

/*
 * Save the workbench UI in a persistence file.
 */
bool Workbench::SaveMementoToFile(XMLMemento::Pointer memento)
{
  // Save it to a file.
  // XXX: nobody currently checks the return value of this method.
  QString stateFile = GetWorkbenchStateFile();
  if (stateFile.isNull())
  {
    return false;
  }
  //BERRY_INFO << "Saving state to: " << stateFile.path() << std::endl;
  try
  {
    Poco::FileOutputStream stream(stateFile.toStdString());
    memento->Save(stream);
  }
  catch (const Poco::IOException& /*e*/)
  {
    QFile::remove(stateFile);
    QMessageBox::critical(nullptr,
                          "Saving Problems",
                          "Unable to store workbench state.");
    return false;
  }

  // Success !
  return true;
}

IWorkbenchWindow::Pointer Workbench::GetActiveWorkbenchWindow() const
{
  // Look for the window that was last known being
  // the active one
  WorkbenchWindow::Pointer win = this->GetActivatedWindow();
  return win;
}

std::size_t Workbench::GetWorkbenchWindowCount() const
{
  return windowManager.GetWindowCount();
}

QList<IWorkbenchWindow::Pointer> Workbench::GetWorkbenchWindows() const
{
  QList<Window::Pointer> windows = windowManager.GetWindows();
  QList<IWorkbenchWindow::Pointer> result;
  for (QList<Window::Pointer>::iterator iter = windows.begin();
      iter != windows.end(); ++iter)
  {
    result.push_back(iter->Cast<WorkbenchWindow>());
  }

  return result;
}

IWorkbenchWindow::Pointer Workbench::OpenWorkbenchWindow(
    const QString& perspID, IAdaptable* input)
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
  return this->OpenWorkbenchWindow(this->GetPerspectiveRegistry()
      ->GetDefaultPerspective(), input);
}

IWorkbenchPage::Pointer Workbench::ShowPerspective(
    const QString& perspectiveId, IWorkbenchWindow::Pointer window)
{
  // If the specified window has the requested perspective open, then the
  // window
  // is given focus and the perspective is shown. The page's input is
  // ignored.
  WorkbenchWindow::Pointer win = window.Cast<WorkbenchWindow> ();
  if (win)
  {
    IWorkbenchPage::Pointer page = win->GetActivePage();
    if (page)
    {
      QList<IPerspectiveDescriptor::Pointer> perspectives(page
          ->GetOpenPerspectives());
      for (int i = 0; i < perspectives.size(); i++)
      {
        IPerspectiveDescriptor::Pointer persp = perspectives[i];
        if (perspectiveId == persp->GetId())
        {
          win->MakeVisible();
          page->SetPerspective(persp);
          return page;
        }
      }
    }
  }

  // If another window that has the workspace root as input and the
  // requested
  // perpective open and active, then the window is given focus.
  IAdaptable* input = GetDefaultPageInput();
  QList<IWorkbenchWindow::Pointer> windows(GetWorkbenchWindows());
  for (int i = 0; i < windows.size(); i++)
  {
    win = windows[i].Cast<WorkbenchWindow>();
    if (window != win)
    {
      WorkbenchPage::Pointer page = win->GetActivePage().Cast<WorkbenchPage>();
      if (page)
      {
        bool inputSame = false;
        if (input == nullptr)
        {
          inputSame = (page->GetInput() == nullptr);
        }
        else
        {
          inputSame = input == page->GetInput();
        }
        if (inputSame)
        {
          Perspective::Pointer persp = page->GetActivePerspective();
          if (persp)
          {
            IPerspectiveDescriptor::Pointer desc = persp->GetDesc();
            if (desc)
            {
              if (perspectiveId == desc->GetId())
              {
                Shell::Pointer shell = win->GetShell();
                shell->Open();
                if (shell->GetMinimized())
                {
                  shell->SetMinimized(false);
                }
                return page;
              }
            }
          }
        }
      }
    }
  }

  // Otherwise the requested perspective is opened and shown in the
  // specified
  // window or in a new window depending on the current user preference
  // for opening
  // perspectives, and that window is given focus.
  win = window.Cast<WorkbenchWindow>();
  if (win)
  {
    IPreferencesService* store = WorkbenchPlugin::GetDefault()->GetPreferencesService();
    int mode = store->GetSystemPreferences()->GetInt(PreferenceConstants::OPEN_PERSP_MODE,
                                                     PreferenceConstants::OPM_ACTIVE_PAGE);
    IWorkbenchPage::Pointer page = win->GetActivePage();
    IPerspectiveDescriptor::Pointer persp;
    if (page)
    {
      persp = page->GetPerspective();
    }

    // Only open a new window if user preference is set and the window
    // has an active perspective.
    if (PreferenceConstants::OPM_NEW_WINDOW == mode && persp)
    {
      IWorkbenchWindow::Pointer newWindow = OpenWorkbenchWindow(perspectiveId,
          input);
      return newWindow->GetActivePage();
    }

    IPerspectiveDescriptor::Pointer desc = GetPerspectiveRegistry()
    ->FindPerspectiveWithId(perspectiveId);
    if (desc == 0)
    {
      throw WorkbenchException(
          "Unable to create perspective \"" + perspectiveId + "\". There is no corresponding perspective extension.");
    }
    win->GetShell()->Open();
    if (page == 0)
    {
      page = win->OpenPage(perspectiveId, input);
    }
    else
    {
      page->SetPerspective(desc);
    }
    return page;
  }

  // Just throw an exception....
  throw WorkbenchException("Problems opening perspective \"" +
      perspectiveId + "\"");
}

IWorkbenchPage::Pointer Workbench::ShowPerspective(
    const QString& /*perspectiveId*/,
    IWorkbenchWindow::Pointer /*window*/,
    IAdaptable* /*input*/)
{
  return IWorkbenchPage::Pointer(nullptr);
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

bool Workbench::SaveAllEditors(bool /*confirm*/)
{
  return true;
}

IIntroManager* Workbench::GetIntroManager() const
{
  return GetWorkbenchIntroManager();
}

WorkbenchIntroManager* Workbench::GetWorkbenchIntroManager() const
{
  if (introManager.isNull())
  {
    introManager.reset(new WorkbenchIntroManager(const_cast<Workbench*>(this)));
  }
  return introManager.data();
}

IntroDescriptor::Pointer Workbench::GetIntroDescriptor() const
{
  return introDescriptor;
}

void Workbench::SetIntroDescriptor(IntroDescriptor::Pointer descriptor)
{
  if (GetIntroManager()->GetIntro())
  {
    GetIntroManager()->CloseIntro(GetIntroManager()->GetIntro());
  }
  introDescriptor = descriptor;
}

bool Workbench::IsRunning() const
{
  return Tweaklets::Get(WorkbenchTweaklet::KEY)->IsRunning();
}

bool Workbench::IsStarting() const
{
  return isStarting;
}

bool Workbench::IsClosing() const
{
  return isClosing;
}

WorkbenchWindow::Pointer Workbench::GetActivatedWindow() const
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
  WorkbenchWindow::Pointer wbw(new WorkbenchWindow(this->GetNewWindowNumber()));
  return wbw;
}

int Workbench::GetNewWindowNumber()
{
  // Get window list.
  QList<Window::Pointer> windows = windowManager.GetWindows();
  int count = static_cast<int>(windows.size());

  // Create an array of booleans (size = window count).
  // Cross off every number found in the window list.
  auto  checkArray = new bool[count];
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
  return static_cast<int>(count + 1);
}

IWorkbenchWindow::Pointer Workbench::BusyOpenWorkbenchWindow(
    const QString& perspID, IAdaptable* input)
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
    }
    catch (WorkbenchException& e)
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

bool Workbench::SaveState(IMemento::Pointer memento)
{
  //  MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID, IStatus.OK,
  //      WorkbenchMessages.Workbench_problemsSaving, null);
  bool result = true;

  // Save the version number.
  memento->PutString(WorkbenchConstants::TAG_VERSION, VERSION_STRING[0]);

  // Save how many plug-ins were loaded while restoring the workbench
  if (progressCount != -1)
  {
    memento->PutInteger(WorkbenchConstants::TAG_PROGRESS_COUNT,
        progressCount);
  }

  // Save the advisor state.
  IMemento::Pointer advisorState = memento
  ->CreateChild(WorkbenchConstants::TAG_WORKBENCH_ADVISOR);
  //result.add(getAdvisor().saveState(advisorState));
  result &= GetAdvisor()->SaveState(advisorState);

  // Save the workbench windows.
  QList<IWorkbenchWindow::Pointer> windows(GetWorkbenchWindows());
  for (int nX = 0; nX < windows.size(); nX++)
  {
    WorkbenchWindow::Pointer window = windows[nX].Cast<WorkbenchWindow>();
    IMemento::Pointer childMem = memento->CreateChild(WorkbenchConstants::TAG_WINDOW);
    //result.merge(window.saveState(childMem));
    result &= window->SaveState(childMem);
  }
  //  result.add(getEditorHistory().saveState(
  //      memento.createChild(IWorkbenchConstants.TAG_MRU_LIST)));

  return result;
}

XMLMemento::Pointer Workbench::RecordWorkbenchState()
{
  XMLMemento::Pointer memento = XMLMemento
  ::CreateWriteRoot(WorkbenchConstants::TAG_WORKBENCH);
  //final IStatus status = saveState(memento);
  bool status = SaveState(memento);
  //if (status.getSeverity() != IStatus.OK) {
  if (!status)
  {
    //    // don't use newWindow as parent because it has not yet been opened
    //    // (bug 76724)
    //    StartupThreading.runWithoutExceptions(new StartupRunnable() {
    //
    //      public void runWithException() throws Throwable {
    //        ErrorDialog.openError(null,
    //            WorkbenchMessages.Workbench_problemsSaving,
    //            WorkbenchMessages.Workbench_problemsSavingMsg, status);
    //      }});

  }
  return memento;
}

void Workbench::AddWorkbenchListener(IWorkbenchListener* listener)
{
  workbenchEvents.AddListener(listener);
}

void Workbench::RemoveWorkbenchListener(IWorkbenchListener* listener)
{
  workbenchEvents.RemoveListener(listener);
}

IWorkbenchListener::Events& Workbench::GetWorkbenchEvents()
{
  return workbenchEvents;
}

void Workbench::AddWindowListener(IWindowListener* l)
{
  windowEvents.AddListener(l);
}

void Workbench::RemoveWindowListener(IWindowListener* l)
{
  windowEvents.RemoveListener(l);
}

IWindowListener::Events& Workbench::GetWindowEvents()
{
  return windowEvents;
}

bool Workbench::FirePreShutdown(bool forced)
{

  //SafeRunnable.run(new SafeRunnable() {
  //  public void run() {
  typedef IWorkbenchListener::Events::PreShutdownEvent::ListenerList ListenerList;
  const ListenerList& listeners = workbenchEvents.preShutdown.GetListeners();
  for ( auto iter = listeners.begin();
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
void Workbench::FirePostShutdown()
{

  //  SafeRunnable.run(new SafeRunnable() {
  //    public void run() {
  workbenchEvents.postShutdown(this);
  //    }

}

void Workbench::FireWindowOpened(IWorkbenchWindow::Pointer window)
{

  // SafeRunner.run(new SafeRunnable() {
  //   public void run() {
  windowEvents.windowOpened(window);
  //   }

}

void Workbench::FireWindowClosed(IWorkbenchWindow::Pointer window)
{
  if (activatedWindow == window)
  {
    // Do not hang onto it so it can be GC'ed
    activatedWindow = nullptr;
  }

  //  SafeRunner.run(new SafeRunnable() {
  //    public void run() {
  windowEvents.windowClosed(window);
  //    }

}

void Workbench::FireWindowActivated(IWorkbenchWindow::Pointer window)
{

  //  SafeRunner.run(new SafeRunnable() {
  //    public void run() {
  windowEvents.windowActivated(window);
  //    }

}

void Workbench::FireWindowDeactivated(IWorkbenchWindow::Pointer window)
{

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
    newWindow->RestoreState(memento, IPerspectiveDescriptor::Pointer(nullptr));
    newWindow->FireWindowRestored();
    newWindow->Open();
    opened = true;
  }
  catch (...)
  {
    if (!opened)
    {
      newWindow->Close();
    }
  }

  return newWindow;
}

void Workbench::Shutdown()
{
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
  serviceLocator->Dispose();

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

  Tweaklets::Clear();
}

void Workbench::InitializeSourcePriorities()
{
  WorkbenchServiceRegistry::GetRegistry()->InitializeSourcePriorities();
}

void Workbench::StartSourceProviders()
{
  /*
   * Phase 3 of the initialization of commands. The source providers that
   * the workbench provides are creating and registered with the above
   * services. These source providers notify the services when particular
   * pieces of workbench state change.
   */
  IEvaluationService* const evaluationService = serviceLocator->GetService<IEvaluationService>();
  //IContextService* const contextService = serviceLocator->GetService<IContextService>();

  auto   sps = new SourceProviderService(serviceLocator.GetPointer());
  sourceProviderService.reset(sps);
  sourceProviderService->Register();
  serviceLocator->RegisterService<ISourceProviderService>(sourceProviderService.data());
  struct SafeSourceProviderRunnable : public ISafeRunnable {

    SafeSourceProviderRunnable(SourceProviderService* sps, IEvaluationService* es) : sps(sps), es(es)
    {}

    void Run() override {
      // this currently instantiates all players ... sigh
      sps->ReadRegistry();
      QList<ISourceProvider::Pointer> sp = sps->GetSourceProviders();
      for (int i = 0; i < sp.size(); i++)
      {
        es->AddSourceProvider(sp[i]);
        //if (!(sp[i] instanceof ActiveContextSourceProvider))
        //{
        //  contextService.addSourceProvider(sp[i]);
        //}
      }
    }

    void HandleException(const ctkException& exception) override
    {
      WorkbenchPlugin::Log("Failed to initialize a source provider", exception);
    }

  private:
    SourceProviderService* sps;
    IEvaluationService* es;
  };
  ISafeRunnable::Pointer sourceProviderRunnable(
        new SafeSourceProviderRunnable(sps, evaluationService));
  SafeRunner::Run(sourceProviderRunnable);

//    SafeRunner.run(new ISafeRunnable() {
//      public void run() throws Exception {
//        // these guys are need to provide the variables they say
//        // they source
//        actionSetSourceProvider = (ActionSetSourceProvider) sourceProviderService
//            .getSourceProvider(ISources.ACTIVE_ACTION_SETS_NAME);

//        FocusControlSourceProvider focusControl = (FocusControlSourceProvider) sourceProviderService
//            .getSourceProvider(ISources.ACTIVE_FOCUS_CONTROL_ID_NAME);
//        serviceLocator.registerService(IFocusService.class, focusControl);

//        menuSourceProvider = (MenuSourceProvider) sourceProviderService
//            .getSourceProvider(ISources.ACTIVE_MENU_NAME);
//      }

//      public void handleException(Throwable exception) {
//        WorkbenchPlugin.log("Failed to initialize a source provider", exception); //$NON-NLS-1$
//      }
//    });
}

void Workbench::UpdateActiveWorkbenchWindowMenuManager(bool textOnly)
{
  if (activeWorkbenchWindow != nullptr)
  {
//    if (actionSetSourceProvider != null)
//    {
//      activeWorkbenchWindow->RemoveActionSetsListener(actionSetSourceProvider);
//    }
    activeWorkbenchWindow = nullptr;
  }
  //bool actionSetsUpdated = false;

  IWorkbenchWindow::Pointer workbenchWindow = GetActiveWorkbenchWindow();

  if (WorkbenchWindow::Pointer wbWnd = workbenchWindow.Cast<WorkbenchWindow>())
  {
    activeWorkbenchWindow = wbWnd.GetPointer();
    if (activeWorkbenchWindow->IsClosing())
    {
      return;
    }

    // Update the action sets.
//    Shell::Pointer windowShell = activeWorkbenchWindow->GetShell();
//    Shell::Pointer activeShell = GetDisplay()->GetActiveShell();
//    IContextService* service = GetService<IContextService>();
//    if ((Util.equals(windowShell, activeShell) || service.getShellType(activeShell) == IContextService.TYPE_WINDOW)
//        && actionSetSourceProvider != null)
//    {
//      activeWorkbenchWindow->AddActionSetsListener(actionSetSourceProvider);
//      final WorkbenchPage page = activeWorkbenchWindow.getActiveWorkbenchPage();
//      final IActionSetDescriptor[] newActionSets;
//      if (page != null)
//      {
//        newActionSets = page.getActionSets();
//        final ActionSetsEvent event = new ActionSetsEvent(newActionSets);
//        actionSetSourceProvider.actionSetsChanged(event);
//        actionSetsUpdated = true;
//      }
//    }

    MenuManager* menuManager = activeWorkbenchWindow->GetMenuManager();

    if (textOnly)
    {
      menuManager->Update(QActionProperties::TEXT);
    }
    else
    {
      menuManager->Update(true);
    }
  }

//  if (!actionSetsUpdated && actionSetSourceProvider != null)
//  {
//    ActionSetsEvent event = new ActionSetsEvent(null);
//    actionSetSourceProvider.actionSetsChanged(event);
  //  }
}

void Workbench::WindowActivated(const IWorkbenchWindow::Pointer&)
{
  this->UpdateActiveWorkbenchWindowMenuManager(true);
}

void Workbench::WindowDeactivated(const IWorkbenchWindow::Pointer&)
{
  this->UpdateActiveWorkbenchWindowMenuManager(true);
}

void Workbench::WindowClosed(const IWorkbenchWindow::Pointer&)
{
  this->UpdateActiveWorkbenchWindowMenuManager(true);
}

void Workbench::WindowOpened(const IWorkbenchWindow::Pointer&)
{
  this->UpdateActiveWorkbenchWindowMenuManager(true);
}

}
