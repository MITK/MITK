#include "QmitkThresholdAction.h"

// MITK
#include <mitkBinaryThresholdTool.h>
#include <mitkRenderingManager.h>
#include <QmitkToolGUI.h>

// Qt
#include <QDialog>
#include <QVBoxLayout>

using namespace berry;
using namespace mitk;
using namespace std;

QmitkThresholdAction::QmitkThresholdAction()
  : m_ThresholdingDialog(NULL)
{ 
}

QmitkThresholdAction::~QmitkThresholdAction()
{
}

void QmitkThresholdAction::Run(const vector<DataNode *> &selectedNodes)
{
  m_ThresholdingToolManager = ToolManager::New(m_DataStorage);

  m_ThresholdingToolManager->RegisterClient();
  m_ThresholdingToolManager->ActiveToolChanged += mitk::MessageDelegate<QmitkThresholdAction>(this, &QmitkThresholdAction::OnThresholdingToolManagerToolModified);

  m_ThresholdingDialog = new QDialog;
  connect(m_ThresholdingDialog, SIGNAL(finished(int)), this, SLOT(ThresholdingDone(int)));

  QVBoxLayout *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);

  Tool *binaryThresholdTool = m_ThresholdingToolManager->GetToolById(m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>());
  
  if (binaryThresholdTool != NULL)
  {
    QmitkToolGUI *gui = dynamic_cast<QmitkToolGUI *>(binaryThresholdTool->GetGUI("Qmitk", "GUI").GetPointer());
    
    if (gui != NULL)
    {
      gui->SetTool(binaryThresholdTool);
      gui->setParent(m_ThresholdingDialog);

      layout->addWidget(gui);

      m_ThresholdingDialog->setLayout(layout);
      m_ThresholdingDialog->setFixedSize(300, 80);

      m_ThresholdingDialog->open();
    }

    m_ThresholdingToolManager->SetReferenceData(selectedNodes[0]);
    m_ThresholdingToolManager->ActivateTool(m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>());
  }
}

void QmitkThresholdAction::ThresholdingDone(int result)
{
  if (result == QDialog::Rejected)
    m_ThresholdingToolManager->ActivateTool(-1);

  m_ThresholdingDialog->deleteLater();
  m_ThresholdingDialog = NULL;

  m_ThresholdingToolManager->SetReferenceData(NULL);
  m_ThresholdingToolManager->SetWorkingData(NULL);

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkThresholdAction::OnThresholdingToolManagerToolModified()
{
  if (m_ThresholdingToolManager.IsNotNull())
    if (m_ThresholdingToolManager->GetActiveToolID() < 0)
      if (m_ThresholdingDialog != NULL)
        m_ThresholdingDialog->accept();
}

void QmitkThresholdAction::SetDataStorage(DataStorage *dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkThresholdAction::SetStdMultiWidget(QmitkStdMultiWidget *)
{
}

void QmitkThresholdAction::SetSmoothed(bool)
{
}

void QmitkThresholdAction::SetDecimated(bool)
{
}

void QmitkThresholdAction::SetFunctionality(QtViewPart *functionality)
{
}
