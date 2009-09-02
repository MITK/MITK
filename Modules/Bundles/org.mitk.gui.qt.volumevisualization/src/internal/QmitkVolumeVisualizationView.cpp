/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15646 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkVolumeVisualizationView.h"

#include <QComboBox>

#include <mitkProperties.h>
#include <mitkNodePredicateDataType.h>

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include "mitkHistogramGenerator.h"
#include "QmitkPiecewiseFunctionCanvas.h"
#include "QmitkColorTransferFunctionCanvas.h"


QmitkVolumeVisualizationView::QmitkVolumeVisualizationView()
: QmitkFunctionality(), 
  m_Controls(NULL)
{

}


QmitkVolumeVisualizationView::~QmitkVolumeVisualizationView()
{

}

void QmitkVolumeVisualizationView::CreateQtPartControl(QWidget* parent)
{

  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkVolumeVisualizationViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    // define data type for combobox
    m_Controls->m_ImageSelector->SetDataStorage( this->GetDefaultDataStorage() );
    m_Controls->m_ImageSelector->SetPredicate( mitk::NodePredicateDataType::New("Image") );

    //Default values
    this->OnSetDefaultShadingValuesClicked();
    m_Controls->LODGroupBox->setEnabled(false);
    m_Controls->ShadingGroupBox->setEnabled(false);
    m_Controls->InteractionGroupBox->setEnabled(false);
    
  }
}


void QmitkVolumeVisualizationView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataTreeNode*)), this, SLOT(OnImageSelected(const mitk::DataTreeNode*)) );
    
    connect( m_Controls->m_EnableRenderingCB, SIGNAL( toggled(bool) ),this, SLOT( OnEnableRendering(bool) ));


    //Tab TF Style
    connect( m_Controls->m_TransferFunctionWidget, SIGNAL( SignalTransferFunctionModeChanged( int ) ), this, SLOT( OnTransferFunctionModeChanged( int ) ) );


    //Tab Preference
    connect( m_Controls->m_ClippingPlaneCB, SIGNAL( toggled( bool ) ), this, SLOT( OnEnableClippingPlane( bool ) ) );
    connect( m_Controls->m_EnableShading0, SIGNAL( toggled( bool ) ), this, SLOT( OnEnableShadingLow( bool ) ) );
    connect( m_Controls->m_EnableShading1, SIGNAL( toggled( bool ) ), this, SLOT( OnEnableShadingMed( bool ) ) );
    connect( m_Controls->m_EnableShading2, SIGNAL( toggled( bool ) ), this, SLOT( OnEnableShadingHigh( bool ) ) );
    connect( m_Controls->m_SetValuesButton, SIGNAL( clicked() ), this, SLOT( OnSetShadingValuesClicked() ) );
    connect( m_Controls->m_DefaultValuesButton, SIGNAL( clicked() ), this, SLOT( OnSetDefaultShadingValuesClicked() ) );

  }
}


void QmitkVolumeVisualizationView::OnImageSelected(const mitk::DataTreeNode* item)
{

  if( item == NULL  || item->GetData() == NULL )
    return;

  mitk::DataTreeNode::Pointer node = const_cast<mitk::DataTreeNode*>(item);

  if( node )
  {
    bool enabled = false;

    node->GetBoolProperty("volumerendering",enabled);
    m_Controls->m_EnableRenderingCB->setChecked(enabled);

    std::cout << "Selected mitk::Image at address " << node->GetData() << std::endl;
  }
  else
  {
    mitk::HistogramGenerator::Pointer histGen= mitk::HistogramGenerator::New();
    vtkPiecewiseFunction* piecewiseFunction = vtkPiecewiseFunction::New();
    vtkColorTransferFunction* colorTransferFunction = vtkColorTransferFunction::New();

    m_Controls->m_TransferFunctionWidget->m_ScalarOpacityFunctionCanvas->SetPiecewiseFunction( piecewiseFunction );
    m_Controls->m_TransferFunctionWidget->m_ScalarOpacityFunctionCanvas->SetHistogram( histGen->GetHistogram() );

    m_Controls->m_TransferFunctionWidget->m_GradientOpacityCanvas->SetPiecewiseFunction( piecewiseFunction );
    m_Controls->m_TransferFunctionWidget->m_GradientOpacityCanvas->SetHistogram( histGen->GetHistogram() );

    m_Controls->m_TransferFunctionWidget->m_ColorTransferFunctionCanvas->SetColorTransferFunction( colorTransferFunction );
    
    m_Controls->m_EnableRenderingCB->setChecked(false);
  }
}


