/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryNativeTabItem.h"

#include "berryNativeTabFolder.h"
#include "berryQCTabBar.h"
#include "berryQtStyleManager.h"

#include <berryConstants.h>

#include <QToolButton>

namespace berry
{

NativeTabItem::NativeTabItem(NativeTabFolder* _parent, int index, int flags) :
  parent(_parent), style(flags), showClose(true), closeButton(nullptr)
{
  parent->GetTabFolder()->insertTab(index, this);

  if (this->GetShowClose())
  {
    parent->GetTabFolder()->setTabButton(index, QTabBar::RightSide, this->GetCloseButton());
    this->connect(this->GetCloseButton(), SIGNAL(clicked()), this, SLOT(CloseButtonClicked()));
  }
}

void NativeTabItem::CloseButtonClicked()
{
  parent->CloseButtonClicked(this);
}

QRect NativeTabItem::GetBounds()
{
  int index = parent->IndexOf(this);

  QTabBar* folder = parent->GetTabFolder();
  QRect localRect = folder->tabRect(index);

  QPoint topLeft = localRect.topLeft();
  QPoint bottomRight = localRect.bottomRight();

  QPoint globalTopLeft = folder->mapToGlobal(topLeft);
  QPoint globalBottomRight = folder->mapToGlobal(bottomRight);

  return QRect(globalTopLeft, globalBottomRight);
}

void NativeTabItem::SetInfo(const PartInfo& info)
{
  QTabBar* widget = parent->GetTabFolder();

  int index = parent->IndexOf(this);
  if (widget->tabText(index) != info.name)
  {
    widget->setTabText(index, info.name);
  }

  if (widget->tabToolTip(index) != info.toolTip)
  {
    widget->setTabToolTip(index, info.toolTip);
  }

  if (widget->tabIcon(index).cacheKey() != info.image.cacheKey())
  {
    widget->setTabIcon(index, info.image);
  }
}

bool NativeTabItem::GetShowClose() const
{
  return ((style & Constants::CLOSE) && showClose);
}

void NativeTabItem::SetShowClose(bool close)
{
  showClose = close;
}

QWidget* NativeTabItem::GetCloseButton()
{
  if (!closeButton)
  {
    QIcon iconCloseTab = QtStyleManager::ThemeIcon(QStringLiteral(":/org.blueberry.ui.qt/tab_close.svg"));
    closeButton = new QToolButton(parent->GetControl());
    closeButton->setObjectName("TabCloseButton");
    closeButton->setContentsMargins(0, 0, 0, 0);
    closeButton->setFixedSize(12,12);
    closeButton->setIcon(iconCloseTab);
    closeButton->setAutoRaise(true);
  }

  return closeButton;
}

void NativeTabItem::Dispose()
{
  QTabBar* widget = parent->GetTabFolder();
  int index = parent->IndexOf(this);
  widget->removeTab(index); // this calls QCTabBar::tabRemoved
}

Object::Pointer NativeTabItem::GetData()
{
  return data;
}

void NativeTabItem::SetData(Object::Pointer d)
{
  this->data = d;
}

}
