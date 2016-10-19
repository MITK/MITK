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
#ifndef QMITKUSNAVIGATIONCALIBRATIONREMOVEDELEGATE_H
#define QMITKUSNAVIGATIONCALIBRATIONREMOVEDELEGATE_H

#include <QStyledItemDelegate>

/**
 * \brief QStyledItemDelegate that provides a QColorDialog as editor.
 */
class QmitkUSNavigationCalibrationRemoveDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkUSNavigationCalibrationRemoveDelegate(QObject* parent = 0);
  ~QmitkUSNavigationCalibrationRemoveDelegate();

  /**
   * \brief A QColorDialog is opened on double click events.
   */
  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
};

#endif // QMITKUSNAVIGATIONCALIBRATIONREMOVEDELEGATE_H
