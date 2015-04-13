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
  void Fill(QMenu* menu, QAction* before);

  using ContributionItem::Fill;

  /**
   * Overridden to always return true and force dynamic menu building.
   */
  bool IsDirty() const;

  /**
   * Overridden to always return true and force dynamic menu building.
   */
  bool IsDynamic() const;

};

}

#endif // BERRYSWITCHTOWINDOWMENU_H
