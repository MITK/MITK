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

// semantic relations plugin
#include "QmitkSemanticRelationsView.h"

// semantic relations module
#include "mitkSemanticRelationsTestHelper.h"
#include "mitkNodePredicates.h"

// blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

const std::string QmitkSemanticRelationsView::VIEW_ID = "org.mitk.views.semanticrelations";

void QmitkSemanticRelationsView::SetFocus()
{
  // nothing here
}

void QmitkSemanticRelationsView::CreateQtPartControl(QWidget* parent)
{
  // create GUI widgets
  m_Controls.setupUi(parent);

  m_SemanticRelationsTableView = new QmitkSemanticRelationsTableView(GetDataStorage(), parent);
  m_SemanticRelationsTableView->setObjectName(QStringLiteral("m_SemanticRelationsTableView"));
  m_Controls.verticalLayout->addWidget(m_SemanticRelationsTableView);

  m_PatientInfoWidget = new QmitkPatientInfoWidget(parent);
  m_Controls.verticalLayout->addWidget(m_PatientInfoWidget);
  m_PatientInfoWidget->hide();

  connect(m_SemanticRelationsTableView, SIGNAL(SelectionChanged(const mitk::DataNode*)), SLOT(OnTableViewSelectionChanged(const mitk::DataNode*)));
  connect(m_SemanticRelationsTableView, SIGNAL(DataChanged(const mitk::SemanticTypes::CaseID&)), SLOT(OnSemanticRelationsDataChanged(const mitk::SemanticTypes::CaseID&)));
}

void QmitkSemanticRelationsView::NodeAdded(const mitk::DataNode* node)
{
  bool helperObject = false;
  node->GetBoolProperty("helper object", helperObject);
  if (helperObject)
  { 
    return;
  }

  // not a helper object
  if (mitk::NodePredicates::GetImagePredicate()->CheckNode(node))
  {
    // add the image to the semantic relations storage (and trigger GUI elements to change)
    m_SemanticRelationsTableView->AddImageInstance(node);
  }
  else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(node))
  {
    // get parent node of the current segmentation node with the node predicate
    mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = GetDataStorage()->GetSources(node, mitk::NodePredicates::GetImagePredicate(), false);
    // add the segmentation to the semantic relations storage (and receive a signal to change the lesion list)
    m_SemanticRelationsTableView->AddSegmentationInstance(node, parentNodes->front());
  }
}

void QmitkSemanticRelationsView::NodeRemoved(const mitk::DataNode* node)
{
  bool helperObject = false;
  node->GetBoolProperty("helper object", helperObject);
  if (helperObject)
  {
    return;
  }

  // not a helper object
  if (mitk::NodePredicates::GetImagePredicate()->CheckNode(node))
  {
    // remove the image from the semantic relations storage (and trigger GUI elements to change)
    m_SemanticRelationsTableView->RemoveImageInstance(node);
  }
  else if (mitk::NodePredicates::GetSegmentationPredicate()->CheckNode(node))
  {
    // get parent node of the current segmentation node with the node predicate
    mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = GetDataStorage()->GetSources(node, mitk::NodePredicates::GetImagePredicate(), false);
    // remove the segmentation from the semantic relations storage (and receive a signal to change the lesion list)
    m_SemanticRelationsTableView->RemoveSegmentationInstance(node, parentNodes->front());
  }
}

void QmitkSemanticRelationsView::OnTableViewSelectionChanged(const mitk::DataNode* node)
{
  m_PatientInfoWidget->SetPatientInfo(node);
  m_PatientInfoWidget->show();
}

void QmitkSemanticRelationsView::OnSemanticRelationsDataChanged(const mitk::SemanticTypes::CaseID& caseID)
{
  // update lesion list here
}
