/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef QmitkUSZoneManagementColorDialogDelegate_h
#define QmitkUSZoneManagementColorDialogDelegate_h

#include <QStyledItemDelegate>

/**
 * \brief QStyledItemDelegate that provides a QColorDialog as editor.
 */
class QmitkUSZoneManagementColorDialogDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkUSZoneManagementColorDialogDelegate(QObject* parent = nullptr);
  ~QmitkUSZoneManagementColorDialogDelegate() override;

  /**
   * \brief A QColorDialog is opened on double click events.
   */
  bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

#endif
