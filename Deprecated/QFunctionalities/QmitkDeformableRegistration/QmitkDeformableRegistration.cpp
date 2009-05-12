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

#include "QmitkDeformableRegistration.h"
#include "QmitkDeformableRegistrationControls.h"
#include "DeformableRegistration.xpm"
#include "QmitkTreeNodeSelector.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkCommonFunctionality.h"
#include "qinputdialog.h"
#include "qmessagebox.h"
#include "qcursor.h"
#include "qapplication.h"
#include "qradiobutton.h"
#include "qslider.h"

#include <vtkTransform.h>

QmitkDeformableRegistration::QmitkDeformableRegistration(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it) ,m_Controls(NULL), m_MultiWidget(mitkStdMultiWidget), m_ShowRedGreen(false),
  m_ShowBothImages(true), m_ShowFixedImage(false), m_ShowMovingImage(false), m_Opacity(0.5), m_OriginalOpacity(1.0), m_FixedNode(NULL), m_MovingNode(NULL),
  m_OldMovingLayerSet(false), m_NewMovingLayerSet(false), m_NewMovingLayer(0), m_OldMovingLayer(0)
{
  SetAvailability(true);
}

QmitkDeformableRegistration::~QmitkDeformableRegistration()
{
}

QWidget * QmitkDeformableRegistration::CreateMainWidget(QWidget *parent)
{
  return NULL;
}

QWidget * QmitkDeformableRegistration::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkDeformableRegistrationControls(parent);
  }
  return m_Controls;
}

static bool Filter( mitk::DataTreeNode * node )
{
  if(node==NULL) return false;
  mitk::BaseData* data = node->GetData();
  if(data==NULL) return false;
  if(dynamic_cast<mitk::Geometry2DData*>(data)!=NULL) return false;
  return true;
}

static bool FilterForMoving( mitk::DataTreeNode * node )
{
  if(node==NULL) return false;
  mitk::BaseData* data = node->GetData();
  if(data==NULL) return false;
  if(dynamic_cast<mitk::Geometry2DData*>(data)!=NULL) return false;
  bool selectedFixed = false;
  node->GetBoolProperty("selectedFixedImage", selectedFixed);
  if(selectedFixed) return false;
  return true;
}

void QmitkDeformableRegistration::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_FixedSelector), SIGNAL(Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(FixedSelected(mitk::DataTreeIteratorClone)) );
    connect( (QObject*)(m_Controls->m_MovingSelector), SIGNAL(Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(MovingSelected(mitk::DataTreeIteratorClone)) );
    connect(m_Controls,SIGNAL(showBothImages()),this,SLOT(ShowBothImages()));
    connect(m_Controls,SIGNAL(showFixedImage()),this,SLOT(ShowFixedImage()));
    connect(m_Controls,SIGNAL(showMovingImage()),this,SLOT(ShowMovingImage()));
    connect(m_Controls,SIGNAL(showImagesRedGreen(bool)),this,SLOT(ShowRedGreen(bool)));
    connect(m_Controls,SIGNAL(opacityChanged(float)),this,SLOT(OpacityUpdate(float)));
    
    
    connect(m_Controls, SIGNAL( reinitFixed(const mitk::Geometry3D*) ), m_MultiWidget, SLOT( InitializeStandardViews(const mitk::Geometry3D*) ));
    connect(m_Controls, SIGNAL( reinitMoving(const mitk::Geometry3D*) ), m_MultiWidget, SLOT( InitializeStandardViews(const mitk::Geometry3D*) ));
    connect(m_Controls, SIGNAL( globalReinit(mitk::DataTreeIteratorBase*) ), m_MultiWidget, SLOT( InitializeStandardViews(mitk::DataTreeIteratorBase*) ));

    m_Controls->m_FixedSelector->m_FilterFunction = Filter;
    m_Controls->m_MovingSelector->m_FilterFunction = FilterForMoving;
  }
}

QAction * QmitkDeformableRegistration::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "DeformableRegistration" ), QPixmap((const char**)DeformableRegistration_xpm), tr( "&DeformableRegistration" ), 0, parent, "DeformableRegistration" );
  return action;
}

