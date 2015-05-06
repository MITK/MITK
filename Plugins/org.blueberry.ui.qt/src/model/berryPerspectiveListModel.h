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
  ~PerspectiveListModel();

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

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
