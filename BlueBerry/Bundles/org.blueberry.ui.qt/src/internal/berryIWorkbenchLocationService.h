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


#ifndef BERRYIWORKBENCHLOCATIONSERVICE_H_
#define BERRYIWORKBENCHLOCATIONSERVICE_H_

#include <berryObject.h>

#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct IWorkbench;
struct IWorkbenchWindow;
struct IWorkbenchPartSite;
struct IPageSite;

/**
 * Query where you are in the workbench hierarchy.
 *
 */
struct BERRY_UI_QT IWorkbenchLocationService : public virtual Object
{

  berryObjectMacro(berry::IWorkbenchLocationService)

  ~IWorkbenchLocationService();

  /**
   * Get the service scope.
   *
   * @return the service scope. May return <code>null</code>.
   * @see IServiceScopes#PARTSITE_SCOPE
   */
  virtual QString GetServiceScope() const = 0;

  /**
   * A more numeric representation of the service level.
   *
   * @return the level - 0==workbench, 1==workbench window, etc
   */
  virtual int GetServiceLevel() const = 0;

  /**
   * @return the workbench. May return <code>null</code>.
   */
  virtual IWorkbench* GetWorkbench() const = 0;

  /**
   * @return the workbench window in this service locator hierarchy. May
   *  return <code>null</code>.
   */
  virtual IWorkbenchWindow* GetWorkbenchWindow() const = 0;

  /**
   * @return the part site in this service locator hierarchy. May return
   *  <code>null</code>.
   */
  virtual IWorkbenchPartSite* GetPartSite() const = 0;

  /**
   * @return the inner page site for a page based view in this service locator
   *  hierarchy. May return <code>null</code>.
   * @see PageBookView
   */
  //virtual SmartPointer<IPageSite> GetPageSite() const = 0;
};

}
Q_DECLARE_INTERFACE(berry::IWorkbenchLocationService, "org.blueberry.ui.IWorkbenchLocationService")

#endif /* BERRYIWORKBENCHLOCATIONSERVICE_H_ */
