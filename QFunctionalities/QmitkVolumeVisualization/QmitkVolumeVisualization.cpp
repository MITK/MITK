/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkVolumeVisualization.h"
#include "QmitkVolumeVisualizationControls.h"
#include "QmitkTransferFunctionWidget.h"
#include "QmitkPiecewiseFunctionCanvas.h"
#include "QmitkColorTransferFunctionCanvas.h"

#include <qaction.h>
#include <qcheckbox.h>
#include "icon.xpm"
#include "QmitkDataTreeComboBox.h"
#include <mitkDataTreeNode.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>
#include <mitkTransferFunction.h>
#include <mitkTransferFunctionProperty.h>
#include "mitkHistogramGenerator.h"

  QmitkVolumeVisualization::QmitkVolumeVisualization(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it) , m_MultiWidget(mitkStdMultiWidget) ,m_Controls(NULL)
{
  SetAvailability(true);
}

QmitkVolumeVisualization::~QmitkVolumeVisualization()
{}

QWidget * QmitkVolumeVisualization::CreateMainWidget(QWidget*)
{
  return NULL;
}

QWidget * QmitkVolumeVisualization::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkVolumeVisualizationControls(parent);

    // !!! Fill Transfer Function List with issues
    m_Controls->m_TransferFunctionTable->insertItem( " choose Transferfunction ..");
    m_Controls->m_TransferFunctionTable->insertItem( "Ramp between 25% & 50%, Tan");
    m_Controls->m_TransferFunctionTable->insertItem( "Ramp between 25% & 75%, Tan");
    m_Controls->m_TransferFunctionTable->insertItem( "CT AAA");
    m_Controls->m_TransferFunctionTable->insertItem( "CT Bone");
    m_Controls->m_TransferFunctionTable->insertItem( "CT Cardiac");
    m_Controls->m_TransferFunctionTable->insertItem( "CT Coronary arteries");
    m_Controls->m_TransferFunctionTable->insertItem( "MR Default");
    m_Controls->m_TransferFunctionTable->insertItem( "MR MIP");
    m_Controls->m_TransferFunctionTable->insertItem( "MITK Default");
  }
  return m_Controls;
}

void QmitkVolumeVisualization::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_TreeNodeSelector), SIGNAL(activated(const mitk::DataTreeFilter::Item*)),(QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item*)) );
    m_Controls->m_TreeNodeSelector->SetDataTree(this->GetDataTreeIterator());
    connect( (QObject*)(m_Controls), SIGNAL(EnableRenderingToggled(bool)),(QObject*) this, SLOT(EnableRendering(bool)));

    //TF-Auswahl
    connect( (QObject*)(m_Controls), SIGNAL(Choice(int)),(QObject*) this, SLOT(GetChoice(int)));

    //Color TF
    connect( (QObject*)(m_Controls), SIGNAL(StyleChoice(int)),(QObject*) this, SLOT(GetCStyle(int)));

    //Preset-TF
    connect( (QObject*)(m_Controls), SIGNAL(PresetTF(int)),(QObject*) this, SLOT(GetPreset(int)));
    
    //***Preferences***
    //Shading
    connect( (QObject*)(m_Controls), SIGNAL(EnableShadingToggled(bool, int)),(QObject*) this, SLOT(SetShading(bool, int)));
    //Clipping plane
    connect( (QObject*)(m_Controls), SIGNAL(EnableCPToggled(bool)),(QObject*) this, SLOT(EnableClippingPlane(bool)));
    //ShadingOptions
    connect( (QObject*)(m_Controls), SIGNAL(ShadingValues(float, float, float, float)),(QObject*) this, SLOT(SetShadingValues(float, float, float, float)));
    
    //Immediate Update
    connect( (QObject*)(m_Controls), SIGNAL(ImmUpdate(bool)),(QObject*) this, SLOT(ImmediateUpdate(bool)));

    // !!! Transfer Function Changed
    connect( m_Controls->m_TransferFunctionTable, SIGNAL( activated( int ) ), this, SLOT( OnChangeTransferFunctionMode( int ) ) );
  }
}

QAction * QmitkVolumeVisualization::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "VolumeVisualization" ), QPixmap((const char**)icon_xpm), tr( "VolumeVisualization" ), 0, parent, "VolumeVisualization" );
  return action;
}

void QmitkVolumeVisualization::TreeChanged()
{
  m_Controls->m_TreeNodeSelector->Update();

  const mitk::DataTreeFilter::Item* item = m_Controls->m_TreeNodeSelector->GetFilter()->GetSelectedItem();
  if (item)
  {
    mitk::DataTreeNode* node = const_cast<mitk::DataTreeNode*>( item->GetNode() );
    bool enabled = false;

    if( node )
      node->GetBoolProperty("volumerendering",enabled);
    
    m_Controls->m_EnableRenderingCB->setChecked(enabled);
  }
  else
  {
    mitk::HistogramGenerator::Pointer histGen= mitk::HistogramGenerator::New();
    vtkPiecewiseFunction* piecewiseFunction = vtkPiecewiseFunction::New();
    vtkColorTransferFunction* colorTransferFunction = vtkColorTransferFunction::New();
    
    m_Controls->m_TransferFunctionWidget->m_ScalarOpacityFunctionCanvas->SetPiecewiseFunction( piecewiseFunction );
    m_Controls->m_TransferFunctionWidget->m_ScalarOpacityFunctionCanvas->SetHistogram( histGen->GetHistogram() );
    
    m_Controls->m_TransferFunctionWidget_2->m_ScalarOpacityFunctionCanvas->SetPiecewiseFunction( piecewiseFunction );
    m_Controls->m_TransferFunctionWidget_2->m_ScalarOpacityFunctionCanvas->SetHistogram( histGen->GetHistogram() );

    m_Controls->m_TransferFunctionWidget->m_GradientOpacityCanvas->SetPiecewiseFunction( piecewiseFunction );
    m_Controls->m_TransferFunctionWidget->m_GradientOpacityCanvas->SetHistogram( histGen->GetHistogram() );
    
    m_Controls->m_TransferFunctionWidget_2->m_GradientOpacityCanvas->SetPiecewiseFunction( piecewiseFunction );
    m_Controls->m_TransferFunctionWidget_2->m_GradientOpacityCanvas->SetHistogram( histGen->GetHistogram() );
    
    m_Controls->m_TransferFunctionWidget->m_ColorTransferFunctionCanvas->SetColorTransferFunction( colorTransferFunction );
    m_Controls->m_TransferFunctionWidget_2->m_ColorTransferFunctionCanvas->SetColorTransferFunction( colorTransferFunction );

    m_Controls->m_EnableRenderingCB->setChecked(false);
  }

}

