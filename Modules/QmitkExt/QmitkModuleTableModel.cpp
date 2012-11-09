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


#include "QmitkModuleTableModel.h"

#include <mitkGetModuleContext.h>
#include <mitkModuleContext.h>
#include <mitkModule.h>

#include <QBrush>


class QmitkModuleTableModelPrivate
{

public:

  QmitkModuleTableModelPrivate(QmitkModuleTableModel* q, mitk::ModuleContext* mc)
    : q(q), context(mc)
  {
    std::vector<mitk::Module*> m;
    context->GetModules(m);
    for (std::vector<mitk::Module*>::const_iterator it = m.begin();
         it != m.end(); ++it)
    {
      modules[(*it)->GetModuleId()] = *it;
    }

    context->AddModuleListener(this, &QmitkModuleTableModelPrivate::ModuleChanged);
  }

  ~QmitkModuleTableModelPrivate()
  {
    context->RemoveModuleListener(this, &QmitkModuleTableModelPrivate::ModuleChanged);
  }

  void ModuleChanged(mitk::ModuleEvent event)
  {
    q->insertModule(event.GetModule());
  }

  QmitkModuleTableModel* q;
  mitk::ModuleContext* context;
  QMap<long, mitk::Module*> modules;
};

QmitkModuleTableModel::QmitkModuleTableModel(QObject* parent, mitk::ModuleContext* mc)
  : QAbstractTableModel(parent),
    d(new QmitkModuleTableModelPrivate(this, mc ? mc : mitk::GetModuleContext()))
{
}

QmitkModuleTableModel::~QmitkModuleTableModel()
{
  delete d;
}

int QmitkModuleTableModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return d->modules.size();
}

int QmitkModuleTableModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid()) return 0;
  return 5;
}

QVariant QmitkModuleTableModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid()) return QVariant();

  if (role == Qt::DisplayRole)
  {
    mitk::Module* module = d->modules[index.row()+1];
    switch(index.column())
    {
    case 0: return QVariant::fromValue(static_cast<int>(module->GetModuleId()));
    case 1: return QString::fromStdString(module->GetName());
    case 2: return QString::fromStdString(module->GetVersion().ToString());
    case 3:
    {
      QString deps = QString::fromStdString(module->GetProperty(mitk::Module::PROP_MODULE_DEPENDS()));
      QString libDeps  = QString::fromStdString(module->GetProperty(mitk::Module::PROP_LIB_DEPENDS()));
      if (!libDeps.isEmpty())
      {
        if (!deps.isEmpty()) deps.append(", ");
        deps.append(libDeps);
      }
      return deps;
    }
    case 4: return QString::fromStdString(module->GetLocation());
    }
  }
  else if (role == Qt::ForegroundRole)
  {
    mitk::Module* module = d->modules[index.row()+1];
    if (!module->IsLoaded())
    {
      return QBrush(Qt::gray);
    }
  }
  else if (role == Qt::ToolTipRole)
  {
    mitk::Module* module = d->modules[index.row()+1];
    QString id = QString::number(module->GetModuleId());
    QString name = QString::fromStdString(module->GetName());
    QString version = QString::fromStdString(module->GetVersion().ToString());
    QString moduleDepends = QString::fromStdString(module->GetProperty(mitk::Module::PROP_MODULE_DEPENDS()));
    QString libDepends = QString::fromStdString(module->GetProperty(mitk::Module::PROP_LIB_DEPENDS()));
    QString location = QString::fromStdString(module->GetLocation());
    QString state = module->IsLoaded() ? "Loaded" : "Unloaded";

    QString tooltip = "Id: %1\nName: %2\nVersion: %3\nModule Dependencies: %4\nLibrary Dependencies: %6\nLocation: %7\nState: %9";

    return tooltip.arg(id, name, version, moduleDepends, libDepends, location, state);
  }
  return QVariant();
}

QVariant QmitkModuleTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return QVariant();

  switch (section)
  {
  case 0: return "Id";
  case 1: return "Name";
  case 2: return "Version";
  case 3: return "Depends";
  case 4: return "Location";
  }
  return QVariant();
}

void QmitkModuleTableModel::insertModule(mitk::Module* module)
{
  long id = module->GetModuleId();
  if (d->modules.contains(id))
  {
    d->modules[id] = module;
    emit dataChanged(createIndex(id-1, 0), createIndex(id-1, columnCount()));
    return;
  }
  else
  {
    beginInsertRows(QModelIndex(), id-1, id-1);
    d->modules[id] = module;
    endInsertRows();
  }
}

