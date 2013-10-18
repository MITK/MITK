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



const std::string QmitkDenoisingView::VIEW_ID = "org.mitk.views.denoisingview";

QmitkDenoisingView::QmitkDenoisingView()
  : QmitkFunctionality()
  , m_Controls( 0 )
  , m_MultiWidget( NULL )
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
  }
}

void QmitkDenoisingView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  //m_MultiWidget = &stdMultiWidget;

  /*{
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
    itk::ReceptorMemberCommand<QmitkDenoisingView>::Pointer command = itk::ReceptorMemberCommand<QmitkDenoisingView>::New();
    command->SetCallbackFunction( this, &QmitkDenoisingView::OnSliceChanged );
    m_SliceObserverTag1 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
  }

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
    itk::ReceptorMemberCommand<QmitkDenoisingView>::Pointer command = itk::ReceptorMemberCommand<QmitkDenoisingView>::New();
    command->SetCallbackFunction( this, &QmitkDenoisingView::OnSliceChanged );
    m_SliceObserverTag2 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
  }

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
    itk::ReceptorMemberCommand<QmitkDenoisingView>::Pointer command = itk::ReceptorMemberCommand<QmitkDenoisingView>::New();
    command->SetCallbackFunction( this, &QmitkDenoisingView::OnSliceChanged );
    m_SliceObserverTag3 = slicer->AddObserver( mitk::SliceNavigationController::GeometrySliceEvent(NULL, 0), command );
  }*/
}


void QmitkDenoisingView::StdMultiWidgetNotAvailable()
{
  /*{
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget1->GetSliceNavigationController();
    slicer->RemoveObserver( m_SliceObserverTag1 );
  }

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget2->GetSliceNavigationController();
    slicer->RemoveObserver( m_SliceObserverTag2 );
  }

  {
    mitk::SliceNavigationController* slicer = m_MultiWidget->mitkWidget3->GetSliceNavigationController();
    slicer->RemoveObserver( m_SliceObserverTag3 );
  }*/
  //m_MultiWidget = NULL;
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
    command->SetCallbackFunction( this, &QmitkDenoisingView::OnSliceChanged );
    m_PropertyObserverTag = m_ImageNode->AddObserver( itk::ModifiedEvent(), command );

    m_Controls->m_InputData->setTitle("Input Data");
  }
}

/**
*
* ist das Interessant für mich?
*
**/
void QmitkDenoisingView::OnSliceChanged(const itk::EventObject& /*e*/)
{
}
