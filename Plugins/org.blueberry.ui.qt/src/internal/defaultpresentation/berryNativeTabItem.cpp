/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "berryNativeTabItem.h"

#include "berryNativeTabFolder.h"
#include "berryQCTabBar.h"

#include <berryConstants.h>

#include <QToolButton>
#include <QFile>
#include <QRegularExpression>
#include <QApplication>

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

QString ParseColor(const QString &subject, const QString &pattern, const QString &fallback)
{
  QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
  auto match = re.match(subject);

  return match.hasMatch()
    ? match.captured(1)
    : fallback;
}

QWidget* NativeTabItem::GetCloseButton()
{
  if (!closeButton)
  {
    closeButton = new QToolButton(parent->GetControl());
    QFile resourceFile(":/org.blueberry.ui.qt/tab_close.svg");
    if (resourceFile.open(QIODevice::ReadOnly))
    {
      auto originalSVG = resourceFile.readAll();
      auto styleSheet = qApp->styleSheet();

      if (styleSheet.isEmpty()) {
        QIcon iconCloseTab(QPixmap::fromImage(QImage::fromData(originalSVG)));
        closeButton->setIcon(iconCloseTab);
      } else {
        auto iconColor = ParseColor(styleSheet,
          QStringLiteral("iconColor\\s*[=:]\\s*(#[0-9a-f]{6})"),
          QStringLiteral("#000000"));

        auto iconAccentColor = ParseColor(styleSheet,
          QStringLiteral("iconAccentColor\\s*[=:]\\s*(#[0-9a-f]{6})"),
          QStringLiteral("#ffffff"));

        auto themedSVG = QString(originalSVG).replace(QStringLiteral("#00ff00"), iconColor, Qt::CaseInsensitive);
        themedSVG = themedSVG.replace(QStringLiteral("#ff00ff"), iconAccentColor, Qt::CaseInsensitive);

        QIcon iconCloseTab(QPixmap::fromImage(QImage::fromData(themedSVG.toLatin1())));
        closeButton->setIcon(iconCloseTab);
      }
    }
    closeButton->setObjectName("TabCloseButton");
    closeButton->setContentsMargins(0, 0, 0, 0);
    closeButton->setFixedSize(12,12);
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

void NativeTabItem::SetLock(bool lock)
{
  // disabled butoon looks same so hide it
  // TODO: add icon for QIcon::Disabled and uncomment
  //this->closeButton->setEnabled(!busy);
  this->closeButton->setVisible(!lock);
}

}
