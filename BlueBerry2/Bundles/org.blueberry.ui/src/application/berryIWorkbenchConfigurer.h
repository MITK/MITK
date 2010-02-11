/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYIWORKBENCHCONFIGURER_H_
#define BERRYIWORKBENCHCONFIGURER_H_

#include <osgi/framework/Macros.h>

#include "../berryUiDll.h"
#include "../berryIWorkbench.h"

#include "berryIWorkbenchWindowConfigurer.h"

namespace berry {

/**
 * Interface providing special access for configuring the workbench.
 * <p>
 * Note that these objects are only available to the main application
 * (the plug-in that creates and owns the workbench).
 * </p>
 * <p>
 * This interface is not intended to be implemented by clients.
 * </p>
 *
 * @see WorkbenchAdvisor#Initialize()
 * @note This interface is not intended to be implemented by clients.
 */
struct BERRY_UI IWorkbenchConfigurer : public Object {

  osgiInterfaceMacro(berry::IWorkbenchConfigurer);

    /**
     * Restore status code indicating that the saved state
     * could not be restored, but that startup should continue
     * with a reset state.
     *
     * @see #RestoreState()
     */
    static const int RESTORE_CODE_RESET = 1;

    /**
     * Restore status code indicating that the saved state
     * could not be restored, and that the application
     * must exit immediately without modifying any previously
     * saved workbench state.
     */
    static const int RESTORE_CODE_EXIT = 2;

    /**
     * Returns the underlying workbench.
     *
     * @return the workbench
     */
    virtual IWorkbench* GetWorkbench() = 0;

    /**
     * Returns whether the workbench state should be saved on close and
     * restored on subsequent open.
     * <p>
     * The initial value is <code>false</code>.
     * </p>
     *
     * @return <code>true</code> to save and restore workbench state, or
     *  <code>false</code> to forget current workbench state on close.
     */
    virtual bool GetSaveAndRestore() = 0;

    /**
     * Sets whether the workbench state should be saved on close and
     * restored on subsequent open.
     *
     * @param enabled <code>true</code> to save and restore workbench state, or
     *  <code>false</code> to forget current workbench state on close.
     */
    virtual void SetSaveAndRestore(bool enabled) = 0;

  /**
   * Restores a workbench window from the given memento.
   *
   * @param memento the memento from which to restore the window's state
   * @return the configurer for the restored window
   * @throws WorkbenchException if an error occurred during the restore
   * @see IWorkbenchWindowConfigurer#SaveState(IMemento::Pointer)
   */
  virtual IWorkbenchWindowConfigurer::Pointer RestoreWorkbenchWindow(IMemento::Pointer memento) = 0;

//    /**
//     * Declares a workbench image.
//     * <p>
//     * The workbench remembers the given image descriptor under the given name,
//     * and makes the image available to plug-ins via
//     * {@link IWorkbench#GetSharedImages() IWorkbench.getSharedImages()}.
//     * For "shared" images, the workbench remembers the image descriptor and
//     * will manages the image object create from it; clients retrieve "shared"
//     * images via
//     * {@link org.blueberry.ui.ISharedImages#getImage ISharedImages.getImage()}.
//     * For the other, "non-shared" images, the workbench remembers only the
//     * image descriptor; clients retrieve the image descriptor via
//     * {@link org.blueberry.ui.ISharedImages#getImageDescriptor
//     * ISharedImages.getImageDescriptor()} and are entirely
//     * responsible for managing the image objects they create from it.
//     * (This is made confusing by the historical fact that the API interface
//     *  is called "ISharedImages".)
//     * </p>
//     *
//     * @param symbolicName the symbolic name of the image
//     * @param descriptor the image descriptor
//     * @param shared <code>true</code> if this is a shared image, and
//     * <code>false</code> if this is not a shared image
//     * @see org.blueberry.ui.ISharedImages#getImage
//     * @see org.blueberry.ui.ISharedImages#getImageDescriptor
//     */
//    virtual void declareImage(String symbolicName, ImageDescriptor descriptor,
//            boolean shared);

