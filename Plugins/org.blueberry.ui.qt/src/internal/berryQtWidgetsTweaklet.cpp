
/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryQtWidgetsTweaklet.h"

#include <QWidget>
#include <QVariant>

namespace berry
{

QtWidgetsTweaklet::QtWidgetsTweaklet()
{

}

void QtWidgetsTweaklet::AddSelectionListener(QWidget* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  impl.AddSelectionListener(static_cast<QWidget*>(widget), listener);
}

void QtWidgetsTweaklet::RemoveSelectionListener(QWidget* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  impl.RemoveSelectionListener(static_cast<QWidget*>(widget), listener);
}

QRect QtWidgetsTweaklet::GetScreenSize(int i)
{
  return impl.GetScreenSize(i);
}

unsigned int QtWidgetsTweaklet::GetScreenNumber()
{
  return impl.GetScreenNumber();
}

int QtWidgetsTweaklet::GetPrimaryScreenNumber()
{
  return impl.GetPrimaryScreenNumber();
}

QRect QtWidgetsTweaklet::GetAvailableScreenSize(int i)
{
  return impl.GetAvailableScreenSize(i);
}

int QtWidgetsTweaklet::GetClosestScreenNumber(const QRect& rect)
{
  return impl.GetClosestScreenNumber(rect);
}

//IMenu::Pointer QtWidgetsTweaklet::CreateMenu(QWidget*, IMenu::Style style)
//{
//  //TODO Qt CreateMenu
//  return IMenu::Pointer(0);
//}

//IMenu::Pointer QtWidgetsTweaklet::CreateMenu(IMenu::Pointer parent)
//{
//  //TODO Qt CreateMenu
//  return IMenu::Pointer(0);
//}

//IMenuItem::Pointer QtWidgetsTweaklet::CreateMenuItem(IMenu::Pointer, IMenuItem::Style, int index)
//{
//  //TODO Qt CreateMenuItem
//  return IMenuItem::Pointer(0);
//}

void QtWidgetsTweaklet::AddControlListener(QWidget* widget,
    GuiTk::IControlListener::Pointer listener)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  QVariant variant = qwidget->property(QtWidgetController::PROPERTY_ID);
  if (variant.isValid())
  {
    QtWidgetController::Pointer controller = variant.value<QtWidgetController::Pointer>();
    if (controller != 0)
      impl.AddControlListener(controller.GetPointer(), listener);
  }
}

void QtWidgetsTweaklet::RemoveControlListener(QWidget* widget,
    GuiTk::IControlListener::Pointer listener)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  QVariant variant = qwidget->property(QtWidgetController::PROPERTY_ID);
  if (variant.isValid())
  {
    QtWidgetController::Pointer controller = variant.value<QtWidgetController::Pointer>();
    if (controller != 0)
      impl.RemoveControlListener(controller.GetPointer(), listener);
  }
}

