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

#ifndef CHERRYWORKBENCHWINDOWADVISOR_H_
#define CHERRYWORKBENCHWINDOWADVISOR_H_

#include "../cherryIMemento.h"
#include "../cherryShell.h"

#include "cherryActionBarAdvisor.h"
#include "cherryIActionBarConfigurer.h"


namespace cherry
{

/**
 * Public base class for configuring a workbench window.
 * <p>
 * The workbench window advisor object is created in response to a workbench
 * window being created (one per window), and is used to configure the window.
 * </p>
 * <p>
 * An application should declare a subclass of <code>WorkbenchWindowAdvisor</code>
 * and override methods to configure workbench windows to suit the needs of the
 * particular application.
 * </p>
 * <p>
 * The following advisor methods are called at strategic points in the
 * workbench window's lifecycle (as with the workbench advisor, all occur
 * within the dynamic scope of the call to
 * {@link PlatformUI#createAndRunWorkbench PlatformUI.createAndRunWorkbench}):
 * <ul>
 * <li><code>preWindowOpen</code> - called as the window is being opened;
 *  use to configure aspects of the window other than actions bars</li>
 * <li><code>postWindowRestore</code> - called after the window has been
 * recreated from a previously saved state; use to adjust the restored
 * window</li>
 * <li><code>postWindowCreate</code> -  called after the window has been created,
 * either from an initial state or from a restored state;  used to adjust the
 * window</li>
 * <li><code>openIntro</code> - called immediately before the window is opened in
 * order to create the introduction component, if any.</li>
 * <li><code>postWindowOpen</code> - called after the window has been
 * opened; use to hook window listeners, etc.</li>
 * <li><code>preWindowShellClose</code> - called when the window's shell
 * is closed by the user; use to pre-screen window closings</li>
 * </ul>
 * </p>
 *
 * @since 3.1
 */
class CHERRY_UI WorkbenchWindowAdvisor {

private:

  IWorkbenchWindowConfigurer::Pointer windowConfigurer;

protected:

  /**
   * Returns the workbench window configurer.
   *
   * @return the workbench window configurer
   */
  IWorkbenchWindowConfigurer::Pointer GetWindowConfigurer();

public:

    /**
     * Creates a new workbench window advisor for configuring a workbench
     * window via the given workbench window configurer.
     *
     * @param configurer an object for configuring the workbench window
     */
    WorkbenchWindowAdvisor(IWorkbenchWindowConfigurer::Pointer configurer);
	virtual ~WorkbenchWindowAdvisor();


    /**
     * Performs arbitrary actions before the window is opened.
     * <p>
     * This method is called before the window's controls have been created.
     * Clients must not call this method directly (although super calls are okay).
     * The default implementation does nothing. Subclasses may override.
     * Typical clients will use the window configurer to tweak the
     * workbench window in an application-specific way; however, filling the
     * window's menu bar, tool bar, and status line must be done in
     * {@link ActionBarAdvisor#fillActionBars}, which is called immediately
     * after this method is called.
     * </p>
     */
    virtual void PreWindowOpen();

    /**
     * Creates a new action bar advisor to configure the action bars of the window
     * via the given action bar configurer.
     * The default implementation returns a new instance of {@link ActionBarAdvisor}.
     *
     * @param configurer the action bar configurer for the window
     * @return the action bar advisor for the window
     */
    virtual ActionBarAdvisor::Pointer CreateActionBarAdvisor(IActionBarConfigurer::Pointer configurer);

    /**
     * Performs arbitrary actions after the window has been restored,
     * but before it is opened.
     * <p>
     * This method is called after a previously-saved window has been
     * recreated. This method is not called when a new window is created from
     * scratch. This method is never called when a workbench is started for the
     * very first time, or when workbench state is not saved or restored.
     * Clients must not call this method directly (although super calls are okay).
     * The default implementation does nothing. Subclasses may override.
     * It is okay to call <code>IWorkbench.close()</code> from this method.
     * </p>
     *
     * @exception WorkbenchException thrown if there are any errors to report
     *   from post-restoration of the window
     */
    virtual void PostWindowRestore();

