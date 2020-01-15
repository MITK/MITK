/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <tweaklets/berryGuiWidgetsTweaklet.h>

#include "berryQtShell.h"

#include "berryQtWidgetsTweakletImpl.h"
#include "berryQtMainWindowControl.h"
#include <internal/berryQtControlWidget.h>

#include <berryConstants.h>
#include <internal/berryTweaklets.h>

#include <QApplication>
#include <QVariant>

namespace berry
{

QtShell::QtShell(QWidget* parent, Qt::WindowFlags flags)
 : updatesDisabled(false)
{
  if (parent == nullptr || flags.testFlag(Qt::Window))
  {
    widget = new QtMainWindowControl(this, parent, flags);
    widget->setUpdatesEnabled(false);
    updatesDisabled = true;

    widget->setAttribute(Qt::WA_DeleteOnClose);

  }
  else
  {
    widget = new QtControlWidget(parent, this, flags | Qt::Dialog);
    widget->setObjectName("shell widget");
  }

  widget->setProperty("shell", QVariant::fromValue(static_cast<Shell*>(this)));
}

QtShell::~QtShell()
{
  widget->setProperty("shell", QVariant::fromValue(static_cast<Shell*>(nullptr)));
  widget->deleteLater();
}

void QtShell::SetBounds(const QRect& bounds)
{
  widget->setGeometry(bounds);
}

QRect QtShell::GetBounds() const
{
  return widget->frameGeometry();
}

void QtShell::SetLocation(int x, int y)
{
  widget->move(x, y);
}

QPoint QtShell::ComputeSize(int  /*wHint*/, int  /*hHint*/, bool changed)
{
  if (changed) widget->updateGeometry();
  QSize size(widget->size());
  QPoint point(size.width(), size.height());
  return point;
}

QString QtShell::GetText() const
{
  return widget->windowTitle();
}

void QtShell::SetText(const QString& title)
{
  widget->setWindowTitle(title);
  widget->setObjectName(title);
}

bool QtShell::IsVisible() const
{
  return widget->isVisible();
}

void QtShell::SetVisible(bool visible)
{
  widget->setVisible(visible);
}

void QtShell::SetActive()
{
  widget->activateWindow();
  widget->raise();
}

QWidget *QtShell::GetControl() const
{
  return widget;
}

void QtShell::SetImages(const QList<QIcon>&  /*images*/)
{
}

bool QtShell::GetMaximized() const
{
  return widget->isMaximized();
}

bool QtShell::GetMinimized() const
{
  return widget->isMinimized();
}

void QtShell::SetMaximized(bool maximized)
{
  maximized ? widget->showMaximized() : widget->showNormal();
}

void QtShell::SetMinimized(bool minimized)
{
  minimized ? widget->showMinimized() : widget->showNormal();
}

void QtShell::AddShellListener(IShellListener* listener)
{
  QVariant variant = widget->property(QtWidgetController::PROPERTY_ID);
  poco_assert(variant.isValid());
  QtWidgetController::Pointer controller = variant.value<QtWidgetController::Pointer>();
  poco_assert(controller != 0);
  controller->AddShellListener(listener);
}

void QtShell::RemoveShellListener(IShellListener* listener)
{
  QVariant variant = widget->property(QtWidgetController::PROPERTY_ID);
  if (variant.isValid())
  {
    QtWidgetController::Pointer controller = variant.value<QtWidgetController::Pointer>();
    if (controller != 0)
      controller->RemoveShellListener(listener);
  }
}

void QtShell::Open(bool block)
{
  if (updatesDisabled)
  {
    widget->setUpdatesEnabled(true);
    updatesDisabled = false;
  }
  widget->setWindowModality(block ? Qt::WindowModal : Qt::NonModal);
  widget->show();
}

void QtShell::Close()
{
  widget->close();
}

QList<Shell::Pointer> QtShell::GetShells()
{
  GuiWidgetsTweaklet* widgetTweaklet = Tweaklets::Get(GuiWidgetsTweaklet::KEY);
  QList<Shell::Pointer> allShells(widgetTweaklet->GetShells());
  QList<Shell::Pointer> descendants;

  for (int i = 0; i < allShells.size(); ++i)
  {
    Shell::Pointer shell = allShells[i];
    if (widgetTweaklet->GetShell(shell->GetControl()) == this)
    {
      descendants.push_back(shell);
    }
  }

  return descendants;
}

Qt::WindowFlags QtShell::GetStyle() const
{
  return widget->windowFlags();
}

QWidget* QtShell::GetWidget()
{
  return widget;
}

}
