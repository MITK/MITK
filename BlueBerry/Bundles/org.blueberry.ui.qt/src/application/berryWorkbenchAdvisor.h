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

#ifndef BERRYWORKBENCHADVISOR_H_
#define BERRYWORKBENCHADVISOR_H_

#include <org_blueberry_ui_qt_Export.h>

#include "berryIMemento.h"
#include <berryIAdaptable.h>

#include "berryWorkbenchWindowAdvisor.h"
#include "berryIWorkbenchConfigurer.h"

namespace berry {

/**
 * public: base class for configuring the workbench.
 * <p>
 * Note that the workbench advisor object is created in advance of creating the
 * workbench. However, by the time the workbench starts calling methods on this
 * class, <code>PlatformUI#GetWorkbench()</code> is guaranteed to have been
 * properly initialized.
 * </p>
 * <p>
 * Example of creating and running a workbench (in an
 * <code>berry#IApplication</code>):
 *
 * <pre>
 * <code>
 *           class MyApplication : public %berry::IApplication {
 *
 *             public:
 *
 *             int Start()
 *             {
 *               WorkbenchAdvisor* workbenchAdvisor = new MyWorkbenchAdvisor();
 *               %berry::Display* display = %berry::PlatformUI::CreateDisplay();
 *               int returnCode = berry::PlatformUI::CreateAndRunWorkbench(display, workbenchAdvisor);
 *
 *               if (returnCode == %PlatformUI::RETURN_RESTART) {
 *                  return %berry::IApplication::EXIT_RESTART;
 *               } else {
 *                  return %berry::IApplication::EXIT_OK;
 *             }
 *           };
 * </code>
 * </pre>
 *
 * </p>
 * <p>
 * An application should declare a subclass of <code>berry::WorkbenchAdvisor</code>
 * and override methods to configure the workbench to suit the needs of the
 * particular application.
 * </p>
 * <p>
 * The following advisor methods are called at strategic points in the
 * workbench's lifecycle (all occur within the dynamic scope of the call to
 * <code>PlatformUI#CreateAndRunWorkbench()</code>):
 * <ul>
 * <li>Initialize() - called first; before any windows; use to
 * register things</li>
 * <li>PreStartup() - called second; after initialize but before
 * first window is opened; use to temporarily disable things during startup or
 * restore</li>
 * <li>PostStartup() - called third; after first window is opened;
 * use to reenable things temporarily disabled in previous step</li>
 * <li>EventLoopException() - called to handle the case where the
 * event loop has crashed; use to inform the user that things are not well (not implemented yet)</li>
 * <li>EventLoopIdle() - called when there are currently no more
 * events to be processed; use to perform other work or to yield until new
 * events enter the queue (not implemented yet)</li>
 * <li>PreShutdown() - called immediately prior to workbench
 * shutdown before any windows have been closed; allows the advisor to veto the
 * shutdown</li>
 * <li>PostShutdown() - called last; after event loop has
 * terminated and all windows have been closed; use to deregister things
 * registered during initialize</li>
 * </ul>
 * </p>
 *
 */
class BERRY_UI_QT WorkbenchAdvisor {

  /**
   * The workbench configurer.
   */
  private: IWorkbenchConfigurer::Pointer workbenchConfigurer;

  /*
   * The workbench error handler.
   */
  //private: AbstractStatusHandler workbenchErrorHandler;

  //private: bool introOpened;

  /**
   * Creates and initializes a new workbench advisor instance.
   */
  protected: WorkbenchAdvisor();
             virtual ~WorkbenchAdvisor();

  /**
   * Remembers the configurer and calls <code>Initialize()</code>.
   * <p>
   * For internal use by the workbench only.
   * </p>
   *
   * @param configurer
   *            an object for configuring the workbench
   */
  public: void InternalBasicInitialize(IWorkbenchConfigurer::Pointer configurer);

  /**
   * Performs arbitrary initialization before the workbench starts running.
   * <p>
   * This method is called during workbench initialization prior to any
   * windows being opened. Clients must not call this method directly
   * (although super calls are okay). The default implementation does nothing.
   * Subclasses may override. Typical clients will use the configurer passed
   * in to tweak the workbench. If further tweaking is required in the future,
   * the configurer may be obtained using <code>GetWorkbenchConfigurer()</code>.
   * </p>
   *
   * @param configurer
   *            an object for configuring the workbench
   */
  public: virtual void Initialize(IWorkbenchConfigurer::Pointer configurer);

