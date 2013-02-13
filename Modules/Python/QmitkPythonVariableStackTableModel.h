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

#ifndef QmitkPythonVariableStackTableModel_h
#define QmitkPythonVariableStackTableModel_h

#include <QAbstractTableModel>
#include <QVariant>
#include <QModelIndex>
#include "mitkIPythonService.h"
#include "mitkPythonExports.h"

///
/// implements a table model to show the variables of the Python "__main__" dictionary
/// furthermore implements dragging and dropping of datanodes (conversion from and to python)
///
class MITK_PYTHON_EXPORT QmitkPythonVariableStackTableModel : public QAbstractTableModel, public mitk::PythonCommandObserver
{
  Q_OBJECT

public:
  static const QString MITK_IMAGE_VAR_NAME;
  static const QString MITK_SURFACE_VAR_NAME;

  QmitkPythonVariableStackTableModel(QObject *parent = 0);
  virtual ~QmitkPythonVariableStackTableModel();

  int rowCount(const QModelIndex &parent = QModelIndex()) const;
  int columnCount(const QModelIndex &parent = QModelIndex()) const;
  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags( const QModelIndex& index ) const;
  virtual QVariant headerData(int section, Qt::Orientation orientation,
                              int role) const;

  QStringList mimeTypes() const;
  bool dropMimeData ( const QMimeData *, Qt::DropAction, int, int, const QModelIndex & );
  Qt::DropActions supportedDropActions() const;
  Qt::DropActions supportedDragActions() const;

  void CommandExecuted(const std::string& pythonCommand);

  std::vector<mitk::PythonVariable> GetVariableStack() const;
private:
  std::vector<mitk::PythonVariable> m_VariableStack;
  mitk::IPythonService* m_PythonService;
};

#endif // QmitkPythonVariableStackTableModel_h
