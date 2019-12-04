/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef BERRYGUIWIDGETSTWEAKLET_H_
#define BERRYGUIWIDGETSTWEAKLET_H_

#include "internal/berryTweaklets.h"
#include "guitk/berryGuiTkISelectionListener.h"
#include "guitk/berryGuiTkIControlListener.h"
#include "berryShell.h"

//#include "commands/berryIMenu.h"
//#include "commands/berryIMenuItem.h"


namespace berry {

struct BERRY_UI_QT GuiWidgetsTweaklet
{

  static Tweaklets::TweakKey<GuiWidgetsTweaklet> KEY;

  virtual void AddSelectionListener(QWidget* widget, GuiTk::ISelectionListener::Pointer listener) = 0;
  virtual void RemoveSelectionListener(QWidget* widget, GuiTk::ISelectionListener::Pointer listener) = 0;

  /**
   * Adds the listener to the collection of listeners who will
   * be notified when the widget is moved or resized, by sending
   * it one of the messages defined in the <code>IControlListener</code>
   * interface.
   *
   * @param listener the listener which should be notified
   *
   * @see IControlListener
   * @see #RemoveControlListener
   */
  virtual void AddControlListener(QWidget* widget, GuiTk::IControlListener::Pointer listener) = 0;

  /**
   * Removes the listener from the collection of listeners who will
   * be notified when the widget is moved or resized.
   *
   * @param listener the listener which should no longer be notified
   *
   * @see IControlListener
   * @see #AddControlListener
   */
  virtual void RemoveControlListener(QWidget* widget, GuiTk::IControlListener::Pointer listener) = 0;

  virtual bool GetEnabled(QWidget* widget) = 0;
  virtual void SetEnabled(QWidget* widget, bool enabled) = 0;

  virtual void SetBounds(QWidget* widget, const QRect& bounds) = 0;
  virtual QRect GetBounds(QWidget* widget) = 0;

  virtual void SetVisible(QWidget* widget, bool visible) = 0;
  virtual bool GetVisible(QWidget* widget) = 0;
  virtual bool IsVisible(QWidget* widget) = 0;

  virtual QRect GetClientArea(QWidget* widget) = 0;

  virtual QWidget* GetParent(QWidget* widget) = 0;
  virtual bool SetParent(QWidget* widget, QWidget* parent) = 0;

  virtual void SetData(QWidget* widget, const QString& id, Object::Pointer data) = 0;
  virtual Object::Pointer GetData(QWidget* widget, const QString& id) = 0;

  virtual QPoint GetCursorLocation() = 0;
  virtual QWidget* GetCursorControl() = 0;
  virtual QWidget* FindControl(const QList<Shell::Pointer>& shells, const QPoint& location) = 0;

  /**
   * Determines if one control is a child of another. Returns true iff the second
   * argument is a child of the first (or the same object).
   *
   * @param potentialParent
   * @param childToTest
   * @return
   */
  virtual bool IsChild(QWidget* potentialParent, QWidget* childToTest) = 0;

  /**
   * Returns the control which currently has keyboard focus,
   * or null if keyboard events are not currently going to
   * any of the controls built by the currently running
   * application.
   *
   * @return the control under the cursor
   */
  virtual QWidget* GetFocusControl() = 0;

  virtual bool IsReparentable(QWidget* widget) = 0;

  virtual void MoveAbove(QWidget* widgetToMove, QWidget* widget) = 0;
  virtual void MoveBelow(QWidget* widgetToMove, QWidget* widget) = 0;

  virtual void Dispose(QWidget* widget) = 0;

  virtual Shell::Pointer CreateShell(Shell::Pointer parent, int style) = 0;
  virtual void DisposeShell(Shell::Pointer shell) = 0;

  virtual QWidget* CreateComposite(QWidget* parent) = 0;

  virtual QList<Shell::Pointer> GetShells() = 0;
  virtual Shell::Pointer GetShell(QWidget* widget) = 0;
  virtual Shell::Pointer GetActiveShell() = 0;

  // command framework interface classes
  //virtual IMenu::Pointer CreateMenu(QWidget*, IMenu::Style = IMenu::POP_UP) = 0;
  //virtual IMenu::Pointer CreateMenu(IMenu::Pointer parent) = 0;
  //virtual IMenuItem::Pointer CreateMenuItem(IMenu::Pointer, IMenuItem::Style, int index = -1) = 0;

  /**
   * @brief returns the coordinates of the center point of the primary screen
   * (where the application starts) of the current desktop.
   *
   * @param i the number of the screen (if there are multiple). If i = -1
   * a rectangle representing the size of the virtual desktop is returned.
   * @return the screen Geometry.
   * @see GetScreenNumber()
   * @see GetPrimaryScreenNumber()
   */
  virtual QRect GetScreenSize(int i = -1) = 0;

  virtual QRect GetAvailableScreenSize(int i = -1) = 0;

  virtual int GetClosestScreenNumber(const QRect&) = 0;

  /**
   * @brief Gets the number of available screens in a multi-screen environment.
   *
   * @return the number of available screens in a multi-screen environment.
   */
  virtual unsigned int GetScreenNumber() = 0;

  /**
   * @brief Gets the number of the primary screen.
   *
   * @return the number of the primary screen.
   */
  virtual int GetPrimaryScreenNumber() = 0;

  /**
   * Converts the given rectangle from display coordinates to the local coordinate system
   * of the given object
   *
   * @param coordinateSystem local coordinate system (widget) being converted to
   * @param toConvert rectangle to convert
   * @return a rectangle in control coordinates
   * @since 3.0
   */
  virtual QRect ToControl(QWidget* coordinateSystem,
          const QRect& toConvert) = 0;

  /**
   * Converts the given point from display coordinates to the local coordinate system
   * of the given object
   *
   * @param coordinateSystem local coordinate system (widget) being converted to
   * @param toConvert point to convert
   * @return a point in control coordinates
   * @since 3.0
   */
  virtual QPoint ToControl(QWidget* coordinateSystem,
          const QPoint& toConvert) = 0;

  /**
   * Converts the given rectangle from the local coordinate system of the given object
   * into display coordinates.
   *
   * @param coordinateSystem local coordinate system (widget) being converted from
   * @param toConvert rectangle to convert
   * @return a rectangle in display coordinates
   * @since 3.0
   */
  virtual QRect ToDisplay(QWidget* coordinateSystem,
          const QRect& toConvert) = 0;

  /**
   * Converts the given point from the local coordinate system of the given object
   * into display coordinates.
   *
   * @param coordinateSystem local coordinate system (widget) being converted from
   * @param toConvert point to convert
   * @return a point in display coordinates
   * @since 3.0
   */
  virtual QPoint ToDisplay(QWidget* coordinateSystem,
          const QPoint& toConvert) = 0;
};

}

Q_DECLARE_INTERFACE(berry::GuiWidgetsTweaklet, "org.blueberry.GuiWidgetsTweaklet")

#endif /* BERRYGUIWIDGETSTWEAKLET_H_ */
