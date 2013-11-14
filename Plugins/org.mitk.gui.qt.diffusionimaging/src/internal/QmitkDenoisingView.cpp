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
#include "QmitkDenoisingView.h"
#include <mitkImageCast.h>
#include <mitkProgressBar.h>

QmitkDenoisingWorker::QmitkDenoisingWorker(QmitkDenoisingView *view)
  : m_View(view)
{

}

void QmitkDenoisingWorker::run()
{
  if (m_View->m_ImageNode.IsNotNull() && m_View->m_BrainMaskNode.IsNotNull())
  {
    switch (m_View->m_SelectedFilter)
    {
      case 0:
      {
        break;
      }
      case 1:
      case 2:
      {
        m_View->m_NonLocalMeansFilter->Update();
        break;
      }
    }
    m_View->m_DenoisingThread.quit();
  }
}

const std::string QmitkDenoisingView::VIEW_ID = "org.mitk.views.denoisingview";

QmitkDenoisingView::QmitkDenoisingView()
  : QmitkFunctionality()
  , m_Controls( 0 )
  , m_ImageNode(NULL)
  , m_BrainMaskNode(NULL)
  , m_DenoisingWorker(this)
  , m_ThreadIsRunning(false)
  , m_NonLocalMeansFilter(NULL)
  , m_InputImage(NULL)
{
  m_DenoisingWorker.moveToThread(&m_DenoisingThread);
  connect(&m_DenoisingThread, SIGNAL(started()), this, SLOT(BeforeThread()));
  connect(&m_DenoisingThread, SIGNAL(started()), &m_DenoisingWorker, SLOT(run()));
  connect(&m_DenoisingThread, SIGNAL(finished()), this, SLOT(AfterThread()));
  connect(&m_DenoisingThread, SIGNAL(terminated()), this, SLOT(AfterThread()));
  m_DenoisingTimer = new QTimer(this);
}

QmitkDenoisingView::~QmitkDenoisingView()
{
  delete m_DenoisingTimer;
}

void QmitkDenoisingView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkDenoisingViewControls;
    m_Controls->setupUi( parent );
    CreateConnections();
    ResetParameterPanel();
  }
}

void QmitkDenoisingView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ApplyButton), SIGNAL(clicked()), this, SLOT(StartDenoising()));
    connect( (QObject*)(m_Controls->m_SelectFilterComboBox), SIGNAL(activated(int)), this, SLOT(SelectFilter(int)));
    connect( m_DenoisingTimer, SIGNAL(timeout()), this, SLOT(UpdateProgress()));
  }
}

void QmitkDenoisingView::Activated()
{
  QmitkFunctionality::Activated();

  m_Controls->m_SelectFilterComboBox->clear();
  m_Controls->m_SelectFilterComboBox->insertItem(NOFILTERSELECTED, QString( QApplication::translate("QmitkDenoisingView", "Please select a filter", 0, QApplication::UnicodeUTF8) ));
  m_Controls->m_SelectFilterComboBox->insertItem(NLMR, QString( QApplication::translate("QmitkDenoisingView", "Non local means filter", 0, QApplication::UnicodeUTF8) ));
  m_Controls->m_SelectFilterComboBox->insertItem(NLMV, QString( QApplication::translate("QmitkDenoisingView", "Non local means filter with joint information", 0, QApplication::UnicodeUTF8) ));
}

void QmitkDenoisingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  if (m_ThreadIsRunning)
    return;

  m_Controls->m_InputData->setTitle("Please Select Input Data");
  m_Controls->m_InputImageLabel->setText("<font color='red'>mandatory</font>");
  m_Controls->m_InputBrainMaskLabel->setText("<font color='red'>mandatory</font>");
  m_Controls->m_ApplyButton->setEnabled(false);

  m_ImageNode = NULL;
  m_BrainMaskNode = NULL;

  // iterate selection
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<DiffusionImageType*>(node->GetData()))
    {
      m_Controls->m_InputImageLabel->setText(node->GetName().c_str());
      m_ImageNode = node;
    }

    bool isBinary = false;
    node->GetBoolProperty("binary", isBinary);
    if( node.IsNotNull() && static_cast<mitk::Image*>(node->GetData()) && isBinary)
    {
        m_Controls->m_InputBrainMaskLabel->setText(node->GetName().c_str());
        m_BrainMaskNode = node;

    }
  }

  if (m_ImageNode.IsNotNull() && m_BrainMaskNode.IsNotNull())
  {
    m_Controls->m_InputData->setTitle("Input Data");
    if(m_SelectedFilter != NOFILTERSELECTED)
    {
      m_Controls->m_ApplyButton->setEnabled(true);
    }
  }
}

