/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryLog.h"

#include "berryQtWidgetsTweakletImpl.h"

#include "berryQtSash.h"
#include "berryQtShell.h"
#include <internal/berryQtControlWidget.h>

#include <berryConstants.h>

#include <QAbstractButton>
#include <QDesktopWidget>
#include <QApplication>
#include <QVariant>


namespace berry {

QtSelectionListenerWrapper::QtSelectionListenerWrapper(QWidget* w)
 : widget(w)
{

}

void QtSelectionListenerWrapper::AddListener(GuiTk::ISelectionListener::Pointer listener)
{
  QAbstractButton* button = qobject_cast<QAbstractButton*>(widget);
  if (button != nullptr)
  {
    this->connect(button, "clicked(bool)", this, "QAbstractButtonClicked(bool)");
    selectionEvents.AddListener(listener);
  }

  BERRY_WARN << "WARNING: QtWidgetsTweaklet: no suitable type for listening for selections found!\n";

}

int QtSelectionListenerWrapper::RemoveListener(GuiTk::ISelectionListener::Pointer listener)
{
  selectionEvents.RemoveListener(listener);
  return static_cast<int>(std::max<std::size_t>(selectionEvents.selected.GetListeners().size(),
                  selectionEvents.defaultSelected.GetListeners().size()));
}

void QtSelectionListenerWrapper::QAbstractButtonClicked(bool /*checked*/)
{
  GuiTk::SelectionEvent::Pointer event(new GuiTk::SelectionEvent(widget));
  selectionEvents.selected(event);
}


void QtWidgetsTweakletImpl::AddSelectionListener(QWidget* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  if (widget == nullptr) return;

  // special handling for berry::QtSash
  QtSash* sash = qobject_cast<QtSash*>(widget);
  if (sash != nullptr)
  {
    sash->AddSelectionListener(listener);
    return;
  }


  // "normal" Qt widgets get wrapped
  QtSelectionListenerWrapper* wrapper = selectionListenerMap[widget];
  if (wrapper == nullptr)
  {
    wrapper = new QtSelectionListenerWrapper(widget);
    selectionListenerMap[widget] = wrapper;
  }

  wrapper->AddListener(listener);
}

void QtWidgetsTweakletImpl::RemoveSelectionListener(QWidget* widget,
    GuiTk::ISelectionListener::Pointer listener)
{
  if (widget == nullptr) return;

  // special handling for berry::QtSash
  QtSash* sash = qobject_cast<QtSash*>(widget);
  if (sash != nullptr)
  {
    sash->RemoveSelectionListener(listener);
    return;
  }


  QtSelectionListenerWrapper* wrapper = selectionListenerMap[widget];
  if (wrapper == nullptr) return;

  if (wrapper->RemoveListener(listener) == 0)
  {
    selectionListenerMap.remove(widget);
    delete wrapper;
  }
}

QRect QtWidgetsTweakletImpl::GetScreenSize(int i)
{
  QDesktopWidget *desktop = QApplication::desktop();
  if (i < 0) return desktop->screen()->geometry();
  return desktop->screenGeometry(i);
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

QRect QtWidgetsTweakletImpl::GetAvailableScreenSize(int i)
{
  QDesktopWidget *desktop = QApplication::desktop();
  if (i < 0) return desktop->screen()->geometry();
  return desktop->availableGeometry(i);
}

int QtWidgetsTweakletImpl::GetClosestScreenNumber(const QRect& r)
{
  QDesktopWidget *desktop = QApplication::desktop();
  return desktop->screenNumber(QPoint(r.x() + r.width()/2, r.y() + r.height()/2));
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

void QtWidgetsTweakletImpl::SetBounds(QWidget* widget, const QRect& bounds)
{
  widget->setGeometry(bounds);
}

QRect QtWidgetsTweakletImpl::GetBounds(QWidget* widget)
{
  const QRect& geometry = widget->geometry();
  QRect rect(geometry.x(), geometry.y(), geometry.width(), geometry.height());
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

QRect QtWidgetsTweakletImpl::GetClientArea(QWidget* widget)
{
  return widget->contentsRect();
}

QWidget* QtWidgetsTweakletImpl::GetParent(QWidget* widget)
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

void QtWidgetsTweakletImpl::SetData(QWidget* object, const QString& id, Object::Pointer data)
{
  if (object == nullptr) return;

  QVariant variant;
  if (data != 0)
    variant.setValue(data);

  object->setProperty(qPrintable(id), variant);
}

Object::Pointer QtWidgetsTweakletImpl::GetData(QWidget* object, const QString& id)
{
  if (object == nullptr) return Object::Pointer(nullptr);

  QVariant variant = object->property(qPrintable(id));
  if (variant.isValid())
  {
    return variant.value<Object::Pointer>();
  }
  return Object::Pointer(nullptr);
}

QPoint QtWidgetsTweakletImpl::GetCursorLocation()
{
  QPoint qpoint = QCursor::pos();
  return QPoint(qpoint.x(), qpoint.y());
}

QWidget* QtWidgetsTweakletImpl::GetCursorControl()
{
  return QApplication::widgetAt(QCursor::pos());
}

QWidget* QtWidgetsTweakletImpl::FindControl(const QList<Shell::Pointer>& shells, const QPoint& location)
{
  for (QList<Shell::Pointer>::const_iterator iter = shells.begin();
      iter != shells.end(); ++iter)
  {
    QWidget* shellWidget = static_cast<QWidget*>((*iter)->GetControl());
    QWidget* control = shellWidget->childAt(location.x(), location.y());
    if (control) return control;
  }

  return nullptr;
}

bool QtWidgetsTweakletImpl::IsChild(QObject* parentToTest, QObject* childToTest)
{
  bool found = false;
  QObject* parent = childToTest->parent();
  while (!found && parent != nullptr)
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

void QtWidgetsTweakletImpl::MoveAbove(QWidget* widgetToMove, QWidget*  /*widget*/)
{
  widgetToMove->raise();
}

void QtWidgetsTweakletImpl::MoveBelow(QWidget* widgetToMove, QWidget*  /*widget*/)
{
  widgetToMove->lower();
}

void QtWidgetsTweakletImpl::Dispose(QWidget* widget)
{
  delete widget;
  widget = nullptr;
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
    qtFlags |= Qt::WindowCloseButtonHint;
  }
  if (!(style & Constants::BORDER))
    qtFlags |= Qt::FramelessWindowHint;
  if (style & Constants::TITLE)
    qtFlags |= Qt::WindowTitleHint;
  if (style & Constants::TOOL)
    qtFlags |= Qt::Tool;

  QWidget* parentWidget = nullptr;
  if (parent != 0)
    parentWidget = static_cast<QWidget*>(parent->GetControl());

  auto   qtshell = new QtShell(parentWidget, qtFlags);
  Shell::Pointer shell(qtshell);
  shellList.push_back(shell);

  if ((style & Constants::APPLICATION_MODAL)
      || (style & Constants::SYSTEM_MODAL)) qtshell->GetWidget()->setWindowModality(Qt::ApplicationModal);
  if (style & Constants::PRIMARY_MODAL) qtshell->GetWidget()->setWindowModality(Qt::WindowModal);

  return shell;
}

QWidget* QtWidgetsTweakletImpl::CreateComposite(QWidget* parent)
{
  QWidget* composite = new QtControlWidget(parent, nullptr);
  composite->setObjectName("created composite");
  return composite;
}

void QtWidgetsTweakletImpl::DisposeShell(Shell::Pointer shell)
{
  shellList.removeAll(shell);
  shell->SetVisible(false);
}

QList<Shell::Pointer> QtWidgetsTweakletImpl::GetShells()
{
  return shellList;
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

  return Shell::Pointer(nullptr);
}

Shell::Pointer QtWidgetsTweakletImpl::GetActiveShell()
{
  QWidget* qwidget = QApplication::activeWindow();

  if (qwidget == nullptr) return Shell::Pointer(nullptr);

  QVariant variant = qwidget->property(QtWidgetController::PROPERTY_ID);
  if (variant.isValid())
  {
    return variant.value<QtWidgetController::Pointer>()->GetShell();
  }

  return Shell::Pointer(nullptr);
}

QRect QtWidgetsTweakletImpl::ToControl(QWidget* coordinateSystem,
          const QRect& toConvert)
{
  QPoint globalUpperLeft = toConvert.topLeft();
  QPoint globalLowerRight = toConvert.bottomRight();

  QPoint upperLeft = coordinateSystem->mapFromGlobal(globalUpperLeft);
  QPoint lowerRight = coordinateSystem->mapFromGlobal(globalLowerRight);

  return QRect(upperLeft.x(), upperLeft.y(), lowerRight.x() - upperLeft.x(),
            lowerRight.y() - upperLeft.y());
}

QPoint QtWidgetsTweakletImpl::ToControl(QWidget* coordinateSystem,
          const QPoint& toConvert)
{
  return coordinateSystem->mapFromGlobal(toConvert);
}

QRect QtWidgetsTweakletImpl::ToDisplay(QWidget* coordinateSystem,
        const QRect& toConvert)
{
  QPoint upperLeft = toConvert.topLeft();
  QPoint lowerRight = toConvert.bottomRight();

  QPoint globalUpperLeft = coordinateSystem->mapToGlobal(upperLeft);
  QPoint globalLowerRight = coordinateSystem->mapToGlobal(lowerRight);

  return QRect(globalUpperLeft.x(), globalUpperLeft.y(), globalLowerRight.x() - globalUpperLeft.x(),
            globalLowerRight.y() - globalUpperLeft.y());
}

QPoint QtWidgetsTweakletImpl::ToDisplay(QWidget* coordinateSystem,
        const QPoint& toConvert)
{
  return coordinateSystem->mapToGlobal(toConvert);
}

}
