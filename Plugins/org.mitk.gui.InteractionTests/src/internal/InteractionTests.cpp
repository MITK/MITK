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

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "InteractionTests.h"

// Qt
#include <QMessageBox>

// rest

#include "mitkTestInteractor.h"
#include "mitkDataNode.h"

const std::string InteractionTests::VIEW_ID = "org.mitk.views.interactiontests";

void InteractionTests::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void InteractionTests::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DoImageProcessing()));
}

void InteractionTests::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  // iterate all selected objects, adjust warning visibility
  foreach( mitk::DataNode::Pointer node, nodes ){
  if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
  {
    m_CurrentImage = node;
    m_Controls.labelWarning->setVisible( false );
    m_Controls.buttonPerformImageProcessing->setEnabled( true );
    return;
  }
}

m_Controls.labelWarning->setVisible( true );
m_Controls.buttonPerformImageProcessing->setEnabled( false );
}

void InteractionTests::DoImageProcessing()
{

  m_CurrentDataNode = mitk::DataNode::New();

  GetDataStorage()->Add(m_CurrentDataNode.GetPointer(),m_CurrentImage);

  m_CurrentInteractor = mitk::TestInteractor::New();
  m_CurrentInteractor->LoadStateMachine("/home.local/webechr.local/EclipseTest/test/AddAndRemovePoints.xml");
  m_CurrentInteractor->LoadEventConfig("/home.local/webechr.local/EclipseTest/test/globalConfig.xml");
  m_CurrentInteractor->AddEventConfig("/home.local/webechr.local/EclipseTest/test/PointsConfig.xml");
  m_CurrentInteractor->SetDataNode(m_CurrentDataNode);

}