void QmitkDenoisingView::StartDenoising()
{
  if (m_ImageNode.IsNotNull() && m_BrainMaskNode.IsNotNull())
  {
    switch (m_SelectedFilter)
    {
      case 0:
      {
        break;
      }
      case 1:
      {
        m_InputImage = dynamic_cast<DiffusionImageType*> (m_ImageNode->GetData());
        MaskImageType::Pointer imageMask = MaskImageType::New();
        mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_BrainMaskNode->GetData()), imageMask);

        m_NonLocalMeansFilter = NonLocalMeansDenoisingFilterType::New();
        m_NonLocalMeansFilter->SetNumberOfThreads(12);
        m_NonLocalMeansFilter->SetInputImage(m_InputImage->GetVectorImage());
        m_NonLocalMeansFilter->SetInputMask(imageMask);
        m_NonLocalMeansFilter->SetUseJointInformation(false);
        m_NonLocalMeansFilter->SetVRadius(m_Controls->m_SpinBoxParameter1->value());
        m_NonLocalMeansFilter->SetNRadius(m_Controls->m_SpinBoxParameter2->value());

        break;
      }
      case 2:
      {
        m_InputImage = dynamic_cast<DiffusionImageType*> (m_ImageNode->GetData());
        MaskImageType::Pointer imageMask = MaskImageType::New();
        mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_BrainMaskNode->GetData()), imageMask);

        m_NonLocalMeansFilter = NonLocalMeansDenoisingFilterType::New();
        m_NonLocalMeansFilter->SetNumberOfThreads(12);
        m_NonLocalMeansFilter->SetInputImage(m_InputImage->GetVectorImage());
        m_NonLocalMeansFilter->SetInputMask(imageMask);
        m_NonLocalMeansFilter->SetUseJointInformation(true);
        m_NonLocalMeansFilter->SetVRadius(m_Controls->m_SpinBoxParameter1->value());
        m_NonLocalMeansFilter->SetNRadius(m_Controls->m_SpinBoxParameter2->value());

        break;
      }
    }
    unsigned int maxVoxelCount = m_InputImage->GetDimension(0) * m_InputImage->GetDimension(1) * m_InputImage->GetDimension(2);
    mitk::ProgressBar::GetInstance()->AddStepsToDo(maxVoxelCount);
    m_LastVoxelCount = 0;
    m_DenoisingThread.start(QThread::HighestPriority);
  }
}

void QmitkDenoisingView::ResetParameterPanel()
{
  m_Controls->m_LabelParameter_1->setEnabled(false);
  m_Controls->m_LabelParameter_1->setText("Parameter 1:");
  m_Controls->m_LabelParameter_2->setEnabled(false);
  m_Controls->m_LabelParameter_2->setText("Parameter 2:");
  m_Controls->m_LabelParameter_3->setEnabled(false);
  m_Controls->m_LabelParameter_3->setText("Parameter 3:");
  m_Controls->m_LabelParameter_3->hide();
  m_Controls->m_SpinBoxParameter1->setEnabled(false);
  m_Controls->m_SpinBoxParameter1->setValue(1);
  m_Controls->m_SpinBoxParameter2->setEnabled(false);
  m_Controls->m_SpinBoxParameter2->setValue(1);
  m_Controls->m_DoubleSpinBoxParameter3->setEnabled(false);
  m_Controls->m_DoubleSpinBoxParameter3->setValue(1);
  m_Controls->m_DoubleSpinBoxParameter3->hide();
  m_Controls->m_ApplyButton->setEnabled(false);
}