    /**
     * Opens the introduction componenet.
     * <p>
     * Clients must not call this method directly (although super calls are okay).
     * The default implementation opens the intro in the first window provided
     * if the preference IWorkbenchPreferences.SHOW_INTRO is <code>true</code>.  If
     * an intro is shown then this preference will be set to <code>false</code>.
     * Subsequently, and intro will be shown only if
     * <code>WorkbenchConfigurer.getSaveAndRestore()</code> returns
     * <code>true</code> and the introduction was visible on last shutdown.
     * Subclasses may override.
     * </p>
     */
    virtual void OpenIntro();

    /**
     * Performs arbitrary actions after the window has been created (possibly
     * after being restored), but has not yet been opened.
     * <p>
     * This method is called after the window has been created from scratch,
     * or when it has been restored from a previously-saved window.  In the latter case,
     * this method is called after <code>postWindowRestore</code>.
     * Clients must not call this method directly (although super calls are okay).
     * The default implementation does nothing. Subclasses may override.
     * </p>
     */
    virtual void PostWindowCreate();

    /**
     * Performs arbitrary actions after the window has been opened (possibly
     * after being restored).
     * <p>
     * This method is called after the window has been opened. This method is
     * called after the window has been created from scratch, or when
     * it has been restored from a previously-saved window.
     * Clients must not call this method directly (although super calls are okay).
     * The default implementation does nothing. Subclasses may override.
     * </p>
     */
    virtual void PostWindowOpen();

    /**
   * Performs arbitrary actions as the window's shell is being closed
   * directly, and possibly veto the close.
   * <p>
   * This method is called from a ShellListener associated with the window,
   * for example when the user clicks the window's close button. It is not
   * called when the window is being closed for other reasons, such as if the
   * user exits the workbench via the {@link ActionFactory#QUIT} action.
   * Clients must not call this method directly (although super calls are
   * okay). If this method returns <code>false</code>, then the user's
   * request to close the shell is ignored. This gives the workbench advisor
   * an opportunity to query the user and/or veto the closing of a window
   * under some circumstances.
   * </p>
   *
   * @return <code>true</code> to allow the window to close, and
   *         <code>false</code> to prevent the window from closing
   * @see org.opencherry.ui.IWorkbenchWindow#close
   * @see WorkbenchAdvisor#preShutdown()
   */
  virtual bool PreWindowShellClose();

    /**
   * Performs arbitrary actions after the window is closed.
   * <p>
   * This method is called after the window's controls have been disposed.
   * Clients must not call this method directly (although super calls are
   * okay). The default implementation does nothing. Subclasses may override.
   * </p>
   */
    virtual void PostWindowClose();

    /**
     * Creates the contents of the window.
     * <p>
     * The default implementation adds a menu bar, a cool bar, a status line,
     * a perspective bar, and a fast view bar.  The visibility of these controls
     * can be configured using the <code>setShow*</code> methods on
     * <code>IWorkbenchWindowConfigurer</code>.
     * </p>
     * <p>
     * Subclasses may override to define custom window contents and layout,
     * but must call <code>IWorkbenchWindowConfigurer.createPageComposite</code>.
     * </p>
     *
     * @param shell the window's shell
     * @see IWorkbenchWindowConfigurer#createMenuBar
     * @see IWorkbenchWindowConfigurer#createCoolBarControl
     * @see IWorkbenchWindowConfigurer#createStatusLineControl
     * @see IWorkbenchWindowConfigurer#createPageComposite
     */
    virtual void CreateWindowContents(Shell::Pointer shell);

    /**
     * Creates and returns the control to be shown when the window has no open pages.
     * If <code>null</code> is returned, the default window background is shown.
     * <p>
     * The default implementation returns <code>null</code>.
     * Subclasses may override.
     * </p>
     *
     * @param parent the parent composite
     * @return the control or <code>null</code>
     */
    virtual void* CreateEmptyWindowContents(void* parent);



  /**
   * Saves arbitrary application specific state information.
   *
   * @param memento the storage area for object's state
   * @return a status object indicating whether the save was successful
   * @since 3.1
   */
  virtual bool SaveState(IMemento::Pointer memento) ;

  /**
   * Restores arbitrary application specific state information.
   *
   * @param memento the storage area for object's state
   * @return a status object indicating whether the restore was successful
   * @since 3.1
   */
  virtual bool RestoreState(IMemento::Pointer memento);
};

}

#endif /*CHERRYWORKBENCHWINDOWADVISOR_H_*/
