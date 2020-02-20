/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkDataManagerItemDelegate_h
#define QmitkDataManagerItemDelegate_h

#include <QStyledItemDelegate>

class QmitkDataManagerItemDelegate : public QStyledItemDelegate
{
  Q_OBJECT

public:
  explicit QmitkDataManagerItemDelegate(QObject* parent = nullptr);
  ~QmitkDataManagerItemDelegate() override;

  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
};

#endif
