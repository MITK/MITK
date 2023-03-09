/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkTableItemThumbnailDelegate_h
#define QmitkTableItemThumbnailDelegate_h

#include <QStyledItemDelegate>

/*
* @brief The QmitkTableItemThumbnailDelegate is a subclass of the QmitkAbstractSemanticRelationsStorageModel
*/
class QmitkTableItemThumbnailDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:

  QmitkTableItemThumbnailDelegate(QObject* parent = nullptr);

  void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

#endif
