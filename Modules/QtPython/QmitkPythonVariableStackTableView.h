/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPythonVariableStackTableView_h
#define QmitkPythonVariableStackTableView_h

#include <QTableView>
#include <mitkDataStorage.h>
#include "QmitkPythonVariableStackTableModel.h"
#include <mitkIPythonService.h>
#include <MitkQtPythonExports.h>

///
/// implements the table view for the variable stack
/// purpose of this class: 1. Setup the view correctly, 2. Implement the double click to write back results
/// to the datastorage
///
class MITKQTPYTHON_EXPORT QmitkPythonVariableStackTableView : public QTableView
{
  Q_OBJECT

public:
  QmitkPythonVariableStackTableView(QWidget *parent = nullptr);
  ~QmitkPythonVariableStackTableView() override;

  void SetDataStorage(mitk::DataStorage* _DataStorage);

protected slots:
  void OnVariableStackDoubleClicked(const QModelIndex &index);

private:
  QmitkPythonVariableStackTableModel* m_TableModel;
  mitk::DataStorage::Pointer m_DataStorage;
  mitk::IPythonService* m_PythonService;
};

#endif // QmitkPythonVariableStackTableView_h
