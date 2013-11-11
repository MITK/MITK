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

const std::string QmitkDenoisingView::VIEW_ID = "org.mitk.views.denoisingview";

QmitkDenoisingView::QmitkDenoisingView()
  : QmitkFunctionality()
  , m_Controls( 0 )
  , m_ImageNode(NULL)
{
}

QmitkDenoisingView::~QmitkDenoisingView()
{
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
        DiffusionImageType::Pointer inImage = dynamic_cast<DiffusionImageType*> (m_ImageNode->GetData());
        MaskImageType::Pointer imageMask = MaskImageType::New();
        mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_BrainMaskNode->GetData()), imageMask);

        NonLocalMeansDenoisingFilterType::Pointer denoisingFilter = NonLocalMeansDenoisingFilterType::New();
        denoisingFilter->SetNumberOfThreads(12);
        denoisingFilter->SetInputImage(inImage->GetVectorImage());
        denoisingFilter->SetInputMask(imageMask);
        denoisingFilter->SetUseJointInformation(false);
        denoisingFilter->SetVRadius(m_Controls->m_SpinBoxParameter1->value());
        denoisingFilter->SetNRadius(m_Controls->m_SpinBoxParameter2->value());
        denoisingFilter->Update();

        DiffusionImageType::Pointer image = DiffusionImageType::New();
        image->SetVectorImage(denoisingFilter->GetOutput());
        image->SetB_Value(inImage->GetB_Value());
        image->SetDirections(inImage->GetDirections());
        image->InitializeFromVectorImage();
        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( image );
        QString name = m_ImageNode->GetName().c_str();

        imageNode->SetName((name+"_NLMr").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode);

        break;
      }
      case 2:
      {
        DiffusionImageType::Pointer inImage = dynamic_cast<DiffusionImageType*> (m_ImageNode->GetData());
        MaskImageType::Pointer imageMask = MaskImageType::New();
        mitk::CastToItkImage(dynamic_cast<mitk::Image*>(m_BrainMaskNode->GetData()), imageMask);

        NonLocalMeansDenoisingFilterType::Pointer denoisingFilter = NonLocalMeansDenoisingFilterType::New();
        denoisingFilter->SetNumberOfThreads(12);
        denoisingFilter->SetInputImage(inImage->GetVectorImage());
        denoisingFilter->SetInputMask(imageMask);
        denoisingFilter->SetUseJointInformation(true);
        denoisingFilter->SetVRadius(m_Controls->m_SpinBoxParameter1->value());
        denoisingFilter->SetNRadius(m_Controls->m_SpinBoxParameter2->value());
        denoisingFilter->Update();

        DiffusionImageType::Pointer image = DiffusionImageType::New();
        image->SetVectorImage(denoisingFilter->GetOutput());
        image->SetB_Value(inImage->GetB_Value());
        image->SetDirections(inImage->GetDirections());
        image->InitializeFromVectorImage();
        mitk::DataNode::Pointer imageNode = mitk::DataNode::New();
        imageNode->SetData( image );
        QString name = m_ImageNode->GetName().c_str();

        imageNode->SetName((name+"_NLMv").toStdString().c_str());
        GetDefaultDataStorage()->Add(imageNode);

        break;
      }
    }


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
