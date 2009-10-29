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

#include "cherryQtShell.h"

#include "cherryQtWidgetsTweakletImpl.h"
#include "cherryQtMainWindowControl.h"
#include "cherryQtControlWidget.h"

#include <cherryConstants.h>

#include <QApplication>
#include <QVariant>

namespace cherry
{

QtShell::QtShell(QWidget* parent, Qt::WindowFlags flags)
 : updatesDisabled(false)
{
  Shell::Pointer thisShell(this);
  if (parent == 0 || flags.testFlag(Qt::Window))
  {
    widget = new QtMainWindowControl(thisShell, parent, flags);
    widget->setUpdatesEnabled(false);
    updatesDisabled = true;

    widget->setAttribute(Qt::WA_DeleteOnClose);
  }
  else
  {
    widget = new QtControlWidget(parent, thisShell, flags | Qt::Dialog);
    widget->setObjectName("shell widget");
  }
}

QtShell::~QtShell()
{
  delete widget;
}

void QtShell::SetBounds(const Rectangle& bounds)
{
  widget->setGeometry(bounds.x, bounds.y, bounds.width, bounds.height);
}

Rectangle QtShell::GetBounds() const
{
  const QRect& qRect = widget->geometry();
  Rectangle rect(qRect.x(), qRect.y(), qRect.width(), qRect.height());
  return rect;
}

void QtShell::SetLocation(int x, int y)
{
  widget->move(x, y);
}

Point QtShell::ComputeSize(int  /*wHint*/, int  /*hHint*/, bool changed)
{
  if (changed) widget->updateGeometry();
  QSize size(widget->size());
  Point point(size.width(), size.height());
  return point;
}

void QtShell::SetText(const std::string& text)
{
  QString title(QString::fromStdString(text));
  widget->setWindowTitle(title);
  widget->setObjectName(title);
}

bool QtShell::IsVisible()
{
  return widget->isVisible();
}

void QtShell::SetVisible(bool visible)
{
  widget->setVisible(visible);
}

void* QtShell::GetControl()
{
  return widget;
}

void QtShell::SetImages(const std::vector<void*>&  /*images*/)
{
}

bool QtShell::GetMaximized()
{
  return widget->isMaximized();
}

bool QtShell::GetMinimized()
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

void QtShell::AddShellListener(IShellListener::Pointer listener)
{
  QVariant variant = widget->property(QtWidgetController::PROPERTY_ID);
  poco_assert(variant.isValid());
  QtWidgetController::Pointer controller = variant.value<QtWidgetController::Pointer>();
  poco_assert(controller != 0);
  controller->AddShellListener(listener);
}

void QtShell::RemoveShellListener(IShellListener::Pointer listener)
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

std::vector<Shell::Pointer> QtShell::GetShells()
{
  return std::vector<Shell::Pointer>(QtWidgetsTweakletImpl::shellList.begin(),
                                     QtWidgetsTweakletImpl::shellList.end());
}

int QtShell::GetStyle()
{
  Qt::WindowFlags qtFlags = widget->windowFlags();

  int cherryFlags = 0;
  if (!(qtFlags & Qt::FramelessWindowHint))
    cherryFlags |= Constants::BORDER;
  if (qtFlags & Qt::WindowTitleHint)
    cherryFlags |= Constants::TITLE;
  if (qtFlags & Qt::WindowSystemMenuHint)
    cherryFlags |= Constants::CLOSE;
  if (qtFlags & Qt::WindowMinimizeButtonHint)
    cherryFlags |= Constants::MIN;
  if (qtFlags & Qt::WindowMaximizeButtonHint)
    cherryFlags |= Constants::MAX;

  if (widget->windowModality() == Qt::WindowModal)
    cherryFlags |= Constants::PRIMARY_MODAL;
  else if(widget->windowModality() == Qt::ApplicationModal)
    cherryFlags |= Constants::APPLICATION_MODAL;

  return cherryFlags;
}

QWidget* QtShell::GetWidget()
{
  return widget;
}

}
