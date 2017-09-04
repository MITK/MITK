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

#ifndef QMITKPATIENTTABLEWIDGET_H
#define QMITKPATIENTTABLEWIDGET_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include <ui_QmitkPatientTableWidgetControls.h>
#include "QmitkPatientTableModel.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

// qt
#include <QMenu>
#include <QWidget>

class MITKSEMANTICRELATIONSUI_EXPORT QmitkPatientTableWidget : public QWidget
{
  Q_OBJECT

public:

  QmitkPatientTableWidget(std::shared_ptr<mitk::SemanticRelations> semanticRelations, QWidget* parent = nullptr);
  ~QmitkPatientTableWidget();

  void SetCurrentCaseID(const mitk::SemanticTypes::CaseID& caseID);
  void UpdatePatientTable();
  void SetPixmapOfNode(const mitk::DataNode* dataNode);
  void DeletePixmapOfNode(const mitk::DataNode* dataNode);

Q_SIGNALS:
  void SelectionChanged(const mitk::DataNode*);

private Q_SLOTS:

  void OnPatientTableModelUpdated();
  void OnPatientTableViewItemClicked(const QModelIndex&);
  void OnPatientTableViewContextMenuRequested(const QPoint&);
  void OnContextMenuSetInformationType();
  void OnContextMenuSetControlPoint();
  void OnPatientTableViewSelectionChanged(const QItemSelection&, const QItemSelection&);

private:

  void Init();
  void SetUpConnections();

  QPixmap GetPixmapFromImageNode(const mitk::DataNode* dataNode) const;

  Ui::QmitkPatientTableWidgetControls m_Controls;

  mitk::DataStorage::Pointer m_DataStorage;
  std::shared_ptr<mitk::SemanticRelations> m_SemanticRelations;
  std::unique_ptr<QmitkPatientTableModel> m_PatientTableModel;

  QMenu* m_ContextMenu;
  mitk::DataNode* m_SelectedDataNode;

};

#endif // QMITKPATIENTTABLEWIDGET_H
