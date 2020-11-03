/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSWITCHTOWINDOWMENU_H
#define BERRYSWITCHTOWINDOWMENU_H

#include "berryContributionItem.h"

namespace berry {

struct IMenuManager;
struct IWorkbenchWindow;

/**
 * A dynamic menu item to switch to other opened workbench windows.
 */
class SwitchToWindowMenu : public QObject, public ContributionItem
{
  Q_OBJECT

private:

  static const int MAX_TEXT_LENGTH = 40;

  IWorkbenchWindow* workbenchWindow;

  QList<IWorkbenchWindow*> windows;

  bool showSeparator;

  bool dirty;

  Q_SLOT void AboutToShow(IMenuManager* manager);

  Q_SLOT void MenuItemTriggered(int index);

  /**
   * Returns the text for a window. This may be truncated to fit
   * within the MAX_TEXT_LENGTH.
   */
  QString CalcText(int number, IWorkbenchWindow* window);

public:

  /**
   * Creates a new instance of this class.
   *
   * @param window the workbench window this action applies to
   * @param showSeparator whether to add a separator in the menu
   */
  SwitchToWindowMenu(IWorkbenchWindow* window, const QString& id,
                     bool showSeparator);

  /**
   * Fills the given menu with menu items for all
   * opened workbench windows.
   */
  void Fill(QMenu* menu, QAction* before) override;

  using ContributionItem::Fill;

  /**
   * Overridden to always return true and force dynamic menu building.
   */
  bool IsDirty() const override;

  /**
   * Overridden to always return true and force dynamic menu building.
   */
  bool IsDynamic() const override;

};

}

#endif // BERRYSWITCHTOWINDOWMENU_H