bool QtWidgetsTweaklet::GetEnabled(QWidget* widget)
{
  return impl.GetEnabled(static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::SetEnabled(QWidget* widget, bool enabled)
{
  impl.SetEnabled(static_cast<QWidget*>(widget), enabled);
}

void QtWidgetsTweaklet::SetBounds(QWidget* widget, const QRect& bounds)
{
  impl.SetBounds(static_cast<QWidget*>(widget), bounds);
}

QRect QtWidgetsTweaklet::GetBounds(QWidget* widget)
{
  return impl.GetBounds(static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::SetVisible(QWidget* widget, bool visible)
{
  impl.SetVisible(static_cast<QWidget*>(widget), visible);
}

bool QtWidgetsTweaklet::GetVisible(QWidget* widget)
{
  return impl.GetVisible(static_cast<QWidget*>(widget));
}

bool QtWidgetsTweaklet::IsVisible(QWidget* widget)
{
  return impl.IsVisible(static_cast<QWidget*>(widget));
}

QRect QtWidgetsTweaklet::GetClientArea(QWidget* widget)
{
  return impl.GetClientArea(static_cast<QWidget*>(widget));
}

QWidget* QtWidgetsTweaklet::GetParent(QWidget* widget)
{
  return impl.GetParent(static_cast<QWidget*>(widget));
}

bool QtWidgetsTweaklet::SetParent(QWidget* widget, QWidget* parent)
{
  return impl.SetParent(static_cast<QWidget*>(widget),
                        static_cast<QWidget*>(parent));
}

void QtWidgetsTweaklet::SetData(QWidget* widget, const QString& id, Object::Pointer data)
{
  impl.SetData(static_cast<QWidget*>(widget), id, data);
}

Object::Pointer QtWidgetsTweaklet::GetData(QWidget* widget, const QString& id)
{
  return impl.GetData(static_cast<QWidget*>(widget), id);
}

QPoint QtWidgetsTweaklet::GetCursorLocation()
{
  return impl.GetCursorLocation();
}

QWidget* QtWidgetsTweaklet::GetCursorControl()
{
  return impl.GetCursorControl();
}

QWidget* QtWidgetsTweaklet::FindControl(const QList<Shell::Pointer>& shells, const QPoint& location)
{
  return impl.FindControl(shells, location);
}

bool QtWidgetsTweaklet::IsChild(QWidget* parentToTest, QWidget* childToTest)
{
  return impl.IsChild(static_cast<QObject*>(parentToTest),
                      static_cast<QObject*>(childToTest));
}

QWidget* QtWidgetsTweaklet::GetFocusControl()
{
  return impl.GetFocusControl();
}

bool QtWidgetsTweaklet::IsReparentable(QWidget* widget)
{
  return impl.IsReparentable(static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::MoveAbove(QWidget* widgetToMove, QWidget* widget)
{
  impl.MoveAbove(static_cast<QWidget*>(widgetToMove),
                 static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::MoveBelow(QWidget* widgetToMove, QWidget* widget)
{
  impl.MoveBelow(static_cast<QWidget*>(widgetToMove),
                 static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::Dispose(QWidget* widget)
{
  impl.Dispose(static_cast<QWidget*>(widget));
}

Shell::Pointer QtWidgetsTweaklet::CreateShell(Shell::Pointer parent, int style)
{
  return impl.CreateShell(parent, style);
}

QWidget* QtWidgetsTweaklet::CreateComposite(QWidget* parent)
{
  return impl.CreateComposite(static_cast<QWidget*>(parent));
}

void QtWidgetsTweaklet::DisposeShell(Shell::Pointer shell)
{
  impl.DisposeShell(shell);
}

QList<Shell::Pointer> QtWidgetsTweaklet::GetShells()
{
  return impl.GetShells();
}

Shell::Pointer QtWidgetsTweaklet::GetShell(QWidget* widget)
{
  return impl.GetShell(static_cast<QWidget*>(widget));
}

Shell::Pointer QtWidgetsTweaklet::GetActiveShell()
{
  return impl.GetActiveShell();
}

QRect QtWidgetsTweaklet::ToControl(QWidget* coordinateSystem,
          const QRect& toConvert)
{
  return impl.ToControl(static_cast<QWidget*>(coordinateSystem), toConvert);
}

QPoint QtWidgetsTweaklet::ToControl(QWidget* coordinateSystem,
          const QPoint& toConvert)
{
  return impl.ToControl(static_cast<QWidget*>(coordinateSystem), toConvert);
}

QRect QtWidgetsTweaklet::ToDisplay(QWidget* coordinateSystem,
        const QRect& toConvert)
{
  return impl.ToDisplay(static_cast<QWidget*>(coordinateSystem), toConvert);
}

QPoint QtWidgetsTweaklet::ToDisplay(QWidget* coordinateSystem,
        const QPoint& toConvert)
{
  return impl.ToDisplay(static_cast<QWidget*>(coordinateSystem), toConvert);
}

}
