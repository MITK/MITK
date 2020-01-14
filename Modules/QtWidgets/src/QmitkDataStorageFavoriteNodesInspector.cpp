/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkDataStorageFavoriteNodesInspector.h>

#include <QmitkDataStorageDefaultListModel.h>

#include "mitkNodePredicateAnd.h"

#include "QPushButton"

QmitkDataStorageFavoriteNodesInspector::QmitkDataStorageFavoriteNodesInspector(QWidget* parent/* = nullptr*/)
  : QmitkDataStorageListInspector(parent)
{
  auto favoriteNodesButton = new QPushButton("Remove from favorites", parent);
  QmitkDataStorageListInspector::m_Controls.verticalLayout->addWidget(favoriteNodesButton, 0, Qt::AlignRight);

  m_FavoriteNodeSelectionPredicate = mitk::NodePredicateProperty::New("org.mitk.selection.favorite", mitk::BoolProperty::New(true));
  m_NodePredicate = m_FavoriteNodeSelectionPredicate;

  connect(favoriteNodesButton, &QPushButton::clicked, this, &QmitkDataStorageFavoriteNodesInspector::OnFavoriteNodesButtonClicked);
}

void QmitkDataStorageFavoriteNodesInspector::SetNodePredicate(mitk::NodePredicateBase* nodePredicate)
{
  mitk::NodePredicateAnd::Pointer combinedPredicate = mitk::NodePredicateAnd::New();

  combinedPredicate->AddPredicate(m_FavoriteNodeSelectionPredicate);
  combinedPredicate->AddPredicate(nodePredicate);

  QmitkDataStorageListInspector::SetNodePredicate(combinedPredicate);
}

void QmitkDataStorageFavoriteNodesInspector::UpdateView()
{
  dynamic_cast<QmitkDataStorageDefaultListModel*>(m_StorageModel)->NodePredicateChanged();
}

void QmitkDataStorageFavoriteNodesInspector::OnFavoriteNodesButtonClicked()
{
  auto selectedNodes = GetSelectedNodes();
  for (auto node : selectedNodes)
  {
    node->SetBoolProperty("org.mitk.selection.favorite", false);
  }

  UpdateView();
}
