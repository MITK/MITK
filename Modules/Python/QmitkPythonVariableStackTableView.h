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

#ifndef QmitkPythonVariableStackTableView_h
#define QmitkPythonVariableStackTableView_h

#include <QTableView>
#include <mitkDataStorage.h>
#include "QmitkPythonVariableStackTableModel.h"
#include <mitkIPythonService.h>
#include "mitkPythonExports.h"

///
/// implements the table view for the variable stack
/// purpose of this class: 1. Setup the view correctly, 2. Implement the double click to write back results
/// to the datastorage
///
class MITK_PYTHON_EXPORT QmitkPythonVariableStackTableView : public QTableView
{
  Q_OBJECT

public:
  QmitkPythonVariableStackTableView(QWidget *parent = 0);
  virtual ~QmitkPythonVariableStackTableView();

  void SetDataStorage(mitk::DataStorage* _DataStorage);

protected slots:
  void OnVariableStackDoubleClicked(const QModelIndex &index);

private:
  QmitkPythonVariableStackTableModel* m_TableModel;
  mitk::DataStorage::Pointer m_DataStorage;
  mitk::IPythonService* m_PythonService;
};

#endif // QmitkPythonVariableStackTableView_h
