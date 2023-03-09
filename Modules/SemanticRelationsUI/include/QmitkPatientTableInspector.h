/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkPatientTableInspector_h
#define QmitkPatientTableInspector_h

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include "QmitkAbstractSemanticRelationsStorageInspector.h"
#include "QmitkPatientTableModel.h"
#include "QmitkTableItemThumbnailDelegate.h"

#include "ui_QmitkPatientTableInspector.h"

// qt widgets module
#include <QmitkEnums.h>

// qt
#include <QMenu>

/*
* @brief The QmitkPatientTableInspector is a QmitkAbstractSemanticRelationsStorageInspector that shows the currently
*        available data of the semantic relations storage model in a control-point - information type matrix.
*
*   The QmitkPatientTableInspector uses the QmitkSemanticRelationsStorageModel, a QmitkAbstractDataStorageModel that
*   presents the semantic relations data as a table, showing a QPixmap as thumbnail for the data nodes.
*/
class MITKSEMANTICRELATIONSUI_EXPORT QmitkPatientTableInspector : public QmitkAbstractSemanticRelationsStorageInspector
{
  Q_OBJECT

public:

  QmitkPatientTableInspector(QWidget* parent = nullptr);

  QAbstractItemView* GetView() override;
  const QAbstractItemView* GetView() const override;

  void SetSelectionMode(SelectionMode mode) override;
  SelectionMode GetSelectionMode() const override;

  void SetCaseID(const mitk::SemanticTypes::CaseID& caseID) override;
  void SetLesion(const mitk::SemanticTypes::Lesion& lesion) override;

  QItemSelectionModel* GetSelectionModel();

Q_SIGNALS:

  void DataNodeDoubleClicked(const mitk::DataNode*);
  void OnContextMenuRequested(const QPoint&);
  void OnNodeRemoved(const mitk::DataNode*);

private Q_SLOTS:

  void OnModelUpdated();
  void OnNodeButtonClicked(const QString&);
  void OnDataNodeSelectionChanged(const QList<mitk::DataNode::Pointer>&);
  void OnItemDoubleClicked(const QModelIndex&);

protected:

  void Initialize() override;

private:

  void SetUpConnections();

  void keyPressEvent(QKeyEvent* e) override;

  Ui::QmitkPatientTableInspector m_Controls;
  QmitkPatientTableModel* m_StorageModel;
  QmitkTableItemThumbnailDelegate* m_ItemDelegate;

};

#endif
