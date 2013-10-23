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
#include <QmitkStdMultiWidget.h>
#include <QTableWidgetItem>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkLookupTable.h>
#include <mitkOdfNormalizationMethodProperty.h>
#include <QTextEdit>
#include <mitkTensorImage.h>
#include <QMessageBox>
#include <QmitkRenderingManager.h>

#include <mitkImageReadAccessor.h>
#include <itkNonLocalMeansDenoisingFilter.h>



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
    this->CreateConnections();
  }
}

void QmitkDenoisingView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ApplyButton), SIGNAL(clicked()), this, SLOT(StartDenoising()));
  }
}

void QmitkDenoisingView::OnSelectionChanged( std::vector<mitk::DataNode*> nodes )
{
  if (m_ImageNode.IsNotNull())
    m_ImageNode->RemoveObserver( m_PropertyObserverTag );

  m_Controls->m_InputData->setTitle("Please Select Input Data");
  m_Controls->m_InputImageLabel->setText("<font color='red'>mandatory</font>");

  m_ImageNode = NULL;

  // iterate selection
  for( std::vector<mitk::DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it )
  {
    mitk::DataNode::Pointer node = *it;

    if( node.IsNotNull() && dynamic_cast<DiffusionImageType*>(node->GetData()))
    {
      m_Controls->m_InputImageLabel->setText(node->GetName().c_str());
      m_ImageNode = node;
    }
  }

  if (m_ImageNode.IsNotNull())
  {
    itk::ReceptorMemberCommand<QmitkDenoisingView>::Pointer command = itk::ReceptorMemberCommand<QmitkDenoisingView>::New();
    m_PropertyObserverTag = m_ImageNode->AddObserver( itk::ModifiedEvent(), command );

    m_Controls->m_InputData->setTitle("Input Data");
  }
}

void QmitkDenoisingView::StartDenoising()
{
  if (m_ImageNode.IsNotNull())
  {
    MITK_INFO << "JOPP";
    DiffusionImageType::Pointer inImage = dynamic_cast<DiffusionImageType*> (m_ImageNode->GetData());
    typedef itk::NonLocalMeansDenoisingFilter<DiffusionPixelType, DiffusionPixelType> NonLocalMeansDenoisingFilterType;
    NonLocalMeansDenoisingFilterType::Pointer denoisingFilter = NonLocalMeansDenoisingFilterType::New();
    denoisingFilter->SetNumberOfThreads(1);
    denoisingFilter->SetInput(inImage->GetVectorImage());
    denoisingFilter->SetVRadius((unsigned int)m_Controls->m_Parameter1->value());
    denoisingFilter->SetNRadius((unsigned int)m_Controls->m_Parameter2->value());
    denoisingFilter->SetH((unsigned int)m_Controls->m_Parameter3->value());
    denoisingFilter->Update();
  }
}