  /**
   * Returns the workbench configurer for the advisor. Can be
   * <code>null</code> if the advisor is not initialized yet.
   *
   * @return the workbench configurer, or <code>null</code> if the advisor
   *         is not initialized yet
   */
  protected: IWorkbenchConfigurer::Pointer GetWorkbenchConfigurer();

  /*
   * Returns the workbench error handler for the advisor.
   *
   * @return the workbench error handler
   * @since 3.3
   */
//  public: AbstractStatusHandler getWorkbenchErrorHandler() {
//    if (workbenchErrorHandler == null) {
//      workbenchErrorHandler = new WorkbenchErrorHandler();
//    }
//    return workbenchErrorHandler;
//  }

  /**
   * Performs arbitrary actions just before the first workbench window is
   * opened (or restored).
   * <p>
   * This method is called after the workbench has been initialized and just
   * before the first window is about to be opened. Clients must not call this
   * method directly (although super calls are okay). The default
   * implementation does nothing. Subclasses may override.
   * </p>
   */
  public: virtual void PreStartup();

  /**
   * Performs arbitrary actions after the workbench windows have been opened
   * (or restored), but before the main event loop is run.
   * <p>
   * This method is called just after the windows have been opened. Clients
   * must not call this method directly (although super calls are okay). The
   * default implementation does nothing. Subclasses may override. It is okay
   * to call <code>IWorkbench#Close()</code> from this method.
   * </p>
   */
  public: virtual void PostStartup();

  /**
   * Performs arbitrary finalization before the workbench is about to shut
   * down.
   * <p>
   * This method is called immediately prior to workbench shutdown before any
   * windows have been closed. Clients must not call this method directly
   * (although super calls are okay). The default implementation returns
   * <code>true</code>. Subclasses may override.
   * </p>
   * <p>
   * The advisor may veto a regular shutdown by returning <code>false</code>,
   * although this will be ignored if the workbench is being forced to shut
   * down.
   * </p>
   *
   * @return <code>true</code> to allow the workbench to proceed with
   *         shutdown, <code>false</code> to veto a non-forced shutdown
   */
  public: virtual bool PreShutdown();

  /**
   * Performs arbitrary finalization after the workbench stops running.
   * <p>
   * This method is called during workbench shutdown after all windows have
   * been closed. Clients must not call this method directly (although super
   * calls are okay). The default implementation does nothing. Subclasses may
   * override.
   * </p>
   */
  public: virtual void PostShutdown();

  /*
   * Performs arbitrary actions when the event loop crashes (the code that
   * handles a UI event throws an exception that is not caught).
   * <p>
   * This method is called when the code handling a UI event throws an
   * exception. In a perfectly functioning application, this method would
   * never be called. In practice, it comes into play when there are bugs in
   * the code that trigger unchecked runtime exceptions. It is also activated
   * when the system runs short of memory, etc. Fatal errors (ThreadDeath) are
   * not passed on to this method, as there is nothing that could be done.
   * </p>
   * <p>
   * Clients must not call this method directly (although super calls are
   * okay). The default implementation logs the problem so that it does not go
   * unnoticed. Subclasses may override or extend this method. It is generally
   * a bad idea to override with an empty method, and you should be especially
   * careful when handling Errors.
   * </p>
   *
   * @param exception
   *            the uncaught exception that was thrown inside the UI event
   *            loop
   */
//  public: void eventLoopException(Throwable exception) {
//    // Protection from client doing super(null) call
//    if (exception == null) {
//      return;
//    }
//
//    try {
//      StatusManager.getManager().handle(
//          new Status(IStatus.ERR, WorkbenchPlugin.PI_WORKBENCH,
//              "Unhandled event loop exception", exception)); //$NON-NLS-1$
//
//      if (WorkbenchPlugin.DEBUG) {
//        exception.printStackTrace();
//      }
//    } catch (Throwable e) {
//      // One of the log listeners probably failed. Core should have logged
//      // the
//      // exception since its the first listener.
//      System.err.println("Error while logging event loop exception:"); //$NON-NLS-1$
//      exception.printStackTrace();
//      System.err.println("Logging exception:"); //$NON-NLS-1$
//      e.printStackTrace();
//    }
//  }

