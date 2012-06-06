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
#include "QmitkThresholdAction.h"

// MITK
#include <mitkBinaryThresholdTool.h>
#include <mitkRenderingManager.h>
#include <QmitkToolGUI.h>

// Qt
#include <QDialog>
#include <QVBoxLayout>
#include <QApplication>
#include <QDialogButtonBox>

using namespace berry;
using namespace mitk;
using namespace std;

QmitkThresholdAction::QmitkThresholdAction()
{ 
}

QmitkThresholdAction::~QmitkThresholdAction()
{
}

void QmitkThresholdAction::Run(const QList<DataNode::Pointer> &selectedNodes)
{
  m_ThresholdingToolManager = ToolManager::New(m_DataStorage);

  m_ThresholdingToolManager->RegisterClient();

  Tool *binaryThresholdTool = m_ThresholdingToolManager->GetToolById(m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>());
  if (binaryThresholdTool != NULL)
  {
    QmitkToolGUI *gui = dynamic_cast<QmitkToolGUI *>(binaryThresholdTool->GetGUI("Qmitk", "GUI").GetPointer()); 
    if (gui != NULL)
    {
      QDialog ThresholdingDialog(QApplication::activeWindow());

      QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
      connect(buttonBox, SIGNAL(rejected()), &ThresholdingDialog, SLOT(reject()));
      connect(gui, SIGNAL(thresholdConfirmed()), &ThresholdingDialog, SLOT(reject()));

      QVBoxLayout *layout = new QVBoxLayout;
      layout->setContentsMargins(0, 0, 0, 0);

      gui->SetTool(binaryThresholdTool);
      gui->setParent(&ThresholdingDialog);

      layout->addWidget(gui);

      ThresholdingDialog.setLayout(layout);
      ThresholdingDialog.setFixedHeight(135);
      ThresholdingDialog.setMinimumWidth(350);

      layout->addWidget(buttonBox);

      m_SelectedNode = selectedNodes[0];
      m_ThresholdingToolManager->SetReferenceData(selectedNodes[0]);
      m_ThresholdingToolManager->ActivateTool(m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>());

      m_ThresholdingToolManager->ActiveToolChanged += mitk::MessageDelegate<QmitkThresholdAction>(this, &QmitkThresholdAction::OnThresholdingToolManagerToolModified);
      ThresholdingDialog.exec();
      m_ThresholdingToolManager->ActiveToolChanged -= mitk::MessageDelegate<QmitkThresholdAction>(this, &QmitkThresholdAction::OnThresholdingToolManagerToolModified);

      m_ThresholdingToolManager->SetReferenceData(NULL);
      m_ThresholdingToolManager->ActivateTool(-1);
      m_SelectedNode = 0;
    }
  }

  m_ThresholdingToolManager->UnregisterClient();
}

void QmitkThresholdAction::OnThresholdingToolManagerToolModified()
{
  if (m_ThresholdingToolManager.IsNotNull())
  {
    if (m_ThresholdingToolManager->GetActiveToolID() < 0)
    {
      m_ThresholdingToolManager->SetReferenceData(m_SelectedNode);
      m_ThresholdingToolManager->ActivateTool(m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>());
    }
  }
}

void QmitkThresholdAction::SetDataStorage(DataStorage *dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkThresholdAction::SetSmoothed(bool)
{
}

void QmitkThresholdAction::SetDecimated(bool)
{
}

void QmitkThresholdAction::SetFunctionality(QtViewPart* /*functionality*/)
{
}
