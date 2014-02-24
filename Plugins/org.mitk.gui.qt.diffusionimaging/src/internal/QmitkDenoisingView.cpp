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
      case 3:
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
  , m_LastProgressCount(0)
  , m_MaxProgressCount(0)
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
  m_Controls->m_SelectFilterComboBox->insertItem(GAUSS, QString( QApplication::translate("QmitkDenoisingView", "Discrete gaussian filter", 0, QApplication::UnicodeUTF8) ));
}

void QmitkDenoisingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  if (m_ThreadIsRunning)
    return;

  if (m_SelectedFilter != NOFILTERSELECTED)
  {
    m_Controls->m_InputImageLabel->setText("<font color='red'>mandatory</font>");
    m_Controls->m_InputBrainMaskLabel->setText("<font color='red'>mandatory</font>");
  }
  else
  {
    m_Controls->m_InputImageLabel->setText("mandatory");
  }
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
    // look for a brainmask in selection
    if( node.IsNotNull() && static_cast<mitk::Image*>(node->GetData()) && isBinary)
    {
        m_Controls->m_InputBrainMaskLabel->setText(node->GetName().c_str());
        m_BrainMaskNode = node;

    }
  }

  // Preparation of GUI to start denoising if a filter is selected
  if (m_ImageNode.IsNotNull() && m_BrainMaskNode.IsNotNull())
  {
    if(m_SelectedFilter != NOFILTERSELECTED)
    {
      m_Controls->m_ApplyButton->setEnabled(true);
    }
  }
  else if (m_ImageNode.IsNotNull() && m_SelectedFilter == GAUSS)
  {
    m_Controls->m_ApplyButton->setEnabled(true);
  }
}

void QmitkDenoisingView::StartDenoising()
{
  if (m_ImageNode.IsNotNull() && m_BrainMaskNode.IsNotNull() && m_SelectedFilter != GAUSS)
  {
    m_LastProgressCount = 0;
    switch (m_SelectedFilter)
    {
      case NOFILTERSELECTED:
      case GAUSS:
      {
        break;
      }
      case NLMR:
      {
        // initialize NLMr
        m_InputImage = dynamic_cast<DiffusionImageType*> (m_ImageNode->GetData());
        m_ImageMask = dynamic_cast<mitk::Image*>(m_BrainMaskNode->GetData());
        itk::Image<DiffusionPixelType, 3>::Pointer itkMask = itk::Image<DiffusionPixelType, 3>::New();
        mitk::CastToItkImage(m_ImageMask, itkMask);

        m_NonLocalMeansFilter = NonLocalMeansDenoisingFilterType::New();
        m_NonLocalMeansFilter->SetNumberOfThreads(12);
        m_NonLocalMeansFilter->SetInputImage(m_InputImage->GetVectorImage());
        m_NonLocalMeansFilter->SetInputMask(itkMask);
        m_NonLocalMeansFilter->SetUseJointInformation(false);
        m_NonLocalMeansFilter->SetSearchRadius(m_Controls->m_SpinBoxParameter1->value());
        m_NonLocalMeansFilter->SetComparisonRadius(m_Controls->m_SpinBoxParameter2->value());

        // initialize the progressbar
        m_MaxProgressCount = m_InputImage->GetDimension(0) * m_InputImage->GetDimension(1) * m_InputImage->GetDimension(2);
        mitk::ProgressBar::GetInstance()->AddStepsToDo(m_MaxProgressCount);

        // start denoising in detached thread
        m_DenoisingThread.start(QThread::HighestPriority);

        break;
      }
      case NLMV:
      {
        // initialize NLMv
        m_InputImage = dynamic_cast<DiffusionImageType*> (m_ImageNode->GetData());
        m_ImageMask = dynamic_cast<mitk::Image*>(m_BrainMaskNode->GetData());
        itk::Image<DiffusionPixelType, 3>::Pointer itkMask = itk::Image<DiffusionPixelType, 3>::New();
        mitk::CastToItkImage(m_ImageMask, itkMask);

        m_NonLocalMeansFilter = NonLocalMeansDenoisingFilterType::New();
        m_NonLocalMeansFilter->SetNumberOfThreads(12);
        m_NonLocalMeansFilter->SetInputImage(m_InputImage->GetVectorImage());
        m_NonLocalMeansFilter->SetInputMask(itkMask);
        m_NonLocalMeansFilter->SetUseJointInformation(true);
        m_NonLocalMeansFilter->SetSearchRadius(m_Controls->m_SpinBoxParameter1->value());
        m_NonLocalMeansFilter->SetComparisonRadius(m_Controls->m_SpinBoxParameter2->value());
        m_NonLocalMeansFilter->SetChannelRadius(m_Controls->m_SpinBoxParameter3->value());

        // initialize the progressbar
        m_MaxProgressCount = m_InputImage->GetDimension(0) * m_InputImage->GetDimension(1) * m_InputImage->GetDimension(2);
        mitk::ProgressBar::GetInstance()->AddStepsToDo(m_MaxProgressCount);

        // start denoising in detached thread
        m_DenoisingThread.start(QThread::HighestPriority);

        break;
      }
    }
  }
  else if(m_SelectedFilter == GAUSS && m_ImageNode.IsNotNull())
  {
    // initialize GAUSS
    m_InputImage = dynamic_cast<DiffusionImageType*> (m_ImageNode->GetData());

    ExtractFilterType::Pointer extractor = ExtractFilterType::New();
    extractor->SetInput(m_InputImage->GetVectorImage());
    ComposeFilterType::Pointer composer = ComposeFilterType::New();

    for (unsigned int i = 0; i < m_InputImage->GetVectorImage()->GetVectorLength(); ++i)
    {
      extractor->SetIndex(i);
      extractor->Update();

      m_GaussianFilter = GaussianFilterType::New();
      m_GaussianFilter->SetInput(extractor->GetOutput());
      m_GaussianFilter->SetVariance(m_Controls->m_SpinBoxParameter1->value());
      m_GaussianFilter->Update();

      composer->SetInput(i, m_GaussianFilter->GetOutput());
    }
    composer->Update();


    DiffusionImageType::Pointer image = DiffusionImageType::New();
    image->SetVectorImage(composer->GetOutput());
    image->SetReferenceBValue(m_InputImage->GetReferenceBValue());
    image->SetDirections(m_InputImage->GetDirections());
    image->InitializeFromVectorImage();
    mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
    imageNode->SetData( image );
    QString name = m_ImageNode->GetName().c_str();

    imageNode->SetName((name+"_gauss_"+QString::number(m_Controls->m_SpinBoxParameter1->value())).toStdString().c_str());
    GetDefaultDataStorage()->Add(imageNode);
  }
}