void QmitkVolumeVisualizationView::OnEnableRendering(bool state) 
{
  m_Controls->ShadingGroupBox->setEnabled( state );
  m_Controls->InteractionGroupBox->setEnabled( state );
  
  image_ok = false;

  mitk::DataTreeNode::Pointer node = m_Controls->m_ImageSelector->GetSelectedNode();
  
  if (state && node) 
  {
    node->SetProperty("volumerendering",mitk::BoolProperty::New(true));
    mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
    
    if (!image) 
      return;
    
    image_ok = true;
    
    m_Controls->m_TransferFunctionWidget->SetDataTreeNode(node);
  } 
  
  else if (!state && node) 
    node->SetProperty("volumerendering",mitk::BoolProperty::New(false));
}


/****************/
// TAB TF STLYE //
/****************/

void QmitkVolumeVisualizationView::OnTransferFunctionModeChanged( int mode )
{
  //set Transfer Function to Data Node
  mitk::DataTreeNode::Pointer node = m_Controls->m_ImageSelector->GetSelectedNode();
 
  if( node )
  {
      //Create new Transfer Function
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();

      //-- Create Histogramm (for min / max Calculation)
      if( mode == 8 ) 
      {
        if( mitk::Image* image = dynamic_cast<mitk::Image*>( node->GetData() ) )
          tf->InitializeHistogram( image );
      }

      // -- Creat new TransferFunction
      tf->SetTransferFunctionMode( mode );

      node->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(tf.GetPointer()));
      m_Controls->m_TransferFunctionWidget->SetDataTreeNode(node);
  }
}

/*******************/
// TAB Preferences //
/*******************/
void QmitkVolumeVisualizationView::OnEnableClippingPlane( bool state )
{
  mitk::RenderingManager::GetInstance()->SetClippingPlaneStatus(state);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll( mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkVolumeVisualizationView:: OnEnableShadingLow( bool state )
{
  mitk::RenderingManager::GetInstance()->SetShading(state, 0);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkVolumeVisualizationView::OnEnableShadingMed( bool state )
{
  mitk::RenderingManager::GetInstance()->SetShading(state, 1);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkVolumeVisualizationView::OnEnableShadingHigh( bool state )
{
  mitk::RenderingManager::GetInstance()->SetShading(state, 2);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkVolumeVisualizationView::OnSetShadingValuesClicked()
{

  mitk::RenderingManager::GetInstance()->SetShadingValues( 
                                              m_Controls->m_AmbientEdit->text().toFloat(), 
                                              m_Controls->m_DiffuseEdit->text().toFloat(), 
                                              m_Controls->m_SpecularEdit->text().toFloat(), 
                                              m_Controls->m_SpecPowerEdit->text().toFloat());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll( mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}


void QmitkVolumeVisualizationView::OnSetDefaultShadingValuesClicked()
{
  m_Controls->m_AmbientEdit->setText(QString::number(0.1));
  m_Controls->m_DiffuseEdit->setText(QString::number(0.7));
  m_Controls->m_SpecularEdit->setText(QString::number(0.2));
  m_Controls->m_SpecPowerEdit->setText(QString::number(10));
}


void QmitkVolumeVisualizationView::SetFocus()
{

}

