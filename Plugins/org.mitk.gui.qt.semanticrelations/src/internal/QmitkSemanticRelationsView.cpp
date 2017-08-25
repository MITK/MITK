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
}

void QmitkSemanticRelationsView::NodeAdded(const mitk::DataNode* node)
{
  bool helperObject = false;
  node->GetBoolProperty("helper object", helperObject);
  if (helperObject)
  { 
    return;
  }

  // not a helper object; add the image to the table view widget
  m_SemanticRelationsTableView->AddImageInstance(node);
}

void QmitkSemanticRelationsView::NodeRemoved(const mitk::DataNode* node)
{
  bool helperObject = false;
  node->GetBoolProperty("helper object", helperObject);
  if (helperObject)
  {
    return;
  }

  // not a helper object; remove the image from the table view widget
  m_SemanticRelationsTableView->RemoveImageInstance(node);
}

void QmitkSemanticRelationsView::OnTableViewSelectionChanged(const mitk::DataNode* node)
{
  m_PatientInfoWidget->SetPatientInfo(node);
  m_PatientInfoWidget->show();
}