void QmitkDenoisingView::SelectFilter(int filter)
{
  if (m_ThreadIsRunning)
    return;

  //Prepare GUI
  this->ResetParameterPanel();

  switch (filter)
  {
    case 0:
    {
      m_SelectedFilter = NOFILTERSELECTED;

      break;
    }
    case 1:
    {
      m_SelectedFilter = NLMR;
      m_Controls->m_LabelParameter_1->setEnabled(true);
      m_Controls->m_LabelParameter_1->setText("Neigborhood V:");
      m_Controls->m_LabelParameter_2->setEnabled(true);
      m_Controls->m_LabelParameter_2->setText("Neighborhood N:");
      m_Controls->m_SpinBoxParameter1->setEnabled(true);
      m_Controls->m_SpinBoxParameter1->setValue(7);
      m_Controls->m_SpinBoxParameter2->setEnabled(true);
      m_Controls->m_SpinBoxParameter2->setValue(3);

      break;
    }
    case 2:
    {
      m_SelectedFilter = NLMV;
      m_Controls->m_LabelParameter_1->setEnabled(true);
      m_Controls->m_LabelParameter_1->setText("Neigborhood V:");
      m_Controls->m_LabelParameter_2->setEnabled(true);
      m_Controls->m_LabelParameter_2->setText("Neighborhood N:");
      m_Controls->m_SpinBoxParameter1->setEnabled(true);
      m_Controls->m_SpinBoxParameter1->setValue(7);
      m_Controls->m_SpinBoxParameter2->setEnabled(true);
      m_Controls->m_SpinBoxParameter2->setValue(3);

      break;
    }
  }

  if (m_ImageNode.IsNotNull() && m_BrainMaskNode.IsNotNull() && m_SelectedFilter != NOFILTERSELECTED)
  {
    m_Controls->m_ApplyButton->setEnabled(true);
  }
}

void QmitkDenoisingView::BeforeThread()
{
  m_ThreadIsRunning = true;
  m_DenoisingTimer->start(1000);
  m_Controls->m_LabelParameter_1->setEnabled(false);
  m_Controls->m_LabelParameter_2->setEnabled(false);
  m_Controls->m_SpinBoxParameter1->setEnabled(false);
  m_Controls->m_SpinBoxParameter2->setEnabled(false);
  m_Controls->m_SelectFilterComboBox->setEnabled(false);
  m_Controls->m_ApplyButton->setEnabled(false);
}

void QmitkDenoisingView::AfterThread()
{
  m_ThreadIsRunning = false;
  m_DenoisingTimer->stop();
  unsigned int currentVoxelCount = m_NonLocalMeansFilter->GetCurrentVoxelCount();
  mitk::ProgressBar::GetInstance()->Progress(currentVoxelCount-m_LastVoxelCount+1);
  m_LastVoxelCount = currentVoxelCount;
  switch (m_SelectedFilter)
  {
    case 0:
    {
      break;
    }

    case 1:
    {
      DiffusionImageType::Pointer image = DiffusionImageType::New();
      image->SetVectorImage(m_NonLocalMeansFilter->GetOutput());
      image->SetB_Value(m_InputImage->GetB_Value());
      image->SetDirections(m_InputImage->GetDirections());
      image->InitializeFromVectorImage();
      mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
      imageNode->SetData( image );
      QString name = m_ImageNode->GetName().c_str();

      imageNode->SetName((name+"_NLMr_V"+QString::number(m_Controls->m_SpinBoxParameter1->value())+"_N"+QString::number(m_Controls->m_SpinBoxParameter2->value())).toStdString().c_str());
      GetDefaultDataStorage()->Add(imageNode);
      break;
    }

    case 2:
    {
      DiffusionImageType::Pointer image = DiffusionImageType::New();
      image->SetVectorImage(m_NonLocalMeansFilter->GetOutput());
      image->SetB_Value(m_InputImage->GetB_Value());
      image->SetDirections(m_InputImage->GetDirections());
      image->InitializeFromVectorImage();
      mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
      imageNode->SetData( image );
      QString name = m_ImageNode->GetName().c_str();

      imageNode->SetName((name+"_NLMv_V"+QString::number(m_Controls->m_SpinBoxParameter1->value())+"_N"+QString::number(m_Controls->m_SpinBoxParameter2->value())).toStdString().c_str());
      GetDefaultDataStorage()->Add(imageNode);
      break;
    }
  }

  m_Controls->m_LabelParameter_1->setEnabled(true);
  m_Controls->m_LabelParameter_2->setEnabled(true);
  m_Controls->m_SpinBoxParameter1->setEnabled(true);
  m_Controls->m_SpinBoxParameter2->setEnabled(true);
  m_Controls->m_SelectFilterComboBox->setEnabled(true);
  m_Controls->m_ApplyButton->setEnabled(true);
}

void QmitkDenoisingView::UpdateProgress()
{
  unsigned int currentVoxelCount = m_NonLocalMeansFilter->GetCurrentVoxelCount();
  mitk::ProgressBar::GetInstance()->Progress(currentVoxelCount-m_LastVoxelCount);
  m_LastVoxelCount = currentVoxelCount;
}
