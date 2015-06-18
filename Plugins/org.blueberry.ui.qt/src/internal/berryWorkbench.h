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

#ifndef BERRYWORKBENCH_H_
#define BERRYWORKBENCH_H_

#include "berryIViewPart.h"
#include "berryIWorkbench.h"
#include "berryWorkbenchWindow.h"
#include "berryIWorkbenchPage.h"
#include "berryIWorkbenchPartReference.h"
#include "berryXMLMemento.h"

#include "berryPartPane.h"
#include "berryEditorAreaHelper.h"
#include "berryWindowManager.h"
#include "berryWorkbenchConfigurer.h"
#include "application/berryWorkbenchAdvisor.h"
#include "berryWorkbenchTestable.h"
#include "intro/berryIntroDescriptor.h"
#include "intro/berryWorkbenchIntroManager.h"

#include "berryILayoutContainer.h"

#include "berryServiceLocator.h"

#include <Poco/Exception.h>
#include <Poco/File.h>

namespace berry {

struct ICommandService;
struct IEvaluationService;
struct IMenuService;
struct IServiceLocatorCreator;
struct ISaveablesLifecycleListener;
struct ISourceProviderService;
struct IWorkbenchLocationService;

class CommandManager;
class EditorHistory;
class ViewRegistry;
class EditorRegistry;
class WorkbenchWindowConfigurer;

/**
 * \ingroup org_blueberry_ui_qt
 *
 * The workbench class represents the top of the BlueBerry user interface. Its
 * primary responsibility is the management of workbench windows, dialogs,
 * wizards, and other workbench-related windows.
 * <p>
 * Note that any code that is run during the creation of a workbench instance
 * should not required access to the display.
 */
class BERRY_UI_QT Workbench : public IWorkbench, private IWindowListener
{

public:

  berryObjectMacro(Workbench)

  friend class RestoreStateRunnable;

  /**
   * Creates the workbench and associates it with the the given display and
   * workbench advisor, and runs the workbench UI. This entails processing and
   * dispatching events until the workbench is closed or restarted.
   * <p>
   * This method is intended to be called by <code>PlatformUI</code>. Fails
   * if the workbench UI has already been created.
   * </p>
   * <p>
   * The display passed in must be the default display.
   * </p>
   *
   * @param display
   *            the display to be used for all UI interactions with the
   *            workbench
   * @param advisor
   *            the application-specific advisor that configures and
   *            specializes the workbench
   * @return return code {@link PlatformUI#RETURN_OK RETURN_OK}for normal
   *         exit; {@link PlatformUI#RETURN_RESTART RETURN_RESTART}if the
   *         workbench was terminated with a call to
   *         {@link IWorkbench#restart IWorkbench.restart}; other values
   *         reserved for future use
   */
  static int CreateAndRunWorkbench(Display* display, WorkbenchAdvisor* advisor);

  /**
   * Creates the <code>Display</code> to be used by the workbench.
   *
   * @return the display
   */
  static Display* CreateDisplay();

  /**
   * Returns the one and only instance of the workbench, if there is one.
   *
   * @return the workbench, or <code>null</code> if the workbench has not
   *         been created, or has been created and already completed
   */
  static Workbench* GetInstance();

  virtual ~Workbench();

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.services.IServiceLocator#getService(java.lang.Object)
   */
  Object* GetService(const QString& key) override;

  /*
   * (non-Javadoc)
   *
   * @see org.blueberry.ui.services.IServiceLocator#hasService(java.lang.Object)
   */
  bool HasService(const QString& key) const override;

  /*
   * Method declared on IWorkbench.
   */
  bool Close() override;

  /**
   * Returns the testable object facade, for use by the test harness.
   *
   * @return the testable object facade
   * @since 3.0
   */
  static WorkbenchTestable::Pointer GetWorkbenchTestable();

  /*
   *  Method declared on IWorkbench.
   */
  void AddWorkbenchListener(IWorkbenchListener* listener) override;

  /*
   * Method declared on IWorkbench.
   */
  void RemoveWorkbenchListener(IWorkbenchListener* listener) override;

  /*
   * Method declared on IWorkbench.
   */
  IWorkbenchListener::Events& GetWorkbenchEvents() override;

  /*
   * Method declared on IWorkbench.
   */
  void AddWindowListener(IWindowListener* l) override;

  /*
   * Method declared on IWorkbench.
   */
  void RemoveWindowListener(IWindowListener* l) override;

  /*
   * Method declared on IWorkbench.
   */
  IWindowListener::Events& GetWindowEvents() override;

  IWorkbenchWindow::Pointer GetActiveWorkbenchWindow() const override;

