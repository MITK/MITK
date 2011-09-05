/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
