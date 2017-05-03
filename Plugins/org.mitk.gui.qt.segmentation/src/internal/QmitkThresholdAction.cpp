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
#include <QmitkBinaryThresholdToolGUI.h>

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
    QmitkBinaryThresholdToolGUI *gui = dynamic_cast<QmitkBinaryThresholdToolGUI *>(binaryThresholdTool->GetGUI("Qmitk", "GUI").GetPointer());
    if (gui != NULL)
    {
      QDialog thresholdingDialog(QApplication::activeWindow(), Qt::Window | Qt::WindowStaysOnTopHint);

      thresholdingDialog.setWindowFlags(thresholdingDialog.windowFlags() & ~Qt::WindowMinimizeButtonHint);

      QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Cancel);
      connect(buttonBox, SIGNAL(rejected()), &thresholdingDialog, SLOT(reject()));
      connect(gui, SIGNAL(thresholdAccepted()), &thresholdingDialog, SLOT(reject()));

      QVBoxLayout *layout = new QVBoxLayout;
      layout->setContentsMargins(3, 3, 3, 3);

      gui->SetTool(binaryThresholdTool);
      gui->setParent(&thresholdingDialog);

      layout->addWidget(gui);
      layout->addWidget(buttonBox);

      thresholdingDialog.setLayout(layout);
      thresholdingDialog.setMinimumWidth(350);

      m_SelectedNode = selectedNodes[0];
      m_ThresholdingToolManager->SetReferenceData(selectedNodes[0]);
      m_ThresholdingToolManager->ActivateTool(m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>());

      m_ThresholdingToolManager->ActiveToolChanged += mitk::MessageDelegate<QmitkThresholdAction>(this, &QmitkThresholdAction::OnThresholdingToolManagerToolModified);
      thresholdingDialog.exec();
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

void QmitkThresholdAction::SetFunctionality(QtViewPart* /*view*/)
{
}