void QmitkDenoisingView::ResetParameterPanel()
{
  m_Controls->m_DwiLabel->setEnabled(false);
  m_Controls->m_InputImageLabel->setEnabled(false);
  m_Controls->m_BrainMaskLabel->hide();
  m_Controls->m_InputBrainMaskLabel->hide();
  m_Controls->m_ParameterBox->hide();
  m_Controls->m_LabelParameter_1->hide();
  m_Controls->m_LabelParameter_2->hide();
  m_Controls->m_LabelParameter_3->hide();
  m_Controls->m_SpinBoxParameter1->hide();
  m_Controls->m_SpinBoxParameter2->hide();
  m_Controls->m_SpinBoxParameter3->hide();
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
      m_Controls->m_DwiLabel->setEnabled(true);
      m_Controls->m_InputImageLabel->setEnabled(true);
      m_Controls->m_BrainMaskLabel->show();
      m_Controls->m_InputBrainMaskLabel->show();
      m_Controls->m_ParameterBox->show();
      m_Controls->m_LabelParameter_1->show();
      m_Controls->m_LabelParameter_1->setText("Search Radius:");
      m_Controls->m_LabelParameter_2->show();
      m_Controls->m_LabelParameter_2->setText("Comparision Radius:");
      m_Controls->m_SpinBoxParameter1->show();
      m_Controls->m_SpinBoxParameter1->setValue(1);
      m_Controls->m_SpinBoxParameter2->show();
      m_Controls->m_SpinBoxParameter2->setValue(1);

      break;
    }
    case 2:
    {
      m_SelectedFilter = NLMV;
      m_Controls->m_DwiLabel->setEnabled(true);
      m_Controls->m_InputImageLabel->setEnabled(true);
      m_Controls->m_BrainMaskLabel->show();
      m_Controls->m_InputBrainMaskLabel->show();
      m_Controls->m_ParameterBox->show();
      m_Controls->m_LabelParameter_1->show();
      m_Controls->m_LabelParameter_1->setText("Search Radius:");
      m_Controls->m_LabelParameter_2->show();
      m_Controls->m_LabelParameter_2->setText("Comparision Radius:");
      m_Controls->m_LabelParameter_3->show();
      m_Controls->m_LabelParameter_3->setText("Number of neighboring gradients:");
      m_Controls->m_SpinBoxParameter1->show();
      m_Controls->m_SpinBoxParameter1->setValue(1);
      m_Controls->m_SpinBoxParameter2->show();
      m_Controls->m_SpinBoxParameter2->setValue(1);
      m_Controls->m_SpinBoxParameter3->show();
      m_Controls->m_SpinBoxParameter3->setValue(1);

      break;
    }
    case 3:
    {
      m_SelectedFilter = GAUSS;
      m_Controls->m_DwiLabel->setEnabled(true);
      m_Controls->m_InputImageLabel->setEnabled(true);
      m_Controls->m_ParameterBox->show();
      m_Controls->m_LabelParameter_1->show();
      m_Controls->m_LabelParameter_1->setText("Variance:");
      m_Controls->m_SpinBoxParameter1->show();
      m_Controls->m_SpinBoxParameter1->setValue(2);
      m_Controls->m_LabelParameter_2->hide();
      m_Controls->m_SpinBoxParameter2->hide();
    }
  }

  if (m_ImageNode.IsNull())
  {
    if (m_SelectedFilter != NOFILTERSELECTED)
      m_Controls->m_InputImageLabel->setText("<font color='red'>mandatory</font>");
    else
      m_Controls->m_InputImageLabel->setText("mandatory");
  }

  if (m_ImageNode.IsNotNull())
  {
    m_Controls->m_ApplyButton->setEnabled(false);
    switch(filter)
    {
      case NOFILTERSELECTED:
      {
        break;
      }
      case NLMR:
      case NLMV:
      {
        if (m_BrainMaskNode.IsNotNull())
          m_Controls->m_ApplyButton->setEnabled(true);
        break;
      }
      case GAUSS:
      {
        m_Controls->m_ApplyButton->setEnabled(true);
        break;
      }
    }
  }
}

