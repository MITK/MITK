/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#include "berryPerspectiveListModel.h"

#include <berryIPerspectiveRegistry.h>
#include <berryIPerspectiveDescriptor.h>

#include <QIcon>

namespace berry {

struct PerspectiveListModel::Impl
{
  IPerspectiveRegistry& m_PerspReg;
  QList<IPerspectiveDescriptor::Pointer> m_Perspectives;
  const bool m_MarkDefault;

  Impl(IPerspectiveRegistry& perspReg, bool markDefault)
    : m_PerspReg(perspReg)
  // TODO  use activity filter for correct perspective list
    , m_Perspectives(perspReg.GetPerspectives())
    , m_MarkDefault(markDefault)
  {
  }
};

PerspectiveListModel::PerspectiveListModel(IPerspectiveRegistry& perspReg, bool markDefault, QObject* parent)
  : QAbstractListModel(parent)
  , d(new Impl(perspReg, markDefault))
{
}

PerspectiveListModel::~PerspectiveListModel()
{
}

int PerspectiveListModel::rowCount(const QModelIndex& /*parent*/) const
{
  return d->m_Perspectives.size();
}

QVariant PerspectiveListModel::data(const QModelIndex& index, int role) const
{
  if (index.row() < 0 || index.row() >= d->m_Perspectives.size() ||
      index.column() > 0)
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    const IPerspectiveDescriptor::Pointer& desc = d->m_Perspectives.at(index.row());
    QString label = desc->GetLabel();
    if (d->m_MarkDefault)
    {
      QString def = d->m_PerspReg.GetDefaultPerspective();
      if (desc->GetId() == def)
      {
        label += " (default)";
      }
    }
    return label;
  }
  else if (role == Qt::DecorationRole)
  {
    const IPerspectiveDescriptor::Pointer& desc = d->m_Perspectives.at(index.row());
    return desc->GetImageDescriptor();
  }
  else if (role == Id)
  {
    const IPerspectiveDescriptor::Pointer& desc = d->m_Perspectives.at(index.row());
    return desc->GetId();
  }
  else if (role == Description)
  {
    const IPerspectiveDescriptor::Pointer& desc = d->m_Perspectives.at(index.row());
    return desc->GetDescription();
  }
  return QVariant();
}

QVariant PerspectiveListModel::headerData(int section, Qt::Orientation /*orientation*/, int role) const
{
  if (role == Qt::DisplayRole)
  {
    if (section == 0)
    {
      return QString("Perspective");
    }
  }
  return QVariant();
}

QString PerspectiveListModel::perspectiveName(const QModelIndex& index) const
{
  if (!index.isValid()) return QString::null;

  return d->m_Perspectives.at(index.row())->GetLabel();
}

IPerspectiveDescriptor::Pointer PerspectiveListModel::perspectiveDescriptor(const QModelIndex& index) const
{
  return d->m_Perspectives.at(index.row());
}

QModelIndex PerspectiveListModel::index(const QString& perspId) const
{
  int index = -1;
  for(int i = 0; i < d->m_Perspectives.size(); ++i)
  {
    if (d->m_Perspectives.at(i)->GetId() == perspId)
    {
      index = i;
      break;
    }
  }

  if (index > -1)
  {
    return this->createIndex(index, 0);
  }
  return QModelIndex();
}



}
