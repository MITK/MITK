/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPythonVariableStackTableModel_h
#define QmitkPythonVariableStackTableModel_h

#include <QAbstractTableModel>
#include <QVariant>
#include <QModelIndex>
#include "mitkIPythonService.h"
#include <MitkQtPythonExports.h>
#include <usServiceReference.h>

///
/// implements a table model to show the variables of the Python "__main__" dictionary
/// furthermore implements dragging and dropping of datanodes (conversion from and to python)
///
class MITKQTPYTHON_EXPORT QmitkPythonVariableStackTableModel : public QAbstractTableModel, public mitk::PythonCommandObserver
{
  Q_OBJECT

public:
  static const QString MITK_IMAGE_VAR_NAME;
  static const QString MITK_SURFACE_VAR_NAME;

  QmitkPythonVariableStackTableModel(QObject *parent = nullptr);
  ~QmitkPythonVariableStackTableModel() override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  Qt::ItemFlags flags( const QModelIndex& index ) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                              int role) const override;

  QStringList mimeTypes() const override;
  bool dropMimeData ( const QMimeData *, Qt::DropAction, int, int, const QModelIndex & ) override;
  Qt::DropActions supportedDropActions() const override;
  //Qt::DropActions supportedDragActions() const;

  void CommandExecuted(const std::string& pythonCommand) override;

  std::vector<mitk::PythonVariable> GetVariableStack() const;
private:
  std::vector<mitk::PythonVariable> m_VariableStack;
  mitk::IPythonService* m_PythonService;
  us::ServiceReference<mitk::IPythonService> m_PythonServiceRef;
};

#endif
