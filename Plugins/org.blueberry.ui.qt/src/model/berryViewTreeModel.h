/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYVIEWTREEMODEL_H
#define BERRYVIEWTREEMODEL_H

#include <org_blueberry_ui_qt_Export.h>

#include <berrySmartPointer.h>

#include <QAbstractItemModel>

namespace berry {

struct IViewRegistry;
struct IWorkbenchWindow;

class BERRY_UI_QT ViewTreeModel : public QAbstractItemModel
{
  Q_OBJECT

public:

  ViewTreeModel(const IWorkbenchWindow* window,
                QObject* parent = nullptr);

  ~ViewTreeModel() override;

  enum Role {
    Description = Qt::UserRole,
    Keywords,
    Id
  };

  QVariant data(const QModelIndex &index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &child) const override;
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  const IWorkbenchWindow* GetWorkbenchWindow() const;

private:

  struct Impl;
  QScopedPointer<Impl> d;
};

}

#endif // BERRYVIEWTREEMODEL_H
