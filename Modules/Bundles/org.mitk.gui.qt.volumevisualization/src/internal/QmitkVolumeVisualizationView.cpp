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

#include <berryISelectionProvider.h>
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>
//#include <berryISelectionService.h>
#include <mitkDataNodeObject.h>

#include <mitkProperties.h>
#include <mitkNodePredicateDataType.h>

#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include "mitkHistogramGenerator.h"
#include "QmitkPiecewiseFunctionCanvas.h"
#include "QmitkColorTransferFunctionCanvas.h"

#include "mitkBaseRenderer.h"
#include "mitkGPUVolumeMapper3D.h"

#include <QToolTip>
#include <qxtspanslider.h>

QmitkVolumeVisualizationView::QmitkVolumeVisualizationView()
: QmitkFunctionality(), 
  m_Controls(NULL)
{
}

QmitkVolumeVisualizationView::~QmitkVolumeVisualizationView()
{
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->RemoveSelectionListener(m_SelectionListener);


}

void QmitkVolumeVisualizationView::CreateQtPartControl(QWidget* parent)
{
   
  if (!m_Controls)
  {
    m_Controls = new Ui::QmitkVolumeVisualizationViewControls;
    m_Controls->setupUi(parent);

    connect( m_Controls->m_EnableRenderingCB, SIGNAL( toggled(bool) ),this, SLOT( OnEnableRendering(bool) ));
    connect( m_Controls->m_EnableLOD, SIGNAL( toggled(bool) ),this, SLOT( OnEnableLOD(bool) ));
    connect( m_Controls->m_EnableGPU, SIGNAL( toggled(bool) ),this, SLOT( OnEnableGPU(bool) ));

    connect( m_Controls->m_TransferFunctionGeneratorWidget, SIGNAL( SignalUpdateCanvas( ) ),   m_Controls->m_TransferFunctionWidget, SLOT( OnUpdateCanvas( ) ) );
 
    m_Controls->m_EnableRenderingCB->setEnabled(false);
    m_Controls->m_EnableLOD->setEnabled(false);
    m_Controls->m_EnableGPU->setEnabled(false);
    m_Controls->m_TransferFunctionWidget->setEnabled(false);
    m_Controls->m_TransferFunctionGeneratorWidget->setEnabled(false);
    m_Controls->m_SelectedImageLabel->hide();
    m_Controls->m_ErrorImageLabel->hide();
    
    
    
  }
  
  m_SelectionListener = berry::ISelectionListener::Pointer(new berry::SelectionChangedAdapter<QmitkVolumeVisualizationView>
    (this, &QmitkVolumeVisualizationView::SelectionChanged));
  berry::ISelectionService* s = GetSite()->GetWorkbenchWindow()->GetSelectionService();
  if(s)
    s->AddSelectionListener(m_SelectionListener);
    
  UpdateFromCurrentDataManagerSelection();
}


void QmitkVolumeVisualizationView::SelectionChanged( berry::IWorkbenchPart::Pointer, berry::ISelection::ConstPointer selection )
{ 
  //if(!this->m_IsVisible)
    //return;

  mitk::DataNodeSelection::ConstPointer _DataNodeSelection 
    = selection.Cast<const mitk::DataNodeSelection>();

  bool weHadAnImageButItsNotThreeDeeOrFourDee = false;
   
  if(_DataNodeSelection.IsNotNull())
  {
    std::vector<mitk::DataNode*> selectedNodes;
    mitk::DataNodeObject* _DataNodeObject = 0;

    for(mitk::DataNodeSelection::iterator it = _DataNodeSelection->Begin();it != _DataNodeSelection->End(); ++it)
    {
      _DataNodeObject = dynamic_cast<mitk::DataNodeObject*>((*it).GetPointer());
      if(_DataNodeObject)
      {
        mitk::DataNode::Pointer node = _DataNodeObject->GetDataNode();
      
        if( node.IsNotNull() && dynamic_cast<mitk::Image*>(node->GetData()) )
        {
          if( dynamic_cast<mitk::Image*>(node->GetData())->GetDimension()>=3 )
            selectedNodes.push_back( node );
          else
            weHadAnImageButItsNotThreeDeeOrFourDee = true;
        } 
      }
    }

    m_SelectedNode = 0;

    mitk::DataNode::Pointer node;
    
    if(selectedNodes.size() > 0)
      node=selectedNodes.front();

    if( node.IsNotNull() )
    {
      m_Controls->m_NoSelectedImageLabel->hide();
      m_Controls->m_ErrorImageLabel->hide();
      m_Controls->m_SelectedImageLabel->show();
      
      std::string  infoText;
      
      if (node->GetName().empty())
        infoText = std::string("Selected Image: [currently selected image has no name]");
      else
        infoText = std::string("Selected Image: ") + node->GetName();
      
      m_Controls->m_SelectedImageLabel->setText( QString( infoText.c_str() ) );
      
      m_SelectedNode = node;
    }
    else
    {
      if(weHadAnImageButItsNotThreeDeeOrFourDee)
      {
        m_Controls->m_NoSelectedImageLabel->hide();
        m_Controls->m_ErrorImageLabel->show();
        std::string  infoText;
        infoText = std::string("only 3D or 4D images are supported");
        m_Controls->m_ErrorImageLabel->setText( QString( infoText.c_str() ) ); 
      }
      else
      {
        m_Controls->m_SelectedImageLabel->hide();
        m_Controls->m_ErrorImageLabel->hide();
        m_Controls->m_NoSelectedImageLabel->show();
      }
    }

    UpdateInterface();
  }

}