void QmitkDenoisingView::BeforeThread()
{
  m_ThreadIsRunning = true;
  // initialize timer to update the progressbar at each timestep
  m_DenoisingTimer->start(500);
  m_Controls->m_LabelParameter_1->setEnabled(false);
  m_Controls->m_LabelParameter_2->setEnabled(false);
  m_Controls->m_LabelParameter_3->setEnabled(false);
  m_Controls->m_SpinBoxParameter1->setEnabled(false);
  m_Controls->m_SpinBoxParameter2->setEnabled(false);
  m_Controls->m_SpinBoxParameter3->setEnabled(false);
  m_Controls->m_SelectFilterComboBox->setEnabled(false);
  m_Controls->m_ApplyButton->setEnabled(false);
}

void QmitkDenoisingView::AfterThread()
{
  m_ThreadIsRunning = false;
  // stop timer to stop updates of progressbar
  m_DenoisingTimer->stop();
  // make sure progressbar is finished
  mitk::ProgressBar::GetInstance()->Progress(m_MaxProgressCount);
  switch (m_SelectedFilter)
  {
    case NOFILTERSELECTED:
    case GAUSS:
    {
      break;
    }

    case NLMR:
    {
      DiffusionImageType::Pointer image = DiffusionImageType::New();
      image->SetVectorImage(m_NonLocalMeansFilter->GetOutput());
      image->SetReferenceBValue(m_InputImage->GetReferenceBValue());
      image->SetDirections(m_InputImage->GetDirections());
      image->InitializeFromVectorImage();
      mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
      imageNode->SetData( image );
      QString name = m_ImageNode->GetName().c_str();

      imageNode->SetName((name+"_NLMr_"+QString::number(m_Controls->m_SpinBoxParameter1->value())+"-"+QString::number(m_Controls->m_SpinBoxParameter2->value())).toStdString().c_str());
      GetDefaultDataStorage()->Add(imageNode);
      break;
    }

    case NLMV:
    {
      DiffusionImageType::Pointer image = DiffusionImageType::New();
      image->SetVectorImage(m_NonLocalMeansFilter->GetOutput());
      image->SetReferenceBValue(m_InputImage->GetReferenceBValue());
      image->SetDirections(m_InputImage->GetDirections());
      image->InitializeFromVectorImage();
      mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
      imageNode->SetData( image );
      QString name = m_ImageNode->GetName().c_str();

      imageNode->SetName((name+"_NLMv_"+QString::number(m_Controls->m_SpinBoxParameter1->value())+"-"+QString::number(m_Controls->m_SpinBoxParameter2->value())+"-"+QString::number(m_Controls->m_SpinBoxParameter3->value())).toStdString().c_str());
      GetDefaultDataStorage()->Add(imageNode);

      m_Controls->m_LabelParameter_3->setEnabled(true);
      m_Controls->m_SpinBoxParameter3->setEnabled(true);
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
  switch (m_SelectedFilter)
  {
    case NOFILTERSELECTED:
    case GAUSS:
    {
      break;
    }
    case NLMR:
    case NLMV:
    {
      unsigned int currentProgressCount = m_NonLocalMeansFilter->GetCurrentVoxelCount();
      mitk::ProgressBar::GetInstance()->Progress(currentProgressCount-m_LastProgressCount);
      m_LastProgressCount = currentProgressCount;
      break;
    }
  }


}