  IViewRegistry* GetViewRegistry() const override;
  IEditorRegistry* GetEditorRegistry() const override;
  EditorHistory* GetEditorHistory() const;
  IPerspectiveRegistry* GetPerspectiveRegistry() const override;

  std::size_t GetWorkbenchWindowCount() const override;
  QList<IWorkbenchWindow::Pointer> GetWorkbenchWindows() const override;
  IWorkbenchWindow::Pointer OpenWorkbenchWindow(const QString& perspectiveId,
        IAdaptable* input) override;
  IWorkbenchWindow::Pointer OpenWorkbenchWindow(IAdaptable* input) override;

  IWorkbenchPage::Pointer ShowPerspective(const QString& perspectiveId,
      IWorkbenchWindow::Pointer window) override;

  IWorkbenchPage::Pointer ShowPerspective(const QString& perspectiveId,
        IWorkbenchWindow::Pointer window, IAdaptable* input) override;

  bool SaveAllEditors(bool confirm) override;

  IIntroManager* GetIntroManager() const override;

  /**
   * @return the workbench intro manager
   */
  WorkbenchIntroManager* GetWorkbenchIntroManager() const;

  /**
   * @return the intro extension for this workbench.
   */
  IntroDescriptor::Pointer GetIntroDescriptor() const;

  /**
   * This method exists as a test hook. This method should <strong>NEVER</strong>
   * be called by clients.
   *
   * @param descriptor
   *            The intro descriptor to use.
   */
  void SetIntroDescriptor(IntroDescriptor::Pointer descriptor);

  /**
   * Returns <code>true</code> if the workbench is running,
   * <code>false</code> if it has been terminated.
   *
   * @return <code>true</code> if the workbench is running,
   *         <code>false</code> if it has been terminated.
   */
  bool IsRunning() const;

  /**
   * Returns true if the Workbench is in the process of starting.
   *
   * @return <code>true</code> if the Workbench is starting, but not yet
   *         running the event loop.
   */
  bool IsStarting() const;

  bool IsClosing() const override;

  /**
   * Returns the default perspective id, which may be <code>null</code>.
   *
   * @return the default perspective id, or <code>null</code>
   */
  QString GetDefaultPerspectiveId() const;

  /**
   * Returns the default workbench window page input.
   *
   * @return the default window page input or <code>null</code> if none
   */
  IAdaptable* GetDefaultPageInput() const;

  /**
   * Return the presentation ID specified by the preference or the default ID
   * if undefined.
   *
   * @return the presentation ID
   * @see IWorkbenchPreferenceConstants#PRESENTATION_FACTORY_ID
   */
  QString GetPresentationId() const;

  IElementFactory* GetElementFactory(const QString& factoryId) const override;

  void UpdateTheme() override;

  /**
   * <p>
   * Indicates the start of a large update within the workbench. This is used
   * to disable CPU-intensive, change-sensitive services that were temporarily
   * disabled in the midst of large changes. This method should always be
   * called in tandem with <code>largeUpdateEnd</code>, and the event loop
   * should not be allowed to spin before that method is called.
   * </p>
   * <p>
   * Important: always use with <code>largeUpdateEnd</code>!
   * </p>
   */
  void LargeUpdateStart();

  /**
   * <p>
   * Indicates the end of a large update within the workbench. This is used to
   * re-enable services that were temporarily disabled in the midst of large
   * changes. This method should always be called in tandem with
   * <code>largeUpdateStart</code>, and the event loop should not be
   * allowed to spin before this method is called.
   * </p>
   * <p>
   * Important: always protect this call by using <code>finally</code>!
   * </p>
   */
  void LargeUpdateEnd();

  IExtensionTracker* GetExtensionTracker() const override;

protected:

  friend class PlatformUI;
  friend class WorkbenchConfigurer;
  friend class WorkbenchWindowConfigurer;
  friend class WorkbenchWindow;
  friend struct WorkbenchWindow::ShellEventFilter;

  int RunUI();

  void OpenFirstTimeWindow();
  IWorkbenchWindow::Pointer RestoreWorkbenchWindow(IMemento::Pointer memento);

  bool Init();

  /*
   * Restores the workbench UI from the workbench state file (workbench.xml).
   *
   * @return a status object indicating OK if a window was opened,
   * RESTORE_CODE_RESET if no window was opened but one should be, and
   * RESTORE_CODE_EXIT if the workbench should close immediately
   */
  /* package */
  bool RestoreState();

