/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYQTWIDGETSTWEAKLET_H_
#define BERRYQTWIDGETSTWEAKLET_H_

#include <berryGuiWidgetsTweaklet.h>

#include "berryQtWidgetsTweakletImpl.h"

namespace berry {


class QtWidgetsTweaklet : public QObject, public GuiWidgetsTweaklet
{
  Q_OBJECT
  Q_INTERFACES(berry::GuiWidgetsTweaklet)

public:

  QtWidgetsTweaklet();

  void AddSelectionListener(QWidget* widget, GuiTk::ISelectionListener::Pointer listener) override;
  void RemoveSelectionListener(QWidget* widget, GuiTk::ISelectionListener::Pointer listener) override;

  void AddControlListener(QWidget* widget, GuiTk::IControlListener::Pointer listener) override;
  void RemoveControlListener(QWidget* widget, GuiTk::IControlListener::Pointer listener) override;

  bool GetEnabled(QWidget* widget) override;
  void SetEnabled(QWidget* widget, bool enabled) override;

  void SetBounds(QWidget* widget, const QRect& bounds) override;
  QRect GetBounds(QWidget* widget) override;

  void SetVisible(QWidget* widget, bool visible) override;
  bool GetVisible(QWidget* widget) override;
  bool IsVisible(QWidget* widget) override;

  QRect GetClientArea(QWidget* widget) override;

  QWidget* GetParent(QWidget* widget) override;
  bool SetParent(QWidget* widget, QWidget* parent) override;

  void SetData(QWidget* widget, const QString& id, Object::Pointer data) override;
  Object::Pointer GetData(QWidget* widget, const QString& id) override;

  //IMenu::Pointer CreateMenu(QWidget*, IMenu::Style = IMenu::POP_UP);
  //IMenu::Pointer CreateMenu(IMenu::Pointer parent);
  //IMenuItem::Pointer CreateMenuItem(IMenu::Pointer, IMenuItem::Style, int index = -1);

  QRect GetScreenSize(int i = -1) override;
  unsigned int GetScreenNumber() override;
  int GetPrimaryScreenNumber() override;
  QRect GetAvailableScreenSize(int i = -1) override;
  int GetClosestScreenNumber(const QRect&) override;

  QPoint GetCursorLocation() override;
  QWidget* GetCursorControl() override;
  QWidget* FindControl(const QList<Shell::Pointer>& shells, const QPoint& location) override;

  /**
   * Determines if one control is a child of another. Returns true iff the second
   * argument is a child of the first (or the same object).
   *
   * @param potentialParent
   * @param childToTest
   * @return
   */
  bool IsChild(QWidget* potentialParent, QWidget* childToTest) override;

  /**
   * Returns the control which currently has keyboard focus,
   * or null if keyboard events are not currently going to
   * any of the controls built by the currently running
   * application.
   *
   * @return the control under the cursor
   */
  QWidget* GetFocusControl() override;

  bool IsReparentable(QWidget* widget) override;

  void MoveAbove(QWidget* widgetToMove, QWidget* widget) override;
  void MoveBelow(QWidget* widgetToMove, QWidget* widget) override;

  void Dispose(QWidget* widget) override;

  Shell::Pointer CreateShell(Shell::Pointer parent, int style) override;
  void DisposeShell(Shell::Pointer shell) override;

  QWidget* CreateComposite(QWidget* parent) override;

  QList<Shell::Pointer> GetShells() override;
  Shell::Pointer GetShell(QWidget* widget) override;
  Shell::Pointer GetActiveShell() override;

  QRect ToControl(QWidget* coordinateSystem,
          const QRect& toConvert) override;

  QPoint ToControl(QWidget* coordinateSystem,
          const QPoint& toConvert) override;

  QRect ToDisplay(QWidget* coordinateSystem,
          const QRect& toConvert) override;

  QPoint ToDisplay(QWidget* coordinateSystem,
          const QPoint& toConvert) override;

private:

  QtWidgetsTweakletImpl impl;

};

}

#endif /* BERRYQTWIDGETSTWEAKLET_H_ */
