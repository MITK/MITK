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

#include "cherryNativeTabItem.h"

#include "cherryNativeTabFolder.h"
#include "cherryQCTabBar.h"

#include <cherryConstants.h>

#include <QToolButton>

namespace cherry
{

NativeTabItem::NativeTabItem(NativeTabFolder* _parent, int index, int flags) :
  parent(_parent), style(flags), showClose(true), closeButton(0)
{
  parent->GetTabFolder()->insertTab(index, this);
#if QT_VERSION >= 0x040500
  if (this->GetShowClose())
  {
    parent->GetTabFolder()->setTabButton(index, QTabBar::RightSide, this->GetCloseButton());
    this->connect(this->GetCloseButton(), SIGNAL(clicked()), this, SLOT(CloseButtonClicked()));
  }
#endif
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

  if (info.image == 0)
  {
    widget->setTabIcon(index, QIcon());
  }
  else
  {
    QIcon icon(*(info.image));
    if (widget->tabIcon(index).cacheKey() != icon.cacheKey())
    {
      widget->setTabIcon(index, icon);
    }
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
    QIcon iconCloseTab( ":/org.opencherry.ui.qt/tab_close_icon.png" );
    iconCloseTab.addFile(":/org.opencherry.ui.qt/tab_close_icon-active.png", QSize(), QIcon::Active);
    closeButton = new QToolButton(parent->GetControl());
    closeButton->setObjectName("TabCloseButton");
    closeButton->setContentsMargins(0, 0, 0, 0);
    closeButton->setFixedSize(12,12);
    //closeButton->setFlat(true);
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
