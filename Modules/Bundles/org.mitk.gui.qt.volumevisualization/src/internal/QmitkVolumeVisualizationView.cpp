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

#include "mitkBaseRenderer.h"
#include "mitkGPUVolumeMapper3D.h"

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

    m_Controls->m_ImageSelector->SetDataStorage( this->GetDefaultDataStorage() );

    m_Controls->m_ImageSelector->SetPredicate( mitk::NodePredicateDataType::New("Image") );

    connect( m_Controls->m_ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataTreeNode*)), this, SLOT(OnImageSelected(const mitk::DataTreeNode*)) );
    
    connect( m_Controls->m_EnableRenderingCB, SIGNAL( toggled(bool) ),this, SLOT( OnEnableRendering(bool) ));

    connect( m_Controls->m_TransferFunctionGeneratorWidget, SIGNAL( SignalUpdateCanvas( ) ),   m_Controls->m_TransferFunctionWidget, SLOT( OnUpdateCanvas( ) ) );

  }
}

void QmitkVolumeVisualizationView::OnImageSelected(const mitk::DataTreeNode* item)
{
  mitk::DataTreeNode::Pointer node = const_cast<mitk::DataTreeNode*>(item);

  if( node )
  {
    bool enabled = false;
    node->GetBoolProperty("volumerendering",enabled);
    m_Controls->m_EnableRenderingCB->setChecked(enabled);

    m_Controls->m_TransferFunctionWidget->SetDataTreeNode(node);
    m_Controls->m_TransferFunctionGeneratorWidget->SetDataTreeNode(node);
  }
  else
  {
    m_Controls->m_EnableRenderingCB->setChecked(false);

    m_Controls->m_TransferFunctionWidget->SetDataTreeNode(0);
    m_Controls->m_TransferFunctionGeneratorWidget->SetDataTreeNode(0);
  }
}


void QmitkVolumeVisualizationView::OnEnableRendering(bool state) 
{
  mitk::DataTreeNode::Pointer node = m_Controls->m_ImageSelector->GetSelectedNode();

  if(node.IsNull())
    return;

  node->SetProperty("volumerendering",mitk::BoolProperty::New(state));

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkVolumeVisualizationView::SetFocus()
{

}

