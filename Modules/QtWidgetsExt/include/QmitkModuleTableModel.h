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

#include <MitkQtWidgetsExtExports.h>

namespace us {
class ModuleContext;
class Module;
}

class QmitkModuleTableModelPrivate;

class MITKQTWIDGETSEXT_EXPORT QmitkModuleTableModel : public QAbstractTableModel
{
public:

  QmitkModuleTableModel(QObject* parent = 0, us::ModuleContext* mc = 0);
  ~QmitkModuleTableModel();

protected:

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;

  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  QVariant data(const QModelIndex& index, int role) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:

  friend class QmitkModuleTableModelPrivate;

  void insertModule(us::Module* module);

  QmitkModuleTableModelPrivate* const d;
};

#endif

