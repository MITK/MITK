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


#ifndef QMITKMODULETABLEMODEL_H
#define QMITKMODULETABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include <QmitkExtExports.h>

namespace mitk {
class ModuleContext;
class Module;
}

class QmitkModuleTableModelPrivate;

class QmitkExt_EXPORT QmitkModuleTableModel : public QAbstractTableModel
{
public:

  QmitkModuleTableModel(QObject* parent = 0, mitk::ModuleContext* mc = 0);
  ~QmitkModuleTableModel();

protected:

  int rowCount(const QModelIndex& parent = QModelIndex()) const;

  int columnCount(const QModelIndex& parent = QModelIndex()) const;

  QVariant data(const QModelIndex& index, int role) const;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const;

private:

  friend class QmitkModuleTableModelPrivate;

  void insertModule(mitk::Module* module);

  QmitkModuleTableModelPrivate* const d;
};

#endif // QMITKMODULETABLEMODEL_H
