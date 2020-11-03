/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
 * IContributionItem::Pointer reEdit
 *     = ContributionItemFactory::REOPEN_EDITORS->Create(window);
 * menu->Add(reEdit);
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
    * Workbench contribution item (id "openWindows"): A list of windows
    * currently open in the workbench. Selecting one of the items makes the
    * corresponding window the active window.
    * This action dynamically maintains the list of windows.
    */
   static const QScopedPointer<ContributionItemFactory> OPEN_WINDOWS;

   /**
    * Workbench contribution item (id "viewsShortlist"): A list of views
    * available to be opened in the window, arranged as a shortlist of
    * promising views and an "Other" subitem. Selecting
    * one of the items opens the corresponding view in the active window.
    * This action dynamically maintains the view shortlist.
    */
   static const QScopedPointer<ContributionItemFactory> VIEWS_SHORTLIST;

   /**
    * Workbench contribution item (id "reopenEditors"): A list of recent
    * editors (with inputs) available to be reopened in the window. Selecting
    * one of the items reopens the corresponding editor on its input in the
    * active window. This action dynamically maintains the list of editors.
    */
   static const QScopedPointer<ContributionItemFactory> REOPEN_EDITORS;

   /**
    * Workbench contribution item (id "perspectivesShortlist"): A list of
    * perspectives available to be opened, arranged as a shortlist of
    * promising perspectives and an "Other" subitem. Selecting
    * one of the items makes the corresponding perspective active. Should a
    * new perspective need to be opened, a workbench user preference controls
    * whether the prespective is opened in the active window or a new window.
    * This action dynamically maintains the perspectives shortlist.
    */
   static const QScopedPointer<ContributionItemFactory> PERSPECTIVES_SHORTLIST;

   virtual ~ContributionItemFactory() = default;

};

}

#endif // BERRYCONTRIBUTIONITEMFACTORY_H
