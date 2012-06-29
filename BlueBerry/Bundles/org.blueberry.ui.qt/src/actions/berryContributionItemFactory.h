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


#ifndef BERRYCONTRIBUTIONITEMFACTORY_H
#define BERRYCONTRIBUTIONITEMFACTORY_H

#include <QString>

#include <berrySmartPointer.h>
#include <org_blueberry_ui_qt_Export.h>

namespace berry {

struct IContributionItem;
struct IWorkbenchWindow;

/**
 * Access to standard contribution items provided by the workbench.
 * <p>
 * Most of the functionality of this class is provided by
 * static methods and fields.
 * Example usage:
 * <pre>
 * MenuManager menu = ...;
 * IContributionItem reEdit
 *     = ContributionItemFactory.REOPEN_EDITORS.create(window);
 * menu.add(reEdit);
 * </pre>
 * </p>
 * <p>
 * Clients may declare subclasses that provide additional application-specific
 * contribution item factories.
 * </p>
 */
class BERRY_UI_QT ContributionItemFactory
{

private:

  /**
   * Id of contribution items created by this factory.
   */
  const QString contributionItemId;

protected:

  /**
   * Creates a new workbench contribution item factory with the given id.
   *
   * @param contributionItemId the id of contribution items created by this factory
   */
  ContributionItemFactory(const QString& contributionItemId);

public:

   /**
    * Creates a new standard contribution item for the given workbench window.
    * <p>
    * A typical contribution item automatically registers listeners against the
    * workbench window so that it can keep its enablement state up to date.
    * Ordinarily, the window's references to these listeners will be dropped
    * automatically when the window closes. However, if the client needs to get
    * rid of a contribution item while the window is still open, the client must
    * call IContributionItem#dispose to give the item an
    * opportunity to deregister its listeners and to perform any other cleanup.
    * </p>
    *
    * @param window the workbench window
    * @return the workbench contribution item
    */
   virtual SmartPointer<IContributionItem> Create(IWorkbenchWindow* window) = 0;

   /**
    * Returns the id of this contribution item factory.
    *
    * @return the id of contribution items created by this factory
    */
   QString GetId() const;

   /**
    * Workbench contribution item (id "viewsShortlist"): A list of views
    * available to be opened in the window, arranged as a shortlist of
    * promising views and an "Other" subitem. Selecting
    * one of the items opens the corresponding view in the active window.
    * This action dynamically maintains the view shortlist.
    */
   static ContributionItemFactory* const VIEWS_SHORTLIST;
};

}

#endif // BERRYCONTRIBUTIONITEMFACTORY_H
