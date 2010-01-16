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


#ifndef BERRYIWORKBENCHLOCATIONSERVICE_H_
#define BERRYIWORKBENCHLOCATIONSERVICE_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

namespace berry {

struct IWorkbench;
struct IWorkbenchWindow;
struct IWorkbenchPartSite;
struct IPageSite;

/**
 * Query where you are in the workbench hierarchy.
 *
 * @since 3.4
 */
struct IWorkbenchLocationService : public Object {

  osgiInterfaceMacro(berry::IWorkbenchLocationService)

  /**
   * Get the service scope.
   *
   * @return the service scope. May return <code>null</code>.
   * @see IServiceScopes#PARTSITE_SCOPE
   */
  virtual std::string GetServiceScope() const = 0;

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
  virtual SmartPointer<IWorkbenchWindow> GetWorkbenchWindow() const = 0;

  /**
   * @return the part site in this service locator hierarchy. May return
   *  <code>null</code>.
   */
  virtual SmartPointer<IWorkbenchPartSite> GetPartSite() const = 0;

  /**
   * @return the inner page site for a page based view in this service locator
   *  hierarchy. May return <code>null</code>.
   * @see PageBookView
   */
  virtual SmartPointer<IPageSite> GetPageSite() const = 0;
};

}

#endif /* BERRYIWORKBENCHLOCATIONSERVICE_H_ */
