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

  void AddSelectionListener(QWidget* widget, GuiTk::ISelectionListener::Pointer listener);
  void RemoveSelectionListener(QWidget* widget, GuiTk::ISelectionListener::Pointer listener);

  void AddControlListener(QWidget* widget, GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(QWidget* widget, GuiTk::IControlListener::Pointer listener);

  bool GetEnabled(QWidget* widget);
  void SetEnabled(QWidget* widget, bool enabled);

  void SetBounds(QWidget* widget, const QRect& bounds);
  QRect GetBounds(QWidget* widget);

  void SetVisible(QWidget* widget, bool visible);
  bool GetVisible(QWidget* widget);
  bool IsVisible(QWidget* widget);

  QRect GetClientArea(QWidget* widget);

  QWidget* GetParent(QWidget* widget);
  bool SetParent(QWidget* widget, QWidget* parent);

  void SetData(QWidget* widget, const QString& id, Object::Pointer data);
  Object::Pointer GetData(QWidget* widget, const QString& id);

  //IMenu::Pointer CreateMenu(QWidget*, IMenu::Style = IMenu::POP_UP);
  //IMenu::Pointer CreateMenu(IMenu::Pointer parent);
  //IMenuItem::Pointer CreateMenuItem(IMenu::Pointer, IMenuItem::Style, int index = -1);

  QRect GetScreenSize(int i = -1);
  unsigned int GetScreenNumber();
  int GetPrimaryScreenNumber();
  QRect GetAvailableScreenSize(int i = -1);
  int GetClosestScreenNumber(const QRect&);

  QPoint GetCursorLocation();
  QWidget* GetCursorControl();
  QWidget* FindControl(const QList<Shell::Pointer>& shells, const QPoint& location);

  /**
   * Determines if one control is a child of another. Returns true iff the second
   * argument is a child of the first (or the same object).
   *
   * @param potentialParent
   * @param childToTest
   * @return
   */
  bool IsChild(QWidget* potentialParent, QWidget* childToTest);

  /**
   * Returns the control which currently has keyboard focus,
   * or null if keyboard events are not currently going to
   * any of the controls built by the currently running
   * application.
   *
   * @return the control under the cursor
   */
  QWidget* GetFocusControl();

  bool IsReparentable(QWidget* widget);

  void MoveAbove(QWidget* widgetToMove, QWidget* widget);
  void MoveBelow(QWidget* widgetToMove, QWidget* widget);

  void Dispose(QWidget* widget);

  Shell::Pointer CreateShell(Shell::Pointer parent, int style);
  void DisposeShell(Shell::Pointer shell);

  QWidget* CreateComposite(QWidget* parent);

  QList<Shell::Pointer> GetShells();
  Shell::Pointer GetShell(QWidget* widget);
  Shell::Pointer GetActiveShell();

  QRect ToControl(QWidget* coordinateSystem,
          const QRect& toConvert);

  QPoint ToControl(QWidget* coordinateSystem,
          const QPoint& toConvert);

  QRect ToDisplay(QWidget* coordinateSystem,
          const QRect& toConvert);

  QPoint ToDisplay(QWidget* coordinateSystem,
          const QPoint& toConvert);

private:

  QtWidgetsTweakletImpl impl;

};

}

#endif /* BERRYQTWIDGETSTWEAKLET_H_ */
