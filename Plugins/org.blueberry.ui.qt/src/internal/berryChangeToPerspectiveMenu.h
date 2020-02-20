/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCHANGETOPERSPECTIVEMENU_H
#define BERRYCHANGETOPERSPECTIVEMENU_H

#include <berryContributionItem.h>

namespace berry {

struct IPerspectiveDescriptor;
struct IPerspectiveRegistry;
struct IWorkbenchWindow;
struct IMenuManager;

class CommandContributionItemParameter;

/**
 * A menu for perspective selection.
 * <p>
 * A <code>ChangeToPerspectiveMenu</code> is used to populate a menu with
 * perspective shortcut items.  If the user selects one of these items
 * the perspective of the active page in the window is changed to the
 * selected one.
 * </p><p>
 * The visible perspective items within the menu are dynamic and consists
 * of the perspective shortcut list of the current perspective.
 * </p>
 */
class ChangeToPerspectiveMenu : public QObject, public ContributionItem
{
  Q_OBJECT

public:

  /**
   * Constructs a new instance of <code>PerspectiveMenu</code>.
   *
   * @param window the window containing this menu
   * @param id the menu id
   */
  ChangeToPerspectiveMenu(IWorkbenchWindow* window, const QString& id);

  /*
   * Fills the menu with perspective items.
   */
  void Fill(QMenu* menu, QAction* before) override;

  using ContributionItem::Fill;

  /*
   * Returns whether this menu is dynamic.
   */
  bool IsDirty() const override;

  /*
   * Returns whether this menu is dynamic.
   */
  bool IsDynamic() const override;


private:

  IWorkbenchWindow* window;
  IPerspectiveRegistry* reg;
  bool showActive;
  bool dirty;

  IContributionItem::Pointer showDlgItem;

  /**
   * The translatable message to show when there are no perspectives.
   */
  static const QString NO_TARGETS_MSG;

  Q_SLOT void AboutToShow(IMenuManager* manager);

  /**
   * Fills the given menu manager with all the open perspective actions
   * appropriate for the currently active perspective. Filtering is applied to
   * the actions based on the activities and capabilities mechanism.
   *
   * @param manager
   *            The menu manager that should receive the menu items; must not
   *            be <code>null</code>.
   */
  void FillMenu(IMenuManager* manager);

  SmartPointer<CommandContributionItemParameter> GetItem(const SmartPointer<IPerspectiveDescriptor>& desc) const;

  /*
   * Returns the perspective shortcut items for the active perspective.
   *
   * @return a list of <code>IPerspectiveDescriptor</code> items
   */
  QList<SmartPointer<IPerspectiveDescriptor> > GetPerspectiveShortcuts() const;

};

}

#endif // BERRYCHANGETOPERSPECTIVEMENU_H
