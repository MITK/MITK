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

#ifndef QMITKSEMANTICRELATIONSTABLEVIEW_H
#define QMITKSEMANTICRELATIONSTABLEVIEW_H

// semantic relations UI module
#include "MitkSemanticRelationsUIExports.h"
#include <ui_QmitkSemanticRelationsTableViewControls.h>
#include "QmitkSemanticRelationsDataModel.h"

// mitk core
#include <mitkDataNode.h>
#include <mitkDataStorage.h>

// qt
#include <QMenu>
#include <QWidget>

class MITKSEMANTICRELATIONSUI_EXPORT QmitkSemanticRelationsTableView : public QWidget
{
  Q_OBJECT

public:

  QmitkSemanticRelationsTableView(mitk::DataStorage::Pointer dataStorage, QWidget* parent = nullptr);
  ~QmitkSemanticRelationsTableView();

  void AddImageInstance(const mitk::DataNode* dataNode);
  void RemoveImageInstance(const mitk::DataNode* dataNode);
  void AddSegmentationInstance(const mitk::DataNode* segmentationNode, const mitk::DataNode* parentNode);
  void RemoveSegmentationInstance(const mitk::DataNode* segmentationNode, const mitk::DataNode* parentNode);

Q_SIGNALS:
  void SelectionChanged(const mitk::DataNode*);
  void DataChanged(const mitk::SemanticTypes::CaseID&);

private Q_SLOTS:

  void OnCaseIDSelectionChanged(const QString& caseID);

  void OnTableViewDataChanged();
  void OnTableViewItemClicked(const QModelIndex& selectedIndex);
  void OnTableViewContextMenuRequested(const QPoint& pos);
  void OnContextMenuSetInformationType();
  void OnContextMenuSetControlPoint();
  void OnTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

private:

  void Init();
  void SetUpConnections();

  QPixmap GetPixmapFromImageNode(const mitk::DataNode* dataNode) const;

  Ui::QmitkSemanticRelationsTableViewControls m_Controls;

  mitk::DataStorage::Pointer m_DataStorage;
  std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;
  std::unique_ptr<QmitkSemanticRelationsDataModel> m_SemanticRelationsDataModel;

  QMenu* m_ContextMenu;
  mitk::DataNode* m_SelectedDataNode;

};

#endif // QMITKSEMANTICRELATIONSTABLEVIEW_H