  /**
   * Closes the workbench, returning the given return code from the run
   * method. If forced, the workbench is closed no matter what.
   *
   * @param returnCode
   *            {@link PlatformUI#RETURN_OK RETURN_OK}for normal exit;
   *            {@link PlatformUI#RETURN_RESTART RETURN_RESTART}if the
   *            workbench was terminated with a call to
   *            {@link IWorkbench#restart IWorkbench.restart};
   *            {@link PlatformUI#RETURN_EMERGENCY_CLOSE} for an emergency
   *            shutdown
   *            {@link PlatformUI#RETURN_UNSTARTABLE RETURN_UNSTARTABLE}if
   *            the workbench could not be started; other values reserved for
   *            future use
   *
   * @param force
   *            true to force the workbench close, and false for a "soft"
   *            close that can be canceled
   * @return true if the close was successful, and false if the close was
   *         canceled
   */
  /* package */
  bool Close(int returnCode, bool force);

  /**
   * Returns the unique object that applications use to configure the
   * workbench.
   * <p>
   * IMPORTANT This method is declared protected to prevent regular
   * plug-ins from downcasting IWorkbench to Workbench and getting hold of the
   * workbench configurer that would allow them to tamper with the workbench.
   * The workbench configurer is available only to the application.
   * </p>
   */
  WorkbenchConfigurer::Pointer GetWorkbenchConfigurer();

  /**
   * Returns the workbench advisor that created this workbench.
   * <p>
   * IMPORTANT This method is declared protected to prevent regular
   * plug-ins from downcasting IWorkbench to Workbench and getting hold of the
   * workbench advisor that would allow them to tamper with the workbench. The
   * workbench advisor is internal to the application.
   * </p>
   */
  WorkbenchAdvisor* GetAdvisor() const;

  /*
   * Returns the workbench window which was last known being the active one,
   * or <code> null </code> .
   */
  SmartPointer<WorkbenchWindow> GetActivatedWindow() const;

  /*
   * Sets the workbench window which was last known being the active one, or
   * <code> null </code> .
   */
  void SetActivatedWindow(SmartPointer<WorkbenchWindow> window);

  /**
   * Fire workbench preShutdown event, stopping at the first one to veto
   *
   * @param forced
   *            flag indicating whether the shutdown is being forced
   * @return <code>true</code> to allow the workbench to proceed with
   *         shutdown, <code>false</code> to veto a non-forced shutdown
   * @since 3.2
   */
  bool FirePreShutdown(bool forced);

  /**
   * Fire workbench postShutdown event.
   *
   * @since 3.2
   */
  void FirePostShutdown();

  /**
   * Fire window opened event.
   *
   * @param window
   *            The window which just opened; should not be <code>null</code>.
   */
  void FireWindowOpened(IWorkbenchWindow::Pointer window);

  /**
   * Fire window closed event.
   *
   * @param window
   *            The window which just closed; should not be <code>null</code>.
   */
  void FireWindowClosed(IWorkbenchWindow::Pointer window);

  /**
   * Fire window activated event.
   *
   * @param window
   *            The window which was just activated; should not be
   *            <code>null</code>.
   */
  void FireWindowActivated(IWorkbenchWindow::Pointer window);

  /**
   * Fire window deactivated event.
   *
   * @param window
   *            The window which was just deactivated; should not be
   *            <code>null</code>.
   */
  void FireWindowDeactivated(IWorkbenchWindow::Pointer window);


private:

  /**
   * Holds onto the only instance of Workbench.
   */
  static Workbench* instance;

  /**
   * The testable object facade.
   */
  static WorkbenchTestable::Pointer testableObject;

  static const unsigned int VERSION_STRING_COUNT; // = 1;
  static const QString VERSION_STRING[1];

  static const QString DEFAULT_WORKBENCH_STATE_FILENAME; // = "workbench.xml";

  IWorkbenchListener::Events workbenchEvents;
  IWindowListener::Events windowEvents;

  WorkbenchAdvisor* advisor;
  WorkbenchConfigurer::Pointer workbenchConfigurer;

  /**
   * The service locator maintained by the workbench. These services are
   * initialized during workbench during the <code>init</code> method.
   */
  ServiceLocator::Pointer serviceLocator;

  QScopedPointer<IEvaluationService, QScopedPointerObjectDeleter> evaluationService;

  QScopedPointer<ISaveablesLifecycleListener, QScopedPointerObjectDeleter> saveablesList;

  /**
   * The single instance of the command manager used by the workbench. This is
   * initialized in <code>Workbench.init(Display)</code> and then never
   * changed. This value will only be <code>null</code> if the
   * initialization call has not yet completed.
   */
  QScopedPointer<CommandManager> commandManager;

  QScopedPointer<ICommandService, QScopedPointerObjectDeleter> commandService;

  QScopedPointer<IMenuService, QScopedPointerObjectDeleter> menuService;

  mutable QScopedPointer<IExtensionTracker> tracker;

  /**
   * A count of how many plug-ins were loaded while restoring the workbench
   * state. Initially -1 for unknown number.
   */
  int progressCount;