void QmitkDeformableRegistration::TreeChanged()
{
  mitk::DataTreeIteratorClone iter = this->GetDataTreeIterator();
  iter->GoToBegin();
  while ( !iter->IsAtEnd() )
  {
    if ( (iter->Get().IsNotNull()) && (iter->Get()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(iter->Get()->GetData())==NULL )
    {
      iter->Get()->SetVisibility(false);
    }
    ++iter;
  }
  m_Controls->m_FixedSelector->SetDataTreeNodeIterator(this->GetDataTreeIterator());
  m_Controls->m_MovingSelector->SetDataTreeNodeIterator(this->GetDataTreeIterator());
  this->FixedSelected(iter);
  this->MovingSelected(iter);
}

void QmitkDeformableRegistration::Activated()
{
  mitk::DataTreeIteratorClone iter = this->GetDataTreeIterator();
  iter->GoToBegin();
  bool visible = false;
  m_InvisibleNodesList.clear();
  // set all nodes to invisible
  while ( !iter->IsAtEnd() )
  {
    if ( (iter->Get().IsNotNull()) && (iter->Get()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(iter->Get()->GetData())==NULL)
    {
      iter->Get()->GetBoolProperty("visible", visible);
      // safe all previous invisible nodes in a set
      if (visible == false)
      {
        m_InvisibleNodesList.insert(iter->Get());
      }
      else
      {
        iter->Get()->SetVisibility(false);
      }
    }
    ++iter;
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Activated();
  m_Controls->m_FixedSelector->SetDataTreeNodeIterator(this->GetDataTreeIterator());
  m_Controls->m_MovingSelector->SetDataTreeNodeIterator(this->GetDataTreeIterator());
  this->FixedSelected(iter);
  this->MovingSelected(iter);
  this->OpacityUpdate(m_Opacity);
}

void QmitkDeformableRegistration::Deactivated()
{
  // reset previous invisible nodes to invisible and previous visible nodes to visible
  mitk::DataTreeIteratorClone iter = this->GetDataTreeIterator();
  iter->GoToBegin();
  std::set<mitk::DataTreeNode*>::iterator setIter;
  while ( !iter->IsAtEnd() )
  {
    if ( (iter->Get().IsNotNull()) && (iter->Get()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(iter->Get()->GetData())==NULL)
    {
      setIter = m_InvisibleNodesList.find(iter->Get());
      if (setIter != m_InvisibleNodesList.end())
      {
        iter->Get()->SetVisibility(false);
      }
      else
      {
        iter->Get()->SetVisibility(true);
      }
    }
    ++iter;
  }
  m_InvisibleNodesList.clear();
  
  this->SetImageColor(false);
  if (m_MovingNode != NULL)
  {
    m_MovingNode->SetOpacity(m_OriginalOpacity);
    if (m_OldMovingLayerSet)
    {
      m_MovingNode->SetIntProperty("layer", m_OldMovingLayer);
      m_OldMovingLayerSet = false;
    }
  }
  m_FixedNode = NULL;
  m_MovingNode = NULL;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Deactivated();
}

void QmitkDeformableRegistration::FixedSelected(mitk::DataTreeIteratorClone imageIt)
{
  if (m_Controls->m_FixedSelector->GetSelectedNode() != NULL)
  {
    m_Controls->m_ShowFixedImage->setEnabled(true);
    mitk::DataTreeIteratorClone it;
    it = *m_Controls->m_FixedSelector->GetSelectedIterator();
    if (m_FixedNode != it->Get())
    {
      // remove changes on previous selected node
      if (m_FixedNode != NULL)
      {
        this->SetImageColor(false);
        m_FixedNode->SetVisibility(false);
        m_FixedNode->SetProperty("selectedFixedImage", mitk::BoolProperty::New(false));
      }
      // get selected node
      m_FixedNode = it->Get();
      mitk::ColorProperty::Pointer colorProperty;
      colorProperty = dynamic_cast<mitk::ColorProperty*>(m_FixedNode->GetProperty("color"));
      if ( colorProperty.IsNotNull() )
      {
        m_FixedColor = colorProperty->GetColor();
      }
      this->SetImageColor(m_ShowRedGreen);
      m_FixedNode->SetProperty("selectedFixedImage", mitk::BoolProperty::New(true));
      m_Controls->m_MovingSelector->UpdateContent();

      if (m_MovingNode != NULL)
      {
        // safe MovingLayer only if we do not overwrite original MovingLayer
        if (!m_OldMovingLayerSet)
        {
          m_MovingNode->GetIntProperty("layer", m_OldMovingLayer);
          m_OldMovingLayerSet = true;
        }
        m_FixedNode->GetIntProperty("layer", m_NewMovingLayer);
        m_NewMovingLayer += 1;
        m_MovingNode->SetIntProperty("layer", m_NewMovingLayer);
      }
      if (m_ShowBothImages)
      {
        this->ShowBothImages();
      }
      if (m_ShowFixedImage)
      {
        this->ShowFixedImage();
      }
      if (m_ShowMovingImage)
      {
        this->ShowMovingImage();
      }
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
    else
    {
      if (m_ShowFixedImage || m_ShowBothImages)
      {
        if (m_FixedNode != NULL)
        {
          m_FixedNode->SetVisibility(true); 
        }
      }
    }
  }
  else
  {
    if (m_ShowFixedImage)
    {
      m_Controls->m_ShowBothImages->setChecked(true);
      this->ShowBothImages();
    }
    m_Controls->m_ShowFixedImage->setEnabled(false);
  }
}

void QmitkDeformableRegistration::MovingSelected(mitk::DataTreeIteratorClone imageIt)
{
  if (m_Controls->m_MovingSelector->GetSelectedNode() != NULL)
  {
    m_Controls->m_ShowMovingImage->setEnabled(true);
    mitk::DataTreeIteratorClone it;
    it = *m_Controls->m_MovingSelector->GetSelectedIterator();
    if (m_MovingNode != it->Get())
    {
      if (m_MovingNode != NULL)
      {
        m_MovingNode->SetOpacity(m_OriginalOpacity);
        this->SetImageColor(false);
        if (m_MovingNode != m_FixedNode)
        {
          m_MovingNode->SetVisibility(false);
        }
        if (m_OldMovingLayerSet)
        {
          m_MovingNode->SetIntProperty("layer", m_OldMovingLayer);
          m_OldMovingLayerSet = false;
        }
      }
      m_MovingNode = it->Get();
      mitk::ColorProperty::Pointer colorProperty;
      colorProperty = dynamic_cast<mitk::ColorProperty*>(m_MovingNode->GetProperty("color"));
      if ( colorProperty.IsNotNull() )
      {
        m_MovingColor = colorProperty->GetColor();
      }
      this->SetImageColor(m_ShowRedGreen);
      m_MovingNode->GetFloatProperty("opacity", m_OriginalOpacity);
      this->OpacityUpdate(m_Opacity);
      // safe MovingLayer only if we do not overwrite original MovingLayer
      m_MovingNode->GetIntProperty("layer", m_OldMovingLayer);
      m_OldMovingLayerSet = true;
      // change MovingLayer to be one higher than FixedLayer -> MovingImage will be the upper image
      if (m_FixedNode != NULL)
      {
        m_FixedNode->GetIntProperty("layer", m_NewMovingLayer);
        m_NewMovingLayer += 1;
        m_MovingNode->SetIntProperty("layer", m_NewMovingLayer);
      }
      if (m_ShowBothImages)
      {
        this->ShowBothImages();
      }
      if (m_ShowFixedImage)
      {
        this->ShowFixedImage();
      }
      if (m_ShowMovingImage)
      {
        this->ShowMovingImage();
      }
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
      this->OpacityUpdate(m_Opacity);
    }
    else
    {
      if (m_ShowMovingImage || m_ShowBothImages)
      {
        if (m_MovingNode != NULL)
        {
          m_MovingNode->SetVisibility(true); 
        }
      }
    }
  }
  else
  {
    if (m_ShowMovingImage)
    {
      m_Controls->m_ShowBothImages->setChecked(true);
      this->ShowBothImages();
    }
    m_Controls->m_ShowMovingImage->setEnabled(false);
  }
}

bool QmitkDeformableRegistration::CheckCalculate()
{
  if(m_MovingNode==m_FixedNode)
    return false;
  return true;
}

void QmitkDeformableRegistration::ShowRedGreen(bool redGreen)
{
  m_ShowRedGreen = redGreen;
  this->SetImageColor(m_ShowRedGreen);
}

void QmitkDeformableRegistration::ShowFixedImage()
{
  m_ShowBothImages = false;
  m_ShowFixedImage = true;
  m_ShowMovingImage = false;
  if(m_FixedNode != NULL)
  {
    m_FixedNode->SetVisibility(true);
  }
  if(m_MovingNode != NULL)
  {
    m_MovingNode->SetVisibility(false);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDeformableRegistration::ShowMovingImage()
{
  m_ShowBothImages = false;
  m_ShowFixedImage = false;
  m_ShowMovingImage = true;
  if(m_FixedNode != NULL)
  {
    m_FixedNode->SetVisibility(false);
  }
  if(m_MovingNode != NULL)
  {
    m_MovingNode->SetVisibility(true);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDeformableRegistration::ShowBothImages()
{
  m_ShowBothImages = true;
  m_ShowFixedImage = false;
  m_ShowMovingImage = false;
  if(m_FixedNode != NULL)
  {
    m_FixedNode->SetVisibility(true);
  }
  if(m_MovingNode != NULL)
  {
    m_MovingNode->SetVisibility(true);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDeformableRegistration::SetImageColor(bool redGreen)
{
  if (!redGreen && m_FixedNode != NULL)
  {
    m_FixedNode->SetColor(m_FixedColor);
  }
  if (!redGreen && m_MovingNode != NULL)
  {
    m_MovingNode->SetColor(m_MovingColor);
  }
  if (redGreen && m_FixedNode != NULL)
  {
    m_FixedNode->SetColor(1.0f, 0.0f, 0.0f);
  }
  if (redGreen && m_MovingNode != NULL)
  {
    m_MovingNode->SetColor(0.0f, 1.0f, 0.0f);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDeformableRegistration::OpacityUpdate(float opacity)
{
  m_Opacity = opacity;
  if (m_MovingNode != NULL)
  {
    m_MovingNode->SetOpacity(m_Opacity);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
