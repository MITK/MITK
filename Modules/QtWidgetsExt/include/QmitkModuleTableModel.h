/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKMODULETABLEMODEL_H
#define QMITKMODULETABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>

#include <MitkQtWidgetsExtExports.h>

namespace us
{
  class ModuleContext;
  class Module;
}

class QmitkModuleTableModelPrivate;

class MITKQTWIDGETSEXT_EXPORT QmitkModuleTableModel : public QAbstractTableModel
{
public:
  QmitkModuleTableModel(QObject *parent = nullptr, us::ModuleContext *mc = nullptr);
  ~QmitkModuleTableModel() override;

protected:
  int rowCount(const QModelIndex &parent = QModelIndex()) const override;

  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role) const override;

  QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
  friend class QmitkModuleTableModelPrivate;

  void insertModule(us::Module *module);

  QmitkModuleTableModelPrivate *const d;
};

#endif