    /**
     * Forces the workbench to close due to an emergency. This method should
     * only be called when the workbench is in dire straights and cannot
     * continue, and cannot even risk a normal workbench close (think "out of
     * memory" or "unable to create shell"). When this method is called, an
     * abbreviated workbench shutdown sequence is performed (less critical
     * steps may be skipped). The workbench advisor is still called; however,
     * it must not attempt to communicate with the user. While an emergency
     * close is in progress, EmergencyClosing() returns
     * <code>true</code>. Workbench advisor methods should always check this
     * flag before communicating with the user.
     *
     * @see #EmergencyClosing()
     */
    virtual void EmergencyClose() = 0;

    /**
     * Returns whether the workbench is being closed due to an emergency.
     * When this method returns <code>true</code>, the workbench is in dire
     * straights and cannot continue. Indeed, things are so bad that we cannot
     * even risk a normal workbench close. Workbench advisor methods should
     * always check this flag before attempting to communicate with the user.
     *
     * @return <code>true</code> if the workbench is in the process of being
     * closed under emergency conditions, and <code>false</code> otherwise
     */
    virtual bool EmergencyClosing() = 0;

    /**
     * Returns an object that can be used to configure the given window.
     *
     * @param window a workbench window
     * @return a workbench window configurer
     */
    virtual IWorkbenchWindowConfigurer::Pointer GetWindowConfigurer(
            IWorkbenchWindow::Pointer window) = 0;

    /**
     * Returns the data associated with the workbench at the given key.
     *
     * @param key the key
     * @return the data, or <code>null</code> if there is no data at the given
     * key
     */
    virtual Object::Pointer GetData(const std::string& key) const = 0;

    /**
     * Sets the data associated with the workbench at the given key.
     *
     * @param key the key
     * @param data the data, or <code>null</code> to delete existing data
     */
    virtual void SetData(const std::string& key, Object::Pointer data) = 0;

    /**
     * Restores the workbench state saved from the previous session, if any.
     * This includes any open windows and their open perspectives, open views
     * and editors, layout information, and any customizations to the open
     * perspectives.
     * <p>
     * This is typically called from the advisor's <code>openWindows()</code>
     * method.
     * </p>
     *
     * @return a status object indicating whether the restore was successful
     * @see #RESTORE_CODE_RESET
     * @see #RESTORE_CODE_EXIT
     * @see WorkbenchAdvisor#OpenWindows()
     */
    virtual bool RestoreState() = 0;

    /**
     * Opens the first time window, using the default perspective and
     * default page input.
     * <p>
     * This is typically called from the advisor's OpenWindows()
     * method.
     * </p>
     *
     * @see WorkbenchAdvisor#OpenWindows()
     */
    virtual void OpenFirstTimeWindow() = 0;

    /**
   * Returns <code>true</code> if the workbench should exit when the last
   * window is closed, <code>false</code> if the window should just be
   * closed, leaving the workbench (and its event loop) running.
   * <p>
   * If <code>true</code>, the last window's state is saved before closing,
   * so that it will be restored in the next session. This applies only if
   * #GetSaveAndRestore() returns <code>true</code>).
   * </p>
   * <p>
   * If <code>false</code>, the window is simply closed, losing its state.
   * </p>
   * <p>
   * If the workbench is left running, it can be closed using
   * IWorkbench#Close(), or a new window can be opened using
   * IWorkbench#OpenWorkbenchWindow(const std::string&, IAdaptable*).
   * </p>
   * <p>
   * The initial value is <code>true</code>.
   * </p>
   *
   * @return <code>true</code> if the workbench will exit when the last
   *         window is closed, <code>false</code> if the window should just
   *         be closed
   */
    virtual bool GetExitOnLastWindowClose() = 0;

    /**
   * Sets whether the workbench should exit when the last window is closed, or
   * whether the window should just be closed, leaving the workbench (and its
   * event loop) running.
   * <p>
   * For more details, see #GetExitOnLastWindowClose().
   * </p>
   *
   * @param enabled
   *            <code>true</code> if the workbench should exit when the last
   *            window is closed, <code>false</code> if the window should
   *            just be closed
   */
    virtual void SetExitOnLastWindowClose(bool enabled) = 0;
};

}

#endif /*BERRYIWORKBENCHCONFIGURER_H_*/