  /**
   * A field to hold the workbench windows that have been restored. In the
   * event that not all windows have been restored, this field allows the
   * openWindowsAfterRestore method to open some windows.
   */
  QList<WorkbenchWindow::Pointer> createdWindows;

  struct ServiceLocatorOwner : public IDisposable
  {
    ServiceLocatorOwner(Workbench* workbench);

    void Dispose() override;

  private:
    Workbench* workbench;
  };

  friend struct ServiceLocatorOwner;

  IDisposable::Pointer serviceLocatorOwner;

  QScopedPointer<IServiceLocatorCreator, QScopedPointerObjectDeleter> serviceLocatorCreator;
  QScopedPointer<IWorkbenchLocationService, QScopedPointerObjectDeleter> workbenchLocationService;

  QScopedPointer<ISourceProviderService, QScopedPointerObjectDeleter> sourceProviderService;

  /**
   * A count of how many large updates are going on. This tracks nesting of
   * requests to disable services during a large update -- similar to the
   * <code>setRedraw</code> functionality on <code>Control</code>. When
   * this value becomes greater than zero, services are disabled. When this
   * value becomes zero, services are enabled. Please see
   * <code>largeUpdateStart()</code> and <code>largeUpdateEnd()</code>.
   */
  int largeUpdates;


  /**
   * The display used for all UI interactions with this workbench.
   */
  Display* display;

  WindowManager windowManager;
  SmartPointer<WorkbenchWindow> activatedWindow;

  mutable QScopedPointer<EditorHistory> editorHistory;

  mutable QScopedPointer<WorkbenchIntroManager> introManager;

  /**
   * The descriptor for the intro extension that is valid for this workspace,
   * <code>null</code> if none.
   */
  IntroDescriptor::Pointer introDescriptor;

  bool isStarting;
  bool isClosing;

  int returnCode;

  mutable QString factoryID;

  WorkbenchWindow* activeWorkbenchWindow;

  /**
   * Creates a new workbench.
   *
   * @param display
   *            the display to be used for all UI interactions with the
   *            workbench
   * @param advisor
   *            the application-specific advisor that configures and
   *            specializes this workbench instance
   */
  Workbench(Display*, WorkbenchAdvisor* advisor);

  /**
   * see IWorkbench#GetDisplay
   */
  Display* GetDisplay() const override;

  /*
   * Creates a new workbench window.
   *
   * @return the new workbench window
   */
  SmartPointer<WorkbenchWindow> NewWorkbenchWindow();

  void OpenWindowsAfterRestore();

  /*
   * Returns the number for a new window. This will be the first number > 0
   * which is not used to identify another window in the workbench.
   */
  int GetNewWindowNumber();

  /**
   * Initializes all of the default services for the workbench. For
   * initializing the command-based services, this also parses the registry
   * and hooks up all the required listeners.
   */
  void InitializeDefaultServices();

  /**
   * Closes the workbench. Assumes that the busy cursor is active.
   *
   * @param force
   *            true if the close is mandatory, and false if the close is
   *            allowed to fail
   * @return true if the close succeeded, and false otherwise
   */
  bool BusyClose(bool force);

  /*
   * Record the workbench UI in a document
   */
  XMLMemento::Pointer RecordWorkbenchState();

  /*
   * Restores the state of the previously saved workbench
   */
  bool RestoreState(IMemento::Pointer memento);

  void DoRestoreState(IMemento::Pointer memento, bool& result);

  /*
   * Saves the current state of the workbench so it can be restored later on
   */
  bool SaveState(IMemento::Pointer memento);

  /*
   * Save the workbench UI in a persistence file.
   */
  bool SaveMementoToFile(XMLMemento::Pointer memento);

  /*
   * Answer the workbench state file.
   */
  QString GetWorkbenchStateFile() const;

  /*
   * Shuts down the application.
   */
  void Shutdown();

  /**
   * Opens a new workbench window and page with a specific perspective.
   *
   * Assumes that busy cursor is active.
   */
  IWorkbenchWindow::Pointer BusyOpenWorkbenchWindow(const QString& perspID,
      IAdaptable* input);

  void InitializeSourcePriorities();

  void StartSourceProviders();

  void UpdateActiveWorkbenchWindowMenuManager(bool textOnly);

  void WindowActivated(const IWorkbenchWindow::Pointer& /*window*/) override;
  void WindowDeactivated(const IWorkbenchWindow::Pointer& /*window*/) override;
  void WindowClosed(const IWorkbenchWindow::Pointer& /*window*/) override;
  void WindowOpened(const IWorkbenchWindow::Pointer& /*window*/) override;
};

} // namespace berry

#endif /*BERRYWORKBENCH_H_*/
