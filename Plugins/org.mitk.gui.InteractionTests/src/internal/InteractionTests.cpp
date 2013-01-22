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

#include "mitkDataNode.h"
#include <vector>

// us
#include "mitkGetModuleContext.h"
#include "mitkModule.h"
#include "mitkModuleRegistry.h"
#include "mitkInformer.h"

#include "mitkEventObserver.h"

//#include "mitkInformerActivator.h"

const std::string InteractionTests::VIEW_ID = "org.mitk.views.interactiontests";

void InteractionTests::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void InteractionTests::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonPerformImageProcessing, SIGNAL(clicked()), this, SLOT(DrawLines()));
  connect(m_Controls.toggleConfig, SIGNAL(clicked()), this, SLOT(ToggleConfig()));
  connect(m_Controls.grabInput, SIGNAL(clicked()), this, SLOT(GrabInput()));
  m_Controls.buttonPerformImageProcessing->setText(QString("Line Interactor (Left Click)"));
  m_ConfigChoice = true;
}

void InteractionTests::ToggleConfig()
{
  std::vector<mitk::DataNode*> selections = GetDataManagerSelection();
  for (std::vector<mitk::DataNode*>::iterator it = selections.begin(); it != selections.end(); ++it)
  {
    if ((*it)->GetDataInteractor().IsNotNull())
    {
      if (!m_ConfigChoice)
      {
        (*it)->GetDataInteractor()->LoadEventConfig("/home.local/webechr.local/EclipseTest/test/PointsConfig.xml");
        (*it)->GetDataInteractor()->AddEventConfig("/home.local/webechr.local/EclipseTest/test/globalConfig.xml");
        m_Controls.buttonPerformImageProcessing->setText(QString("Line Interactor (Left Click)"));
        m_ConfigChoice = true;
      }
      else
      {
        (*it)->GetDataInteractor()->LoadEventConfig("/home.local/webechr.local/EclipseTest/test/PointsConfigRightClick.xml");
        (*it)->GetDataInteractor()->AddEventConfig("/home.local/webechr.local/EclipseTest/test/globalConfig.xml");
        m_Controls.buttonPerformImageProcessing->setText(QString("Line Interactor (Right Click)"));
        m_ConfigChoice = false;
      }
    }
  }
}

void InteractionTests::GrabInput()
{
  m_CurrentDataNode = mitk::DataNode::New();
  GetDataStorage()->Add(m_CurrentDataNode.GetPointer(), m_CurrentImage);

  m_CurrentInteractor = mitk::PointSetDataInteractor::New();
  if (!m_CurrentInteractor->LoadStateMachine("/home.local/webechr.local/EclipseTest/test/LineSMGrab.xml"))
  {
    return;
  }
  m_CurrentInteractor->LoadEventConfig("/home.local/webechr.local/EclipseTest/test/PointsConfig.xml");
  m_CurrentInteractor->AddEventConfig("/home.local/webechr.local/EclipseTest/test/globalConfig.xml");
  m_CurrentInteractor->SetDataNode(m_CurrentDataNode);
}

void InteractionTests::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/, const QList<mitk::DataNode::Pointer>& nodes)
{
  // iterate all selected objects, adjust warning visibility
  foreach ( mitk::DataNode::Pointer node, nodes ){
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

void InteractionTests::DrawLines()
{
  m_CurrentDataNode = mitk::DataNode::New();
  GetDataStorage()->Add(m_CurrentDataNode.GetPointer(), m_CurrentImage);
  //m_CurrentInteractor = mitk::TestInteractor::New();
  m_CurrentInteractor = mitk::PointSetDataInteractor::New();
  m_CurrentInteractor->LoadStateMachine("/home.local/webechr.local/EclipseTest/test/PointSet.xml");
  /* if (!m_CurrentInteractor->LoadStateMachine("/home.local/webechr.local/EclipseTest/test/LineSM.xml"))
   {
   return;
   }*/

  if (m_ConfigChoice)
  {
    m_CurrentInteractor->LoadEventConfig("/home.local/webechr.local/EclipseTest/test/PointsConfig.xml");
    m_CurrentInteractor->AddEventConfig("/home.local/webechr.local/EclipseTest/test/globalConfig.xml");
  }
  else
  {
    m_CurrentInteractor->LoadEventConfig("/home.local/webechr.local/EclipseTest/test/PointsConfigRightClick.xml");
    m_CurrentInteractor->AddEventConfig("/home.local/webechr.local/EclipseTest/test/globalConfig.xml");
  }
  m_CurrentInteractor->SetDataNode(m_CurrentDataNode);

  mitk::EventObserver::Pointer eO = mitk::EventObserver::New();
  mitk::ModuleContext* context = mitk::ModuleRegistry::GetModule(1)->GetModuleContext();
  mitk::ServiceReference serviceRef = context->GetServiceReference<mitk::InformerService>();
  mitk::InformerService* service = dynamic_cast<mitk::InformerService*>(context->GetService(serviceRef));
  service->RegisterObserver(eO);

}
