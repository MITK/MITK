/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWORKBENCHCONFIGURER_H_
#define BERRYWORKBENCHCONFIGURER_H_

#include "application/berryIWorkbenchConfigurer.h"

#include <Poco/HashMap.h>

namespace berry
{

/**
 * Internal class providing special access for configuring the workbench.
 * <p>
 * Note that these objects are only available to the main application
 * (the plug-in that creates and owns the workbench).
 * </p>
 * <p>
 * This class is not intended to be instantiated or subclassed by clients.
 * </p>
 */
class WorkbenchConfigurer : public IWorkbenchConfigurer
{

public:

  berryObjectMacro(WorkbenchConfigurer);

private:

    /**
     * Table to hold arbitrary key-data settings.
     * @see #SetData
     */
    QHash<QString, Object::Pointer> extraData;

    /**
     * Indicates whether workbench state should be saved on close and
     * restored on subsequent open.
     */
    bool saveAndRestore;

    /**
     * Indicates whether the workbench is being forced to close. During
     * an emergency close, no interaction with the user should be done.
     */
    bool isEmergencyClosing;

    /**
     * Indicates the behaviour when the last window is closed.
     * If <code>true</code>, the workbench will exit (saving the last window's state,
     * if configured to do so).
     * If <code>false</code> the window will be closed, leaving the workbench running.
     *
     * @since 3.1
     */
  bool exitOnLastWindowClose;

public:

    /**
     * Creates a new workbench configurer.
     * <p>
     * This method is declared package-private. Clients are passed an instance
     * only via {@link WorkbenchAdvisor#initialize WorkbenchAdvisor.initialize}
     * </p>
     */
    WorkbenchConfigurer();

    /* (non-javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#getWorkbench
     */
    IWorkbench* GetWorkbench() override;


    /* (non-javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#declareImage
     */
//    void declareImage(String symbolicName, ImageDescriptor descriptor,
//            boolean shared) {
//        if (symbolicName == null || descriptor == null) {
//            throw new IllegalArgumentException();
//        }
//        WorkbenchImages.declareImage(symbolicName, descriptor, shared);
//    }

    /* (non-javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#getWindowConfigurer
     */
    IWorkbenchWindowConfigurer::Pointer GetWindowConfigurer(IWorkbenchWindow::Pointer window) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#getSaveAndRestore()
     */
    bool GetSaveAndRestore() override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#setSaveAndRestore(boolean)
     */
    void SetSaveAndRestore(bool enabled) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#getData
     */
    Object::Pointer GetData(const QString& key) const override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#setData(String, Object)
     */
    void SetData(const QString& key, Object::Pointer data) override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#emergencyClose()
     */
    void EmergencyClose() override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#emergencyClosing()
     */
    bool EmergencyClosing() override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#restoreState()
     */
    bool RestoreState() override;

    /* (non-Javadoc)
     * @see org.blueberry.ui.application.IWorkbenchConfigurer#openFirstTimeWindow()
     */
    void OpenFirstTimeWindow() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.application.IWorkbenchConfigurer#restoreWorkbenchWindow(org.blueberry.ui.IMemento)
   */
  IWorkbenchWindowConfigurer::Pointer RestoreWorkbenchWindow(IMemento::Pointer memento) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.application.IWorkbenchConfigurer#getExitOnLastWindowClose()
   */
  bool GetExitOnLastWindowClose() override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.application.IWorkbenchConfigurer#setExitOnLastWindowClose(boolean)
   */
  void SetExitOnLastWindowClose(bool enabled) override;
};

}

#endif /*BERRYWORKBENCHCONFIGURER_H_*/
