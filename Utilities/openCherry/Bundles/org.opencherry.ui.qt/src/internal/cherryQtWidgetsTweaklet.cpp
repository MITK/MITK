
/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY { } without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryQtWidgetsTweaklet.h"

#include <QWidget>
#include <QVariant>

namespace cherry
{

void QtWidgetsTweaklet::AddSelectionListener(void* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  impl.AddSelectionListener(static_cast<QWidget*>(widget), listener);
}

void QtWidgetsTweaklet::RemoveSelectionListener(void* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  impl.RemoveSelectionListener(static_cast<QWidget*>(widget), listener);
}

Rectangle QtWidgetsTweaklet::GetScreenSize(int i)
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

IMenu::Pointer QtWidgetsTweaklet::CreateMenu(void*, IMenu::Style style)
{
  //TODO Qt CreateMenu
  return IMenu::Pointer(0);
}

IMenu::Pointer QtWidgetsTweaklet::CreateMenu(IMenu::Pointer parent)
{
  //TODO Qt CreateMenu
  return IMenu::Pointer(0);
}

IMenuItem::Pointer QtWidgetsTweaklet::CreateMenuItem(IMenu::Pointer, IMenuItem::Style, int index)
{
  //TODO Qt CreateMenuItem
  return IMenuItem::Pointer(0);
}

void QtWidgetsTweaklet::AddControlListener(void* widget,
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

void QtWidgetsTweaklet::RemoveControlListener(void* widget,
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

bool QtWidgetsTweaklet::GetEnabled(void* widget)
{
  return impl.GetEnabled(static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::SetEnabled(void* widget, bool enabled)
{
  impl.SetEnabled(static_cast<QWidget*>(widget), enabled);
}

void QtWidgetsTweaklet::SetBounds(void* widget, const Rectangle& bounds)
{
  impl.SetBounds(static_cast<QWidget*>(widget), bounds);
}

Rectangle QtWidgetsTweaklet::GetBounds(void* widget)
{
  return impl.GetBounds(static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::SetVisible(void* widget, bool visible)
{
  impl.SetVisible(static_cast<QWidget*>(widget), visible);
}

bool QtWidgetsTweaklet::GetVisible(void* widget)
{
  return impl.GetVisible(static_cast<QWidget*>(widget));
}

bool QtWidgetsTweaklet::IsVisible(void* widget)
{
  return impl.IsVisible(static_cast<QWidget*>(widget));
}

Rectangle QtWidgetsTweaklet::GetClientArea(void* widget)
{
  return impl.GetClientArea(static_cast<QWidget*>(widget));
}

void* QtWidgetsTweaklet::GetParent(void* widget)
{
  return impl.GetParent(static_cast<QWidget*>(widget));
}

bool QtWidgetsTweaklet::SetParent(void* widget, void* parent)
{
  return impl.SetParent(static_cast<QWidget*>(widget),
                        static_cast<QWidget*>(parent));
}

void QtWidgetsTweaklet::SetData(void* widget, const std::string& id, Object::Pointer data)
{
  impl.SetData(static_cast<QWidget*>(widget), id, data);
}

Object::Pointer QtWidgetsTweaklet::GetData(void* widget, const std::string& id)
{
  return impl.GetData(static_cast<QWidget*>(widget), id);
}

Point QtWidgetsTweaklet::GetCursorLocation()
{
  return impl.GetCursorLocation();
}

void* QtWidgetsTweaklet::GetCursorControl()
{
  return impl.GetCursorControl();
}

void* QtWidgetsTweaklet::FindControl(const std::vector<Shell::Pointer>& shells, const Point& location)
{
  return impl.FindControl(shells, location);
}

bool QtWidgetsTweaklet::IsChild(void* parentToTest, void* childToTest)
{
  return impl.IsChild(static_cast<QObject*>(parentToTest),
                      static_cast<QObject*>(childToTest));
}

void* QtWidgetsTweaklet::GetFocusControl()
{
  return impl.GetFocusControl();
}

bool QtWidgetsTweaklet::IsReparentable(void* widget)
{
  return impl.IsReparentable(static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::MoveAbove(void* widgetToMove, void* widget)
{
  impl.MoveAbove(static_cast<QWidget*>(widgetToMove),
                 static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::MoveBelow(void* widgetToMove, void* widget)
{
  impl.MoveBelow(static_cast<QWidget*>(widgetToMove),
                 static_cast<QWidget*>(widget));
}

void QtWidgetsTweaklet::Dispose(void* widget)
{
  impl.Dispose(static_cast<QWidget*>(widget));
}

Shell::Pointer QtWidgetsTweaklet::CreateShell(Shell::Pointer parent, int style)
{
  return impl.CreateShell(parent, style);
}

void* QtWidgetsTweaklet::CreateComposite(void* parent)
{
  return impl.CreateComposite(static_cast<QWidget*>(parent));
}

void QtWidgetsTweaklet::DisposeShell(Shell::Pointer shell)
{
  impl.DisposeShell(shell);
}

std::vector<Shell::Pointer> QtWidgetsTweaklet::GetShells()
{
  return impl.GetShells();
}

Shell::Pointer QtWidgetsTweaklet::GetShell(void* widget)
{
  return impl.GetShell(static_cast<QWidget*>(widget));
}

Shell::Pointer QtWidgetsTweaklet::GetActiveShell()
{
  return impl.GetActiveShell();
}

Rectangle QtWidgetsTweaklet::ToControl(void* coordinateSystem,
          const Rectangle& toConvert)
{
  return impl.ToControl(static_cast<QWidget*>(coordinateSystem), toConvert);
}

Point QtWidgetsTweaklet::ToControl(void* coordinateSystem,
          const Point& toConvert)
{
  return impl.ToControl(static_cast<QWidget*>(coordinateSystem), toConvert);
}

Rectangle QtWidgetsTweaklet::ToDisplay(void* coordinateSystem,
        const Rectangle& toConvert)
{
  return impl.ToDisplay(static_cast<QWidget*>(coordinateSystem), toConvert);
}

Point QtWidgetsTweaklet::ToDisplay(void* coordinateSystem,
        const Point& toConvert)
{
  return impl.ToDisplay(static_cast<QWidget*>(coordinateSystem), toConvert);
}

}
