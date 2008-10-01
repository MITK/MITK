
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

#include <QApplication>
#include <QAbstractButton>
#include <QRect>

namespace cherry
{

std::list<Shell::Pointer> QtWidgetsTweaklet::shellList = std::list<Shell::Pointer>();

QtSelectionListenerWrapper::QtSelectionListenerWrapper(QObject* w, GuiTk::ISelectionListener::Pointer l)
 : widget(w), listener(l)
{
  QAbstractButton* button = qobject_cast<QAbstractButton*>(widget);
  if (button != 0)
  {
    this->connect(button, "clicked(bool)", this, "QAbstractButtonClicked(bool)");
  }

  std::cout << "WARNING: QtWidgetsTweaklet: no suitable type for listening for selections found!\n";
}

void QtSelectionListenerWrapper::QAbstractButtonClicked(bool /*checked*/)
{
  GuiTk::SelectionEvent::Pointer event = new GuiTk::SelectionEvent(widget);
  listener->WidgetSelected(event);
}

bool QtWidgetsTweaklet::ContainsSelectionListener(void* widget, GuiTk::ISelectionListener::Pointer listener) const
{
  SelectionListenerMap::const_iterator iter = selectionListenerMap.find(widget);
  if (iter == selectionListenerMap.end()) return false;

  for (std::list<QtSelectionListenerWrapper*>::const_iterator wrapper = iter->second.begin();
       wrapper != iter->second.end(); ++wrapper)
  {
    if ((*wrapper)->widget == widget && (*wrapper)->listener == listener) return true;
  }

  return false;
}

void QtWidgetsTweaklet::AddSelectionListener(void* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  if (this->ContainsSelectionListener(widget, listener)) return;

  QObject* qobject = static_cast<QObject*>(widget);
  QtSelectionListenerWrapper* selectionWrapper = new QtSelectionListenerWrapper(qobject, listener);
  selectionListenerMap[widget].push_back(selectionWrapper);
}

void QtWidgetsTweaklet::RemoveSelectionListener(void* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  SelectionListenerMap::iterator iter = selectionListenerMap.find(widget);
  if (iter == selectionListenerMap.end()) return;

  for (std::list<QtSelectionListenerWrapper*>::iterator wrapper = iter->second.begin();
       wrapper != iter->second.end(); ++wrapper)
  {
    if ((*wrapper)->widget == widget && (*wrapper)->listener == listener)
    {
    iter->second.erase(wrapper);
    delete *wrapper;
    return;
    }
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
  std::cout << "Setting bounds for object " << qwidget->metaObject()->className() << " with name " << qPrintable(qwidget->objectName()) << ": x = " << bounds.x << ", y = " << bounds.y << ", width = " << bounds.width << ", height = " << bounds.height << std::endl;
  if (qwidget->objectName() == "Client Composite")
  {
    int i = 0;
    i++;
  }
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

void QtWidgetsTweaklet::Dispose(void* /*widget*/)
{
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
  std::cout << qwindow->metaObject()->className() << std::endl;
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

}
