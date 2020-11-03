/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkModuleTableModel.h"

#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>

#include <QBrush>

class QmitkModuleTableModelPrivate
{
public:
  QmitkModuleTableModelPrivate(QmitkModuleTableModel *q, us::ModuleContext *mc) : q(q), context(mc)
  {
    std::vector<us::Module *> m = context->GetModules();
    for (std::vector<us::Module *>::const_iterator it = m.begin(); it != m.end(); ++it)
    {
      modules[(*it)->GetModuleId()] = *it;
    }

    context->AddModuleListener(this, &QmitkModuleTableModelPrivate::ModuleChanged);
  }

  ~QmitkModuleTableModelPrivate() { context->RemoveModuleListener(this, &QmitkModuleTableModelPrivate::ModuleChanged); }
  void ModuleChanged(us::ModuleEvent event) { q->insertModule(event.GetModule()); }
  QmitkModuleTableModel *q;
  us::ModuleContext *context;
  QMap<long, us::Module *> modules;
};

QmitkModuleTableModel::QmitkModuleTableModel(QObject *parent, us::ModuleContext *mc)
  : QAbstractTableModel(parent), d(new QmitkModuleTableModelPrivate(this, mc ? mc : us::GetModuleContext()))
{
}

QmitkModuleTableModel::~QmitkModuleTableModel()
{
  delete d;
}

int QmitkModuleTableModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;
  return d->modules.size();
}

int QmitkModuleTableModel::columnCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return 0;
  return 4;
}

QVariant QmitkModuleTableModel::data(const QModelIndex &index, int role) const
{
  if (!index.isValid())
    return QVariant();

  if (role == Qt::DisplayRole)
  {
    us::Module *module = d->modules[index.row() + 1];
    switch (index.column())
    {
      case 0:
        return QVariant::fromValue(static_cast<int>(module->GetModuleId()));
      case 1:
        return QString::fromStdString(module->GetName());
      case 2:
        return QString::fromStdString(module->GetVersion().ToString());
      case 3:
        return QString::fromStdString(module->GetLocation());
    }
  }
  else if (role == Qt::ForegroundRole)
  {
    us::Module *module = d->modules[index.row() + 1];
    if (!module->IsLoaded())
    {
      return QBrush(Qt::gray);
    }
  }
  else if (role == Qt::ToolTipRole)
  {
    us::Module *module = d->modules[index.row() + 1];
    QString id = QString::number(module->GetModuleId());
    QString name = QString::fromStdString(module->GetName());
    QString version = QString::fromStdString(module->GetVersion().ToString());
    QString location = QString::fromStdString(module->GetLocation());
    QString state = module->IsLoaded() ? "Loaded" : "Unloaded";

    QString tooltip = "Id: %1\nName: %2\nVersion: %3\nLocation: %7\nState: %9";

    return tooltip.arg(id, name, version, location, state);
  }
  return QVariant();
}

QVariant QmitkModuleTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return QVariant();

  switch (section)
  {
    case 0:
      return "Id";
    case 1:
      return "Name";
    case 2:
      return "Version";
    case 3:
      return "Location";
  }
  return QVariant();
}

void QmitkModuleTableModel::insertModule(us::Module *module)
{
  long id = module->GetModuleId();
  if (d->modules.contains(id))
  {
    d->modules[id] = module;
    emit dataChanged(createIndex(id - 1, 0), createIndex(id - 1, columnCount()));
    return;
  }
  else
  {
    beginInsertRows(QModelIndex(), id - 1, id - 1);
    d->modules[id] = module;
    endInsertRows();
  }
}