  /*
   * Performs arbitrary work or yields when there are no events to be
   * processed.
   * <p>
   * This method is called when there are currently no more events on the
   * queue to be processed at the moment.
   * </p>
   * <p>
   * Clients must not call this method directly (although super calls are
   * okay). The default implementation yields until new events enter the
   * queue. Subclasses may override or extend this method. It is generally a
   * bad idea to override with an empty method. It is okay to call
   * <code>IWorkbench.close()</code> from this method.
   * </p>
   *
   * @param display
   *            the main display of the workbench UI
   */
//  public: void eventLoopIdle(Display display) {
//    // default: yield cpu until new events enter the queue
//    display.sleep();
//  }

  /**
   * Creates a new workbench window advisor for configuring a new workbench
   * window via the given workbench window configurer. Clients should override
   * to provide their own window configurer.
   *
   * @param configurer
   *            the workbench window configurer
   * @return a new workbench window advisor
   */
  public: virtual WorkbenchWindowAdvisor* CreateWorkbenchWindowAdvisor(
      IWorkbenchWindowConfigurer::Pointer configurer) = 0;


  /**
   * Returns the default input for newly created workbench pages when the
   * input is not explicitly specified.
   * <p>
   * The default implementation returns <code>null</code>. Subclasses may
   * override.
   * </p>
   *
   * @return the default input for a new workbench window page, or
   *         <code>null</code> if none
   *
   * @see #CreateWorkbenchWindowAdvisor(IWorkbenchWindowConfigurer::Pointer)
   */
  public: virtual IAdaptable* GetDefaultPageInput();

  /**
   * Returns the id of the perspective to use for the initial workbench
   * window, or <code>null</code> if no initial perspective should be shown
   * in the initial workbench window.
   * <p>
   * This method is called during startup when the workbench is creating the
   * first new window. Subclasses must implement.
   * </p>
   * <p>
   * If the <code>WorkbenchPreferenceConstants#DEFAULT_PERSPECTIVE_ID</code>
   * preference is specified, it supercedes the perspective specified here.
   * </p>
   *
   * @return the id of the perspective for the initial window, or
   *         <code>null</code> if no initial perspective should be shown
   */
  public: virtual QString GetInitialWindowPerspectiveId() = 0;

  /**
   * Returns the id of the preference page that should be presented most
   * prominently.
   * <p>
   * The default implementation returns <code>null</code>. Subclasses may
   * override.
   * </p>
   *
   * @return the id of the preference page, or <code>null</code> if none
   */
  public: virtual QString GetMainPreferencePageId();

  /**
   * Opens the workbench windows on startup. The default implementation tries
   * to restore the previously saved workbench state using
   * <code>IWorkbenchConfigurer#RestoreState()</code>. If there
   * was no previously saved state, or if the restore failed, then a
   * first-time window is opened using
   * IWorkbenchConfigurer#OpenFirstTimeWindow().
   *
   * @return <code>true</code> to proceed with workbench startup, or
   *         <code>false</code> to exit
   */
  public: virtual bool OpenWindows();

  /**
   * Saves arbitrary application-specific state information for this workbench
   * advisor.
   * <p>
   * The default implementation simply returns an OK status. Subclasses may
   * extend or override.
   * </p>
   *
   * @param memento
   *            the memento in which to save the advisor's state
   * @return a status object indicating whether the save was successful
   */
  public: virtual bool SaveState(IMemento::Pointer memento);

  /**
   * Restores arbitrary application-specific state information for this
   * workbench advisor.
   * <p>
   * The default implementation simply returns an OK status. Subclasses may
   * extend or override.
   * </p>
   *
   * @param memento
   *            the memento from which to restore the advisor's state
   * @return a status object indicating whether the restore was successful
   */
  public: virtual bool RestoreState(IMemento::Pointer memento);

  /*
   * Return the contribution comparator for the particular type of
   * contribution. The default implementation of this class returns a
   * comparator that sorts the items by label.
   *
   * The contributionType may be one of the constants in
   * {@link IContributionService} or it can be a value defined by the user.
   *
   * @param contributionType
   *            the contribution type
   * @return the comparator, must not return <code>null</code>
   * @see IContributionService#GetComparatorFor(const QString&)
   */
//  public: ContributionComparator getComparatorFor(String contributionType) {
//    return new ContributionComparator();
//  }
};

}

#endif /*BERRYWORKBENCHADVISOR_H_*/