void QmitkVolumeVisualizationView::UpdateFromCurrentDataManagerSelection()
{
  //MITK_INFO << "Update selection from DataManager";
  berry::ISelection::ConstPointer selection( this->GetSite()->GetWorkbenchWindow()->GetSelectionService()->GetSelection("org.mitk.views.datamanager"));
  this->SelectionChanged(berry::SmartPointer<IWorkbenchPart>(NULL), selection);
}

void QmitkVolumeVisualizationView::UpdateInterface()
{
  if(m_SelectedNode.IsNull())
  {
    // turnoff all
    m_Controls->m_EnableRenderingCB->setChecked(false);
    m_Controls->m_EnableRenderingCB->setEnabled(false);
    m_Controls->m_EnableLOD->setChecked(false);
    m_Controls->m_EnableLOD->setEnabled(false);
    m_Controls->m_EnableGPU->setEnabled(false);
    m_Controls->m_TransferFunctionWidget->SetDataNode(0);
    m_Controls->m_TransferFunctionGeneratorWidget->SetDataNode(0);
    m_Controls->m_TransferFunctionWidget->setEnabled(false);
    m_Controls->m_TransferFunctionGeneratorWidget->setEnabled(false);
    return;
  }
  
  bool enabled = false;
      
  m_SelectedNode->GetBoolProperty("volumerendering",enabled);
  m_Controls->m_EnableRenderingCB->setEnabled(true);
  m_Controls->m_EnableRenderingCB->setChecked(enabled);

  if(!enabled)
  {
    // turnoff all except volumerendering checkbox
    m_Controls->m_EnableLOD->setChecked(false);
    m_Controls->m_EnableLOD->setEnabled(false);
    m_Controls->m_EnableGPU->setEnabled(false);
    m_Controls->m_TransferFunctionWidget->SetDataNode(0);
    m_Controls->m_TransferFunctionGeneratorWidget->SetDataNode(0);
    m_Controls->m_TransferFunctionWidget->setEnabled(false);
    m_Controls->m_TransferFunctionGeneratorWidget->setEnabled(false);
    return;
  }

  // otherwise we can activate em all
  enabled = false;
  m_SelectedNode->GetBoolProperty("volumerendering.uselod",enabled);
  m_Controls->m_EnableLOD->setEnabled(true);
  m_Controls->m_EnableLOD->setChecked(enabled);
    
  enabled=false;
  m_SelectedNode->GetBoolProperty("volumerendering.usegpu",enabled);
  m_Controls->m_EnableGPU->setEnabled(true);
  m_Controls->m_EnableGPU->setChecked(enabled);

  m_Controls->m_TransferFunctionWidget->SetDataNode(m_SelectedNode);
  m_Controls->m_TransferFunctionWidget->setEnabled(true);
  m_Controls->m_TransferFunctionGeneratorWidget->SetDataNode(m_SelectedNode);
  m_Controls->m_TransferFunctionGeneratorWidget->setEnabled(true);
}


void QmitkVolumeVisualizationView::OnEnableRendering(bool state) 
{
  if(m_SelectedNode.IsNull())
    return;

  m_SelectedNode->SetProperty("volumerendering",mitk::BoolProperty::New(state));
  UpdateInterface();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkVolumeVisualizationView::OnEnableLOD(bool state) 
{
  if(m_SelectedNode.IsNull())
    return;

  m_SelectedNode->SetProperty("volumerendering.uselod",mitk::BoolProperty::New(state));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkVolumeVisualizationView::OnEnableGPU(bool state) 
{
  if(m_SelectedNode.IsNull())
    return;

  m_SelectedNode->SetProperty("volumerendering.usegpu",mitk::BoolProperty::New(state));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

 
}

void QmitkVolumeVisualizationView::SetFocus()
{

}

