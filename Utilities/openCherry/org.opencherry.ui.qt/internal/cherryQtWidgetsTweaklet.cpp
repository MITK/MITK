
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

#include "../cherryQtControlWidget.h"
#include "cherryQtMainWindowShell.h"
#include "cherryQtSash.h"

#include <QApplication>
#include <QAbstractButton>
#include <QCursor>
#include <QRect>
#include <QVariant>

namespace cherry
{

std::list<Shell::Pointer> QtWidgetsTweaklet::shellList = std::list<Shell::Pointer>();

QtSelectionListenerWrapper::QtSelectionListenerWrapper(QWidget* w)
 : widget(w)
{

}

void QtSelectionListenerWrapper::AddListener(GuiTk::ISelectionListener::Pointer listener)
{
  QAbstractButton* button = qobject_cast<QAbstractButton*>(widget);
  if (button != 0)
  {
    this->connect(button, "clicked(bool)", this, "QAbstractButtonClicked(bool)");
    selectionEvents.AddListener(listener);
  }

  std::cout << "WARNING: QtWidgetsTweaklet: no suitable type for listening for selections found!\n";

}

int QtSelectionListenerWrapper::RemoveListener(GuiTk::ISelectionListener::Pointer listener)
{
  selectionEvents.RemoveListener(listener);
  return std::max<int>(selectionEvents.selected.GetListeners().size(),
                  selectionEvents.defaultSelected.GetListeners().size());
}

void QtSelectionListenerWrapper::QAbstractButtonClicked(bool /*checked*/)
{
  GuiTk::SelectionEvent::Pointer event = new GuiTk::SelectionEvent(widget);
  selectionEvents.selected(event);
}

void QtWidgetsTweaklet::AddSelectionListener(void* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  if (qwidget == 0) return;

  // special handling for cherry::QtSash
  QtSash* sash = qobject_cast<QtSash*>(qwidget);
  if (sash != 0)
  {
    sash->AddSelectionListener(listener);
    return;
  }


  // "normal" Qt widgets get wrapped
  QtSelectionListenerWrapper* wrapper = selectionListenerMap[widget];
  if (wrapper == 0)
  {
    wrapper = new QtSelectionListenerWrapper(qwidget);
    selectionListenerMap[widget] = wrapper;
  }

  wrapper->AddListener(listener);
}

void QtWidgetsTweaklet::RemoveSelectionListener(void* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  if (qwidget == 0) return;

  // special handling for cherry::QtSash
  QtSash* sash = qobject_cast<QtSash*>(qwidget);
  if (sash != 0)
  {
    sash->RemoveSelectionListener(listener);
    return;
  }


  QtSelectionListenerWrapper* wrapper = selectionListenerMap[widget];
  if (wrapper == 0) return;

  if (wrapper->RemoveListener(listener) == 0)
  {
    selectionListenerMap.erase(wrapper);
    delete wrapper;
  }
}

void QtWidgetsTweaklet::AddControlListener(void* widget,
    GuiTk::IControlListener::Pointer listener)
{
  QtControlWidget* control = static_cast<QtControlWidget*>(widget);
  control->AddControlListener(listener);
}

void QtWidgetsTweaklet::RemoveControlListener(void* widget,
    GuiTk::IControlListener::Pointer listener)
{
  QtControlWidget* control = static_cast<QtControlWidget*>(widget);
  control->RemoveControlListener(listener);
}

bool QtWidgetsTweaklet::GetEnabled(void* widget)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  return qwidget->isEnabled();
}

void QtWidgetsTweaklet::SetEnabled(void* widget, bool enabled)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  qwidget->setEnabled(enabled);
}

void QtWidgetsTweaklet::SetBounds(void* widget, const Rectangle& bounds)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  qwidget->setGeometry(bounds.x, bounds.y, bounds.width, bounds.height);
}

Rectangle QtWidgetsTweaklet::GetBounds(void* widget)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  const QRect& geometry = qwidget->geometry();
  Rectangle rect(geometry.x(), geometry.y(), geometry.width(), geometry.height());
  return rect;
}

void QtWidgetsTweaklet::SetVisible(void* widget, bool visible)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  qwidget->setVisible(visible);
}

bool QtWidgetsTweaklet::GetVisible(void* widget)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  return !qwidget->isHidden();
}

bool QtWidgetsTweaklet::IsVisible(void* widget)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  return qwidget->isVisible();
}

Rectangle QtWidgetsTweaklet::GetClientArea(void* widget)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  const QRect& contentsRect = qwidget->contentsRect();
  Rectangle rect(contentsRect.x(), contentsRect.y(), contentsRect.width(), contentsRect.height());
  return rect;
}

void* QtWidgetsTweaklet::GetParent(void* widget)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  return qwidget->parent();
}

bool QtWidgetsTweaklet::SetParent(void* widget, void* parent)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  QWidget* qparent = static_cast<QWidget*>(parent);

  if (qparent != qwidget->parentWidget())
  {
    qwidget->setParent(qparent);
    return true;
  }
  return false;
}

void QtWidgetsTweaklet::SetData(void* widget, const std::string& id, Object::Pointer data)
{
  QObject* qobject = static_cast<QObject*>(widget);
  if (qobject == 0) return;

  QVariant variant;
  if (data != 0)
    variant.setValue(data);

  qobject->setProperty(id.c_str(), variant);
}

Object::Pointer QtWidgetsTweaklet::GetData(void* widget, const std::string& id)
{
  QObject* qobject = static_cast<QObject*>(widget);
  if (qobject == 0) return 0;

  QVariant variant = qobject->property(id.c_str());
  if (variant.isValid())
  {
    return variant.value<Object::Pointer>();
  }
  return 0;
}

