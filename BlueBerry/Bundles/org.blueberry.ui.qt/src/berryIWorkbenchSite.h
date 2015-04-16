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

#ifndef BERRYIWORKBENCHSITE_H_
#define BERRYIWORKBENCHSITE_H_

#include <berryMacros.h>

#include <org_blueberry_ui_qt_Export.h>

#include "services/berryIServiceLocator.h"

namespace berry {

struct IWorkbenchPage;
struct ISelectionProvider;
struct IWorkbenchWindow;
class  Shell;

/**
 * \ingroup org_blueberry_ui_qt
 *
 * The common interface between the workbench and its parts, including pages
 * within parts.
 * <p>
 * The workbench site supports a few {@link IServiceLocator services} by
 * default. If these services are used to allocate resources, it is important to
 * remember to clean up those resources after you are done with them. Otherwise,
 * the resources will exist until the workbench site is disposed. The supported
 * services are:
 * </p>
 * <ul>
 * <li>{@link ICommandService}</li>
 * <li>{@link IContextService}</li>
 * <li>{@link IHandlerService}</li>
 * <li>{@link IBindingService}. Resources allocated through this service will
 * not be cleaned up until the workbench shuts down.</li>
 * </ul>
 * <p>
 * This interface is not intended to be implemented or extended by clients.
 * </p>
 *
 * @see org.blueberry.ui.IWorkbenchPartSite
 * @see org.blueberry.ui.part.IPageSite
 * @since 2.0
 * @noimplement This interface is not intended to be implemented by clients.
 */
struct BERRY_UI_QT IWorkbenchSite : public IServiceLocator { // IAdaptable, IShellProvider {

  berryObjectMacro(berry::IWorkbenchSite, IServiceLocator)

  virtual ~IWorkbenchSite();

  /**
   * Returns the page containing this workbench site.
   *
   * @return the page containing this workbench site
   */
  virtual SmartPointer<IWorkbenchPage> GetPage() = 0;

  /**
   * Returns the selection provider for this workbench site.
   *
   * @return the selection provider, or <code>null</code> if none
   */
  virtual SmartPointer<ISelectionProvider> GetSelectionProvider() = 0;

  /**
   * Returns the shell for this workbench site. Not intended to be called from
   * outside the UI thread. Clients should call IWorkbench.getDisplay() to
   * gain access to the display rather than calling getShell().getDisplay().
   *
   * @return the shell for this workbench site
   */
  virtual SmartPointer<Shell> GetShell() = 0;

  /**
   * Returns the workbench window containing this workbench site.
   *
   * @return the workbench window containing this workbench site
   */
  virtual SmartPointer<IWorkbenchWindow> GetWorkbenchWindow() = 0;

  /**
   * Sets the selection provider for this workbench site.
   *
   * @param provider
   *            the selection provider, or <code>null</code> to clear it
   */
  virtual void SetSelectionProvider(SmartPointer<ISelectionProvider> provider) = 0;

};

}

#endif /*BERRYIWORKBENCHSITE_H_*/