void QmitkVolumeVisualization::Activated()
{
  QmitkFunctionality::Activated();
}
void QmitkVolumeVisualization::ImageSelected(const mitk::DataTreeFilter::Item* item)
{
  mitk::DataTreeNode* node = const_cast<mitk::DataTreeNode*>(item->GetNode());
  bool enabled = false; 
  if (node) 
  {
    node->GetBoolProperty("volumerendering",enabled);
  }
  m_Controls->m_EnableRenderingCB->setChecked(enabled);
}
void QmitkVolumeVisualization::EnableRendering(bool state) 
{
  std::cout << "EnableRendering:" << state << std::endl;
  image_ok = false;
  const mitk::DataTreeFilter::Item* item = m_Controls->m_TreeNodeSelector->GetFilter()->GetSelectedItem();
  if (item)
  {
    mitk::DataTreeNode* node = const_cast<mitk::DataTreeNode*>(item->GetNode());
    if (state && node) {
      node->SetProperty("volumerendering",mitk::BoolProperty::New(true));
      mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
      if (!image) return;
      image_ok = true;
      m_Controls->m_TransferFunctionWidget->SetDataTreeNode(node);
      m_Controls->m_TransferFunctionWidget_2->SetDataTreeNode(node);
    } 
    else if (!state && node) 
    {
      node->SetProperty("volumerendering",mitk::BoolProperty::New(false));
    }
  }
}

void QmitkVolumeVisualization::GetChoice(int number)
{
  if(image_ok)
  {
    m_Controls->m_TransferFunctionWidget->ChooseTF(number);
    m_Controls->m_TransferFunctionWidget_2->ChooseTF(number);
  }
  else
  {
    std::cout<<"No image selected!\n";
  }
}

void QmitkVolumeVisualization::GetCStyle(int number)
{
  if(image_ok)
  {
    m_Controls->m_TransferFunctionWidget->ChooseCS(number);
    m_Controls->m_TransferFunctionWidget_2->ChooseCS(number);
  }
  else
  {
    std::cout<<"No image selected!\n";
  }

}
void QmitkVolumeVisualization::GetPreset(int number)
{
  if(image_ok)
  {
    m_Controls->m_TransferFunctionWidget->ChooseTF(number);
    m_Controls->m_TransferFunctionWidget_2->ChooseTF(number);
  }
  else
  {
    std::cout<<"No image selected!\n";
  }

}

void QmitkVolumeVisualization::SetShading(bool state, int lod)
{
 mitk::RenderingManager::GetInstance()->SetShading(state, lod);
 mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkVolumeVisualization::ImmediateUpdate(bool state)
{
  m_Controls->m_TransferFunctionWidget->ImmediateUpdate(state);
}

void QmitkVolumeVisualization::EnableClippingPlane(bool state)
{
  mitk::RenderingManager::GetInstance()->SetClippingPlaneStatus(state);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll( 
  mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

void QmitkVolumeVisualization::SetShadingValues(float ambient, float diffuse, float specular, float specpower)
{
  mitk::RenderingManager::GetInstance()->SetShadingValues(ambient, diffuse, specular, specpower);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll( 
  mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
}

/* !!! ***********/
void QmitkVolumeVisualization::OnChangeTransferFunctionMode( int mode )
{
  //first item is nothing
  if( mode == 0 )
    return;
  else //subract 1 for correct TransferFunction order ( mode = 0 = TF_SKIN_50 )
    mode -= 1;

  //set Transfer Function to Data Node
  const mitk::DataTreeFilter::Item* item = m_Controls->m_TreeNodeSelector->GetFilter()->GetSelectedItem();
  if (item)
  {
    mitk::DataTreeNode* node = const_cast<mitk::DataTreeNode*>( item->GetNode() );
    if( node )
    {
      //Create new Transfer Function
      mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
      
      //-- Create Histogramm (for min / max Calculation)
      if( mode == 8 ) //TF_MITK_DEFAULT           
      {
        if( mitk::Image* image = dynamic_cast<mitk::Image*>( node->GetData() ) )
          tf->InitializeHistogram( image );
      }

      // -- Creat new TransferFunction
      tf->SetTransferFunctionMode( mode );

      node->SetProperty("TransferFunction", mitk::TransferFunctionProperty::New(tf.GetPointer()));
      m_Controls->m_TransferFunctionWidget->SetDataTreeNode(node);
      m_Controls->m_TransferFunctionWidget_2->SetDataTreeNode(node);
    }
  }
}
