/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryQtWidgetsTweakletImpl.h"

#include "cherryQtSash.h"
#include "cherryQtShell.h"
#include "cherryQtControlWidget.h"

#include <cherryConstants.h>

#include <QAbstractButton>
#include <QDesktopWidget>
#include <QApplication>
#include <QVariant>


namespace cherry {

std::list<Shell::Pointer> QtWidgetsTweakletImpl::shellList = std::list<Shell::Pointer>();

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
  GuiTk::SelectionEvent::Pointer event(new GuiTk::SelectionEvent(widget));
  selectionEvents.selected(event);
}


void QtWidgetsTweakletImpl::AddSelectionListener(QWidget* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  if (widget == 0) return;

  // special handling for cherry::QtSash
  QtSash* sash = qobject_cast<QtSash*>(widget);
  if (sash != 0)
  {
    sash->AddSelectionListener(listener);
    return;
  }


  // "normal" Qt widgets get wrapped
  QtSelectionListenerWrapper* wrapper = selectionListenerMap[widget];
  if (wrapper == 0)
  {
    wrapper = new QtSelectionListenerWrapper(widget);
    selectionListenerMap[widget] = wrapper;
  }

  wrapper->AddListener(listener);
}

void QtWidgetsTweakletImpl::RemoveSelectionListener(QWidget* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  if (widget == 0) return;

  // special handling for cherry::QtSash
  QtSash* sash = qobject_cast<QtSash*>(widget);
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

Rectangle QtWidgetsTweakletImpl::GetScreenSize(int i)
{
  QDesktopWidget *desktop = QApplication::desktop();
  QRect screenGeometry;
  if (i < 0) screenGeometry = desktop->screen()->geometry();
  else screenGeometry = desktop->screenGeometry(i);

  return (Rectangle(screenGeometry.x(), screenGeometry.y()
    , screenGeometry.width(), screenGeometry.height()));
}

unsigned int QtWidgetsTweakletImpl::GetScreenNumber()
{
  QDesktopWidget *desktop = QApplication::desktop();
  // get the primary screen
  unsigned int numScreens = desktop->numScreens();
  return numScreens;
}

int QtWidgetsTweakletImpl::GetPrimaryScreenNumber()
{
  QDesktopWidget *desktop = QApplication::desktop();
  // get the primary screen
  int primaryScreenNr = desktop->primaryScreen();
  return primaryScreenNr;
}

void QtWidgetsTweakletImpl::AddControlListener(QtWidgetController* controller,
    GuiTk::IControlListener::Pointer listener)
{
  controller->AddControlListener(listener);
}

void QtWidgetsTweakletImpl::RemoveControlListener(QtWidgetController* controller,
    GuiTk::IControlListener::Pointer listener)
{
  controller->RemoveControlListener(listener);
}

bool QtWidgetsTweakletImpl::GetEnabled(QWidget* widget)
{
  return widget->isEnabled();
}

void QtWidgetsTweakletImpl::SetEnabled(QWidget* widget, bool enabled)
{
  widget->setEnabled(enabled);
}

void QtWidgetsTweakletImpl::SetBounds(QWidget* widget, const Rectangle& bounds)
{
  widget->setGeometry(bounds.x, bounds.y, bounds.width, bounds.height);
}

Rectangle QtWidgetsTweakletImpl::GetBounds(QWidget* widget)
{
  const QRect& geometry = widget->geometry();
  Rectangle rect(geometry.x(), geometry.y(), geometry.width(), geometry.height());
  return rect;
}

void QtWidgetsTweakletImpl::SetVisible(QWidget* widget, bool visible)
{
  widget->setVisible(visible);
}

bool QtWidgetsTweakletImpl::GetVisible(QWidget* widget)
{
  return !widget->isHidden();
}

bool QtWidgetsTweakletImpl::IsVisible(QWidget* widget)
{
  return widget->isVisible();
}

Rectangle QtWidgetsTweakletImpl::GetClientArea(QWidget* widget)
{
  const QRect& contentsRect = widget->contentsRect();
  Rectangle rect(contentsRect.x(), contentsRect.y(), contentsRect.width(), contentsRect.height());
  return rect;
}

void* QtWidgetsTweakletImpl::GetParent(QWidget* widget)
{
  return widget->parentWidget();
}

bool QtWidgetsTweakletImpl::SetParent(QWidget* widget, QWidget* parent)
{
  if (parent != widget->parentWidget())
  {
    widget->setParent(parent);
    return true;
  }
  return false;
}

void QtWidgetsTweakletImpl::SetData(QWidget* object, const std::string& id, Object::Pointer data)
{
  if (object == 0) return;

  QVariant variant;
  if (data != 0)
    variant.setValue(data);

  object->setProperty(id.c_str(), variant);
}

Object::Pointer QtWidgetsTweakletImpl::GetData(QWidget* object, const std::string& id)
{
  if (object == 0) return Object::Pointer(0);

  QVariant variant = object->property(id.c_str());
  if (variant.isValid())
  {
    return variant.value<Object::Pointer>();
  }
  return Object::Pointer(0);
}

Point QtWidgetsTweakletImpl::GetCursorLocation()
{
  QPoint qpoint = QCursor::pos();
  return Point(qpoint.x(), qpoint.y());
}

QWidget* QtWidgetsTweakletImpl::GetCursorControl()
{
  return QApplication::widgetAt(QCursor::pos());
}

QWidget* QtWidgetsTweakletImpl::FindControl(const std::vector<Shell::Pointer>& shells, const Point& location)
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

bool QtWidgetsTweakletImpl::IsChild(QObject* parentToTest, QObject* childToTest)
{
  bool found = false;
  QObject* parent = childToTest->parent();
  while (!found && parent != 0)
  {
    if (parent == parentToTest) found = true;
    parent = parent->parent();
  }

  return found;
}

QWidget* QtWidgetsTweakletImpl::GetFocusControl()
{
  return QApplication::focusWidget();
}

bool QtWidgetsTweakletImpl::IsReparentable(QWidget* /*widget*/)
{
  return true;
}

void QtWidgetsTweakletImpl::MoveAbove(QWidget* widgetToMove, QWidget* widget)
{
  widgetToMove->raise();
}

void QtWidgetsTweakletImpl::MoveBelow(QWidget* widgetToMove, QWidget* widget)
{
  widgetToMove->lower();
}

void QtWidgetsTweakletImpl::Dispose(QWidget* widget)
{
  delete widget;
}

Shell::Pointer QtWidgetsTweakletImpl::CreateShell(Shell::Pointer parent, int style)
{
  Qt::WindowFlags qtFlags(Qt::CustomizeWindowHint);

  if (style & Constants::MAX)
    qtFlags |= Qt::WindowMaximizeButtonHint;
  if (style & Constants::MIN)
    qtFlags |= Qt::WindowMinimizeButtonHint;
  if (style & Constants::CLOSE)
  {
    qtFlags |= Qt::WindowSystemMenuHint;
#if QT_VERSION >= 0x040500
    qtFlags |= Qt::WindowCloseButtonHint;
#endif
  }
  if (!(style & Constants::BORDER))
    qtFlags |= Qt::FramelessWindowHint;
  if (style & Constants::TITLE)
    qtFlags |= Qt::WindowTitleHint;
  if (style & Constants::TOOL)
    qtFlags |= Qt::Tool;

  QWidget* parentWidget = 0;
  if (parent != 0)
    parentWidget = static_cast<QWidget*>(parent->GetControl());

  QtShell* qtshell = new QtShell(parentWidget, qtFlags);
  Shell::Pointer shell(qtshell);
  shellList.push_back(shell);

  if ((style & Constants::APPLICATION_MODAL)
      || (style & Constants::SYSTEM_MODAL)) qtshell->GetWidget()->setWindowModality(Qt::ApplicationModal);
  if (style & Constants::PRIMARY_MODAL) qtshell->GetWidget()->setWindowModality(Qt::WindowModal);

  // we have to enable visibility for main windows to get a proper layout (see bug #1654)
  if (parentWidget == 0 || qtFlags.testFlag(Qt::Window))
  {
    qtshell->SetVisible(true);
  }

  return shell;
}

QWidget* QtWidgetsTweakletImpl::CreateComposite(QWidget* parent)
{
  QWidget* composite = new QtControlWidget(parent, Shell::Pointer(0));
  composite->setObjectName("created composite");
  return composite;
}

void QtWidgetsTweakletImpl::DisposeShell(Shell::Pointer shell)
{
  shellList.remove(shell);
}

std::vector<Shell::Pointer> QtWidgetsTweakletImpl::GetShells()
{
  std::vector<Shell::Pointer> shells(shellList.begin(), shellList.end());
  return shells;
}

Shell::Pointer QtWidgetsTweakletImpl::GetShell(QWidget* widget)
{
  QWidget* qwindow = widget->window();
  QVariant variant = qwindow->property(QtWidgetController::PROPERTY_ID);
  if (variant.isValid())
  {
    QtWidgetController::Pointer controller = variant.value<QtWidgetController::Pointer>();
    poco_assert(controller != 0);
    return controller->GetShell();
  }

  return Shell::Pointer(0);
}

Shell::Pointer QtWidgetsTweakletImpl::GetActiveShell()
{
  QWidget* qwidget = QApplication::activeWindow();

  if (qwidget == 0) return Shell::Pointer(0);

  QVariant variant = qwidget->property(QtWidgetController::PROPERTY_ID);
  if (variant.isValid())
  {
    return variant.value<QtWidgetController::Pointer>()->GetShell();
  }

  return Shell::Pointer(0);
}

Rectangle QtWidgetsTweakletImpl::ToControl(QWidget* coordinateSystem,
          const Rectangle& toConvert)
{
  QPoint globalUpperLeft(toConvert.x, toConvert.y);
  QPoint globalLowerRight(toConvert.x + toConvert.width, toConvert.y + toConvert.height);

  QPoint upperLeft = coordinateSystem->mapFromGlobal(globalUpperLeft);
  QPoint lowerRight = coordinateSystem->mapFromGlobal(globalLowerRight);

  return Rectangle(upperLeft.x(), upperLeft.y(), lowerRight.x() - upperLeft.x(),
            lowerRight.y() - upperLeft.y());
}

Point QtWidgetsTweakletImpl::ToControl(QWidget* coordinateSystem,
          const Point& toConvert)
{
  QPoint displayPoint(toConvert.x, toConvert.y);

  QPoint localPoint = coordinateSystem->mapFromGlobal(displayPoint);
  return Point(localPoint.x(), localPoint.y());
}

Rectangle QtWidgetsTweakletImpl::ToDisplay(QWidget* coordinateSystem,
        const Rectangle& toConvert)
{
  QPoint upperLeft(toConvert.x, toConvert.y);
  QPoint lowerRight(toConvert.x + toConvert.width, toConvert.y + toConvert.height);

  QPoint globalUpperLeft = coordinateSystem->mapToGlobal(upperLeft);
  QPoint globalLowerRight = coordinateSystem->mapToGlobal(lowerRight);

  return Rectangle(globalUpperLeft.x(), globalUpperLeft.y(), globalLowerRight.x() - globalUpperLeft.x(),
            globalLowerRight.y() - globalUpperLeft.y());
}

Point QtWidgetsTweakletImpl::ToDisplay(QWidget* coordinateSystem,
        const Point& toConvert)
{
  QPoint localPoint(toConvert.x, toConvert.y);
  QPoint displayPoint = coordinateSystem->mapToGlobal(localPoint);

  return Point(displayPoint.x(), displayPoint.y());
}

}
