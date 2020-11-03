/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYPERSPECTIVELISTMODEL_H
#define BERRYPERSPECTIVELISTMODEL_H

#include <org_blueberry_ui_qt_Export.h>

#include <berrySmartPointer.h>

#include <QAbstractListModel>
#include <QScopedPointer>

namespace berry {

struct IPerspectiveDescriptor;
struct IPerspectiveRegistry;

class BERRY_UI_QT PerspectiveListModel : public QAbstractListModel
{
  Q_OBJECT

public:

  enum Role {
    Id = Qt::UserRole,
    Description
  };

  PerspectiveListModel(IPerspectiveRegistry& perspReg, bool markDefault = true, QObject* parent = nullptr);
  ~PerspectiveListModel() override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  QString perspectiveName(const QModelIndex& index) const;
  SmartPointer<IPerspectiveDescriptor> perspectiveDescriptor(const QModelIndex& index) const;

  using QAbstractListModel::index;
  QModelIndex index(const QString& perspId) const;

private:

  struct Impl;
  QScopedPointer<Impl> d;
};

}

#endif // BERRYPERSPECTIVELISTMODEL_H
