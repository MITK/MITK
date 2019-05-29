/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

// semantic relations UI module
#include "QmitkTableItemThumbnailDelegate.h"

// qt
#include <QApplication>
#include <QPainter>
#include <QStyle>

QmitkTableItemThumbnailDelegate::QmitkTableItemThumbnailDelegate(QObject* parent /*= nullptr*/)
  : QStyledItemDelegate(parent)
{
  // nothing here
}

void QmitkTableItemThumbnailDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();
  QPoint point = option.rect.center() - pixmap.rect().center();
  painter->save();

  if (option.state & QStyle::State_Selected)
  {
    painter->setBrush(option.palette.highlightedText());
    const QWidget* widget = option.widget;
    QStyle* style = widget ? widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &option, painter, widget);
  }
  painter->drawPixmap(point, pixmap);
  painter->restore();
}
