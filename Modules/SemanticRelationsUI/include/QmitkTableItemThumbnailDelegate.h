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

#ifndef QMITKTABLEITEMTHUMBNAILDELEGATE_H
#define QMITKTABLEITEMTHUMBNAILDELEGATE_H

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

#endif // QMITKTABLEITEMTHUMBNAILDELEGATE_H
