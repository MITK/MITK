#include "QmitkThresholdAction.h"

#include "mitkBinaryThresholdTool.h"
#include "mitkRenderingManager.h"

#include "QmitkToolGUI.h"
#include <QtGui>


QmitkThresholdAction::QmitkThresholdAction()
{ 
}

QmitkThresholdAction::~QmitkThresholdAction()
{
}

void QmitkThresholdAction::Run(const std::vector<mitk::DataNode*> &selectedNodes)
{
  m_ThresholdingToolManager = mitk::ToolManager::New( m_DataStorage );
  m_ThresholdingToolManager->RegisterClient();
  m_ThresholdingToolManager->ActiveToolChanged +=
  mitk::MessageDelegate<QmitkThresholdAction>( this, &QmitkThresholdAction::OnThresholdingToolManagerToolModified );

  m_ThresholdingDialog = new QDialog(NULL);
  connect( m_ThresholdingDialog, SIGNAL(finished(int)), this, SLOT(ThresholdingDone(int)) );

  QVBoxLayout* layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);

  mitk::Tool* tool = m_ThresholdingToolManager->GetToolById( m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>() );
  if (tool)
  {
    itk::Object::Pointer possibleGUI = tool->GetGUI("Qmitk", "GUI");
    QmitkToolGUI* gui = dynamic_cast<QmitkToolGUI*>( possibleGUI.GetPointer() );
    if (gui)
    {
      gui->SetTool(tool);
      gui->setParent(m_ThresholdingDialog);
      layout->addWidget(gui);
      m_ThresholdingDialog->setLayout(layout);
      layout->activate();
      m_ThresholdingDialog->setFixedSize(300,80);
      m_ThresholdingDialog->open();
    }
  }

  for ( NodeList::const_iterator iter = selectedNodes.begin(); iter != selectedNodes.end(); ++iter )
  {
    mitk::DataNode* node = *iter;

    if (node)
    {
      m_ThresholdingToolManager->SetReferenceData( node );
      m_ThresholdingToolManager->ActivateTool( m_ThresholdingToolManager->GetToolIdByToolType<mitk::BinaryThresholdTool>() );
    }
  }
}

void QmitkThresholdAction::ThresholdingDone(int result)
{
  if (result == QDialog::Rejected)
  m_ThresholdingToolManager->ActivateTool(-1);
  MITK_INFO << "Thresholding done, cleaning up";
  m_ThresholdingDialog->deleteLater();
  m_ThresholdingDialog = NULL;
  m_ThresholdingToolManager->SetReferenceData( NULL );
  m_ThresholdingToolManager->SetWorkingData( NULL );

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkThresholdAction::OnThresholdingToolManagerToolModified()
{
  if ( m_ThresholdingToolManager.IsNull() ) return;

  //MITK_INFO << "Now got tool " << m_ThresholdingToolManager->GetActiveToolID();

  if ( m_ThresholdingToolManager->GetActiveToolID() < 0)
  {
    if (m_ThresholdingDialog)
      m_ThresholdingDialog->accept();
  }
}

void QmitkThresholdAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  this->m_DataStorage = dataStorage;
}

void QmitkThresholdAction::SetSmoothed(bool smoothed)
{
  //not needed
}

void QmitkThresholdAction::SetDecimated(bool decimated)
{
  //not needed
}

void QmitkThresholdAction::SetFunctionality(berry::QtViewPart* functionality)
{
  //not needed 
}