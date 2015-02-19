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

  void AddSelectionListener(void* widget, GuiTk::ISelectionListener::Pointer listener);
  void RemoveSelectionListener(void* widget, GuiTk::ISelectionListener::Pointer listener);

  void AddControlListener(void* widget, GuiTk::IControlListener::Pointer listener);
  void RemoveControlListener(void* widget, GuiTk::IControlListener::Pointer listener);

  bool GetEnabled(void* widget);
  void SetEnabled(void* widget, bool enabled);

  void SetBounds(void* widget, const QRect& bounds);
  QRect GetBounds(void* widget);

  void SetVisible(void* widget, bool visible);
  bool GetVisible(void* widget);
  bool IsVisible(void* widget);

  QRect GetClientArea(void* widget);

  void* GetParent(void* widget);
  bool SetParent(void* widget, void* parent);

  void SetData(void* widget, const QString& id, Object::Pointer data);
  Object::Pointer GetData(void* widget, const QString& id);

  //IMenu::Pointer CreateMenu(void*, IMenu::Style = IMenu::POP_UP);
  //IMenu::Pointer CreateMenu(IMenu::Pointer parent);
  //IMenuItem::Pointer CreateMenuItem(IMenu::Pointer, IMenuItem::Style, int index = -1);

  QRect GetScreenSize(int i = -1);
  unsigned int GetScreenNumber();
  int GetPrimaryScreenNumber();
  QRect GetAvailableScreenSize(int i = -1);
  int GetClosestScreenNumber(const QRect&);

  QPoint GetCursorLocation();
  void* GetCursorControl();
  void* FindControl(const QList<Shell::Pointer>& shells, const QPoint& location);

  /**
   * Determines if one control is a child of another. Returns true iff the second
   * argument is a child of the first (or the same object).
   *
   * @param potentialParent
   * @param childToTest
   * @return
   */
  bool IsChild(void* potentialParent, void* childToTest);

  /**
   * Returns the control which currently has keyboard focus,
   * or null if keyboard events are not currently going to
   * any of the controls built by the currently running
   * application.
   *
   * @return the control under the cursor
   */
  void* GetFocusControl();

  bool IsReparentable(void* widget);

  void MoveAbove(void* widgetToMove, void* widget);
  void MoveBelow(void* widgetToMove, void* widget);

  void Dispose(void* widget);

  Shell::Pointer CreateShell(Shell::Pointer parent, int style);
  void DisposeShell(Shell::Pointer shell);

  void* CreateComposite(void* parent);

  QList<Shell::Pointer> GetShells();
  Shell::Pointer GetShell(void* widget);
  Shell::Pointer GetActiveShell();

  QRect ToControl(void* coordinateSystem,
          const QRect& toConvert);

  QPoint ToControl(void* coordinateSystem,
          const QPoint& toConvert);

  QRect ToDisplay(void* coordinateSystem,
          const QRect& toConvert);

  QPoint ToDisplay(void* coordinateSystem,
          const QPoint& toConvert);

private:

  QtWidgetsTweakletImpl impl;

};

}

#endif /* BERRYQTWIDGETSTWEAKLET_H_ */
