/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// semantic relations plugin
#include "QmitkSemanticRelationsStatisticsView.h"

// semantic relations module
#include <mitkSemanticRelationsIntegration.h>
#include <mitkRelationStorage.h>

const std::string QmitkSemanticRelationsStatisticsView::VIEW_ID = "org.mitk.views.semanticrelationsstatistics";

QmitkSemanticRelationsStatisticsView::~QmitkSemanticRelationsStatisticsView()
{
  auto semanticRelationsIntegration = std::make_unique<mitk::SemanticRelationsIntegration>();
  semanticRelationsIntegration->RemoveObserver(this);
}

void QmitkSemanticRelationsStatisticsView::Update(const mitk::SemanticTypes::CaseID& caseID)
{
  AddToComboBox(caseID);
}

void QmitkSemanticRelationsStatisticsView::SetFocus()
{
  // nothing here
}

void QmitkSemanticRelationsStatisticsView::CreateQtPartControl(QWidget* parent)
{
  // create GUI elements
  m_Controls.setupUi(parent);

  m_StatisticsTreeModel = new QmitkStatisticsTreeModel(parent);
  m_StatisticsTreeModel->SetDataStorage(GetDataStorage());
  m_Controls.statisticsTreeView->setModel(m_StatisticsTreeModel);

  auto semanticRelationsIntegration = std::make_unique<mitk::SemanticRelationsIntegration>();
  semanticRelationsIntegration->AddObserver(this);

  SetUpConnections();
  const auto& allCaseIDs = mitk::RelationStorage::GetAllCaseIDs();
  for (const auto& caseID : allCaseIDs)
  {
    AddToComboBox(caseID);
  }
}

void QmitkSemanticRelationsStatisticsView::SetUpConnections()
{
  connect(m_Controls.caseIDComboBox, static_cast<void (QComboBox::*)(const QString&)>(&QComboBox::currentIndexChanged), this, &QmitkSemanticRelationsStatisticsView::OnCaseIDSelectionChanged);
  connect(m_StatisticsTreeModel, &QmitkStatisticsTreeModel::ModelUpdated, this, &QmitkSemanticRelationsStatisticsView::OnModelUpdated);
}

void QmitkSemanticRelationsStatisticsView::OnCaseIDSelectionChanged(const QString& caseID)
{
  m_StatisticsTreeModel->SetCaseID(caseID.toStdString());
}

void QmitkSemanticRelationsStatisticsView::OnModelUpdated()
{
  m_Controls.statisticsTreeView->expandAll();
  int columns = m_Controls.statisticsTreeView->model()->columnCount();
  for (int i = 0; i < columns; ++i)
  {
    m_Controls.statisticsTreeView->resizeColumnToContents(i);
  }
}

void QmitkSemanticRelationsStatisticsView::AddToComboBox(const mitk::SemanticTypes::CaseID& caseID)
{
  int foundIndex = m_Controls.caseIDComboBox->findText(QString::fromStdString(caseID));
  if (-1 == foundIndex)
  {
    // add the caseID to the combo box, as it is not already contained
    m_Controls.caseIDComboBox->addItem(QString::fromStdString(caseID));
  }
}
