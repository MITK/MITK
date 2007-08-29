/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
#include <qaction.h>
#include <qcheckbox.h>
#include "icon.xpm"
#include "QmitkDataTreeComboBox.h"
#include <mitkDataTreeNode.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>

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
      node->SetProperty("volumerendering",new mitk::BoolProperty(true));
      mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
      if (!image) return;
      image_ok = true;
      m_Controls->m_TransferFunctionWidget->SetDataTreeNode(node);
      m_Controls->m_TransferFunctionWidget_2->SetDataTreeNode(node);
    } 
    else if (!state && node) 
    {
      node->SetProperty("volumerendering",new mitk::BoolProperty(false));
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
  mitk::RenderingManager::GetInstance()->RequestUpdateAll3D();
}

void QmitkVolumeVisualization::SetShadingValues(float ambient, float diffuse, float specular, float specpower)
{
  mitk::RenderingManager::GetInstance()->SetShadingValues(ambient, diffuse, specular, specpower);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll3D();
}


