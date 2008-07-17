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

#include "cherryViewRegistry.h"
#include "cherryEditorRegistry.h"

#include "cherryWorkbenchPlugin.h"

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

Workbench::Workbench(WorkbenchAdvisor* advisor)
{
  poco_check_ptr(advisor);

  this->advisor = advisor;
  Workbench::instance = this;

  returnCode = PlatformUI::RETURN_UNSTARTABLE;
}

Workbench* Workbench::GetInstance()
{
  return instance;
}

Workbench::~Workbench()
{

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

  //    initializeDefaultServices();
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

IAdaptable* Workbench::GetDefaultPageInput()
{
  return this->GetAdvisor()->GetDefaultPageInput();
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

    //TODO perspective
    //this->BusyOpenWorkbenchWindow(getPerspectiveRegistry()
    //    .getDefaultPerspective(), input[0]);
    this->BusyOpenWorkbenchWindow("bla", input);
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
    std::cout << "Error: Problems opening page. " << e.what() << std::endl;
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
  return windowManager.GetWindows();
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
  WorkbenchWindow::Pointer wbw =
      Tweaklets::Get(WorkbenchTweaklet::KEY)->CreateWorkbenchWindow(
          this->GetNewWindowNumber());
  wbw->Init();
  return wbw;
}

int Workbench::GetNewWindowNumber()
{
  // Get window list.
  std::vector<IWorkbenchWindow::Pointer> windows = windowManager.GetWindows();
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
  //    newWindow.create(); // must be created before adding to window
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
    } catch (WorkbenchException e)
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
  workbenchEvents.postShutdown.AddListener(listener.GetPointer(),
      &IWorkbenchListener::PostShutdown);
  workbenchEvents.preShutdown.AddListener(listener.GetPointer(),
        &IWorkbenchListener::PreShutdown);
}

void Workbench::RemoveWorkbenchListener(IWorkbenchListener::Pointer listener)
{
  workbenchEvents.postShutdown.RemoveListener(listener.GetPointer(),
      &IWorkbenchListener::PostShutdown);
  workbenchEvents.preShutdown.RemoveListener(listener.GetPointer(),
        &IWorkbenchListener::PreShutdown);
}

IWorkbench::WorkbenchEvents& Workbench::GetWorkbenchEvents()
{
  return workbenchEvents;
}

void Workbench::AddWindowListener(IWindowListener::Pointer l)
{
  windowEvents.windowActivated.AddListener(l.GetPointer(),
      &IWindowListener::WindowActivated);
  windowEvents.windowDeactivated.AddListener(l.GetPointer(),
        &IWindowListener::WindowDeactivated);
  windowEvents.windowClosed.AddListener(l.GetPointer(),
        &IWindowListener::WindowClosed);
  windowEvents.windowOpened.AddListener(l.GetPointer(),
        &IWindowListener::WindowOpened);
}


void Workbench::RemoveWindowListener(IWindowListener::Pointer l)
{
  windowEvents.windowActivated.RemoveListener(l.GetPointer(),
        &IWindowListener::WindowActivated);
    windowEvents.windowDeactivated.RemoveListener(l.GetPointer(),
          &IWindowListener::WindowDeactivated);
    windowEvents.windowClosed.RemoveListener(l.GetPointer(),
          &IWindowListener::WindowClosed);
    windowEvents.windowOpened.RemoveListener(l.GetPointer(),
          &IWindowListener::WindowOpened);
}

IWorkbench::WindowEvents& Workbench::GetWindowEvents()
{
  return windowEvents;
}

bool Workbench::FirePreShutdown(bool forced) {

  //SafeRunnable.run(new SafeRunnable() {
  //  public void run() {
  typedef WorkbenchEvents::PreShutdownEvent::ListenerList ListenerList;
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
