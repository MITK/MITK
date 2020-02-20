/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYREOPENEDITORMENU_H
#define BERRYREOPENEDITORMENU_H

#include <berryContributionItem.h>

namespace berry {

struct IMenuManager;
struct IWorkbenchWindow;

class EditorHistory;
class EditorHistoryItem;

/**
 * A dynamic menu item which supports to switch to other Windows.
 */
class ReopenEditorMenu : public QObject, public ContributionItem
{
  Q_OBJECT

public:

  berryObjectMacro(ReopenEditorMenu);

  /**
   * Create a new instance.
   * @param window the window on which the menu is to be created
   * @param id menu's id
   * @param showSeparator whether or not to show a separator
   */
  ReopenEditorMenu(IWorkbenchWindow* window, const QString& id,
                   bool showSeparator);

  /**
   * Return a string suitable for a file MRU list.  This should not be called
   * outside the framework.
   *
   * @param index the index in the MRU list
   * @param name the file name
   * @param toolTip potentially the path
   * @param rtl should it be right-to-left
   * @return a string suitable for an MRU file menu
   */
  static QString CalcText(int index, const QString& name, const QString& toolTip, bool rtl);

  /**
   * Fills the given menu with
   * menu items for all windows.
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

private:

  /**
   * Reopens the editor for the given history item.
   */
  void Open(const SmartPointer<EditorHistoryItem>& item);

  /**
   * Returns the text for a history item.  This may be truncated to fit
   * within the MAX_TEXT_LENGTH.
   */
  static QString CalcText(int index, const SmartPointer<EditorHistoryItem>& item);

  IWorkbenchWindow* window;

  EditorHistory* history;

  bool showSeparator;

  bool dirty;

  Q_SLOT void MenuAboutToShow(IMenuManager* manager);

  // the maximum length for a file name; must be >= 4
  static const int MAX_TEXT_LENGTH = 40;

  // only assign mnemonic to the first nine items
  static const int MAX_MNEMONIC_SIZE = 9;

};

}
#endif // BERRYREOPENEDITORMENU_H