Point QtWidgetsTweaklet::GetCursorLocation()
{
  QPoint qpoint = QCursor::pos();
  return Point(qpoint.x(), qpoint.y());
}

void* QtWidgetsTweaklet::GetCursorControl()
{
  return QApplication::widgetAt(QCursor::pos());
}

void* QtWidgetsTweaklet::FindControl(const std::vector<Shell::Pointer>& shells, const Point& location)
{
  for (std::vector<Shell::Pointer>::const_iterator iter = shells.begin();
      iter != shells.end(); ++iter)
  {
    QWidget* shellWidget = static_cast<QWidget*>((*iter)->GetControl());
    QWidget* control = shellWidget->childAt(location.x, location.y);
    if (control) return control;
  }

  return 0;
}

bool QtWidgetsTweaklet::IsChild(void* parentToTest, void* childToTest)
{
  QObject* child = static_cast<QObject*>(childToTest);
  QObject* potentialParent = static_cast<QObject*>(parentToTest);

  bool found = false;
  QObject* parent = child->parent();
  while (!found && parent != 0)
  {
    if (parent == potentialParent) found = true;
    parent = parent->parent();
  }

  return found;
}

void* QtWidgetsTweaklet::GetFocusControl()
{
  return QApplication::focusWidget();
}

bool QtWidgetsTweaklet::IsReparentable(void* /*widget*/)
{
  return true;
}

void QtWidgetsTweaklet::MoveAbove(void* widgetToMove, void* widget)
{
  QWidget* qwidget = static_cast<QWidget*>(widgetToMove);
  qwidget->raise();
}

void QtWidgetsTweaklet::MoveBelow(void* widgetToMove, void* widget)
{
  QWidget* qwidget = static_cast<QWidget*>(widgetToMove);
  qwidget->lower();
}

void QtWidgetsTweaklet::Dispose(void* widget)
{
  QObject* obj = static_cast<QObject*>(widget);
  delete obj;
}

Shell::Pointer QtWidgetsTweaklet::CreateShell(Shell::Pointer parent, int style)
{
  Qt::WindowFlags qtFlags;
  QWidget* parentWidget = 0;
  if (parent != 0)
    parentWidget = static_cast<QWidget*>(parent->GetControl());

  Shell::Pointer shell = new QtMainWindowShell(parentWidget, qtFlags & Qt::WA_DeleteOnClose);
  shellList.push_back(shell);

  return shell;
}

void* QtWidgetsTweaklet::CreateComposite(void* parent)
{
  QWidget* qParent = static_cast<QWidget*>(parent);
  QWidget* composite = new QtControlWidget(qParent);
  composite->setObjectName("created composite");
  return composite;
}

void QtWidgetsTweaklet::DisposeShell(Shell::Pointer shell)
{
  shellList.remove(shell);
}

std::vector<Shell::Pointer> QtWidgetsTweaklet::GetShells()
{
  std::vector<Shell::Pointer> shells(shellList.begin(), shellList.end());
  return shells;
}

Shell::Pointer QtWidgetsTweaklet::GetShell(void* widget)
{
  QWidget* qwidget = static_cast<QWidget*>(widget);
  QWidget* qwindow = qwidget->window();
  QtMainWindowShell* window = dynamic_cast<QtMainWindowShell*>(qwindow);
  Shell::Pointer shell = window;
  return shell;
}

Shell::Pointer QtWidgetsTweaklet::GetActiveShell()
{
  QtMainWindowShell* window = dynamic_cast<QtMainWindowShell*>(QApplication::activeWindow());
  Shell::Pointer shell = window;
  return shell;
}

Rectangle QtWidgetsTweaklet::ToControl(void* coordinateSystem,
          const Rectangle& toConvert)
{
  QWidget* widget = static_cast<QWidget*>(coordinateSystem);
  QPoint globalUpperLeft(toConvert.x, toConvert.y);
  QPoint globalLowerRight(toConvert.x + toConvert.width, toConvert.y + toConvert.height);

  QPoint upperLeft = widget->mapFromGlobal(globalUpperLeft);
  QPoint lowerRight = widget->mapFromGlobal(globalLowerRight);

  return Rectangle(upperLeft.x(), upperLeft.y(), lowerRight.x() - upperLeft.x(),
            lowerRight.y() - upperLeft.y());
}

Point QtWidgetsTweaklet::ToControl(void* coordinateSystem,
          const Point& toConvert)
{
  QWidget* widget = static_cast<QWidget*>(coordinateSystem);
  QPoint displayPoint(toConvert.x, toConvert.y);

  QPoint localPoint = widget->mapFromGlobal(displayPoint);
  return Point(localPoint.x(), localPoint.y());
}

Rectangle QtWidgetsTweaklet::ToDisplay(void* coordinateSystem,
        const Rectangle& toConvert)
{
  QWidget* widget = static_cast<QWidget*>(coordinateSystem);
  QPoint upperLeft(toConvert.x, toConvert.y);
  QPoint lowerRight(toConvert.x + toConvert.width, toConvert.y + toConvert.height);

  QPoint globalUpperLeft = widget->mapToGlobal(upperLeft);
  QPoint globalLowerRight = widget->mapToGlobal(lowerRight);

  return Rectangle(globalUpperLeft.x(), globalUpperLeft.y(), globalLowerRight.x() - globalUpperLeft.x(),
            globalLowerRight.y() - globalUpperLeft.y());
}

Point QtWidgetsTweaklet::ToDisplay(void* coordinateSystem,
        const Point& toConvert)
{
  QWidget* widget = static_cast<QWidget*>(coordinateSystem);
  QPoint localPoint(toConvert.x, toConvert.y);
  QPoint displayPoint = widget->mapToGlobal(localPoint);

  return Point(displayPoint.x(), displayPoint.y());
}

}
