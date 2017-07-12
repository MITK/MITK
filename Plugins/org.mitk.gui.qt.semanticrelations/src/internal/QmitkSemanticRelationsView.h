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

#ifndef QMITKSEMANTICRELATIONSVIEW_H
#define QMITKSEMANTICRELATIONSVIEW_H

// semantic relations plugin
#include "ui_QmitkSemanticRelationsControls.h"
#include "QmitkSemanticRelationsDataModel.h"

// semantic relations module
#include <mitkSemanticRelations.h>

// blueberry
#include <berryISelectionListener.h>

// qt
#include <QmitkAbstractView.h>
#include <QMenu>

/**
* @brief SemanticRelation
*/
class QmitkSemanticRelationsView : public QmitkAbstractView
{
  Q_OBJECT

public:

  static const std::string VIEW_ID;

protected:

  virtual void SetFocus() override;

  virtual void CreateQtPartControl(QWidget* parent) override;

  virtual void NodeAdded(const mitk::DataNode* node) override;

  virtual void NodeRemoved(const mitk::DataNode* node) override;

private Q_SLOTS:

  void OnCaseIDSelectionChanged(const QString& caseID);
  void OnTableViewDataChanged();
  void OnTableViewContextMenuRequested(const QPoint& pos);
  void OnContextMenuSetInformationType();
  void OnContextMenuSetControlPoint();
  void OnTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
  void OnTableViewItemClicked(const QModelIndex& selectedIndex);

private:

  QPixmap GetPixmapFromImageNode(const mitk::DataNode* dataNode) const;

  // the Qt parent of our GUI
  QWidget* m_Parent;
  Ui::QmitkSemanticRelationControls m_Controls;
  QMenu* m_ContextMenu;

  std::unique_ptr<QmitkSemanticRelationsDataModel> m_SemanticRelationsDataModel;
  std::unique_ptr<mitk::SemanticRelations> m_SemanticRelations;

  mitk::DataNode* m_SelectedDataNode;
};

#endif // QMITKSEMANTICRELATIONSVIEW_H
