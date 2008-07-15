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

#include "QmitkRigidRegistration.h"
#include "QmitkRigidRegistrationControls.h"
#include "RigidRegistration.xpm"
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

QmitkRigidRegistration::QmitkRigidRegistration(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it) ,m_Controls(NULL), m_MultiWidget(mitkStdMultiWidget), m_ShowRedGreen(false),
  m_ShowBothImages(true), m_ShowFixedImage(false), m_ShowMovingImage(false), m_Opacity(0.5), m_OriginalOpacity(1.0), m_FixedNode(NULL), m_MovingNode(NULL),
  m_OldMovingLayerSet(false), m_NewMovingLayerSet(false), m_NewMovingLayer(0), m_OldMovingLayer(0)
{
  SetAvailability(true);
}

QmitkRigidRegistration::~QmitkRigidRegistration()
{
}

QWidget * QmitkRigidRegistration::CreateMainWidget(QWidget *parent)
{
  return NULL;
}

QWidget * QmitkRigidRegistration::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkRigidRegistrationControls(parent);
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

void QmitkRigidRegistration::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_FixedSelector), SIGNAL(Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(FixedSelected(mitk::DataTreeIteratorClone)) );
    connect( (QObject*)(m_Controls->m_MovingSelector), SIGNAL(Activated(mitk::DataTreeIteratorClone)),(QObject*) this, SLOT(MovingSelected(mitk::DataTreeIteratorClone)) );
    connect(m_Controls,SIGNAL(saveModel()),this,SLOT(SaveModel()));
    connect(m_Controls,SIGNAL(undoTransformation()),this,SLOT(UndoTransformation()));
    connect(m_Controls,SIGNAL(redoTransformation()),this,SLOT(RedoTransformation()));
    connect(m_Controls,SIGNAL(showBothImages()),this,SLOT(ShowBothImages()));
    connect(m_Controls,SIGNAL(showFixedImage()),this,SLOT(ShowFixedImage()));
    connect(m_Controls,SIGNAL(showMovingImage()),this,SLOT(ShowMovingImage()));
    connect(m_Controls,SIGNAL(showImagesRedGreen(bool)),this,SLOT(ShowRedGreen(bool)));
    connect(m_Controls,SIGNAL(opacityChanged(float)),this,SLOT(OpacityUpdate(float)));
    connect(m_Controls,SIGNAL(calculateTransformation()),this,SLOT(CalculateTransformation()));
    connect(m_Controls,SIGNAL(translateMovingImage(int* )),this,SLOT(Translate(int* )));
    connect(m_Controls,SIGNAL(alignCenters()),this,SLOT(AlignCenters()));
    connect(m_Controls,SIGNAL(addNewTransformationToUndoList()),this,SLOT(AddNewTransformationToUndoList()));
    
    
    connect(m_Controls, SIGNAL( reinitFixed(const mitk::Geometry3D*) ), m_MultiWidget, SLOT( InitializeStandardViews(const mitk::Geometry3D*) ));
    connect(m_Controls, SIGNAL( reinitMoving(const mitk::Geometry3D*) ), m_MultiWidget, SLOT( InitializeStandardViews(const mitk::Geometry3D*) ));
    connect(m_Controls, SIGNAL( globalReinit(mitk::DataTreeIteratorBase*) ), m_MultiWidget, SLOT( InitializeStandardViews(mitk::DataTreeIteratorBase*) ));

    m_Controls->m_FixedSelector->m_FilterFunction = Filter;
    m_Controls->m_MovingSelector->m_FilterFunction = FilterForMoving;
  }
}

QAction * QmitkRigidRegistration::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "RigidRegistration" ), QPixmap((const char**)RigidRegistration_xpm), tr( "&RigidRegistration" ), 0, parent, "RigidRegistration" );
  return action;
}

void QmitkRigidRegistration::TreeChanged()
{
  mitk::DataTreeIteratorClone iter = this->GetDataTreeIterator();
  iter->GoToBegin();
  while ( !iter->IsAtEnd() )
  {
    if ( (iter->Get().IsNotNull()) && (iter->Get()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(iter->Get()->GetData())==NULL)
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

void QmitkRigidRegistration::Activated()
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
  m_Controls->FixedImageChanged();
  m_Controls->MovingImageChanged();
  this->ClearTransformationLists();
  this->FixedSelected(iter);
  this->MovingSelected(iter);
  this->OpacityUpdate(m_Opacity);
  m_Controls->CheckCalculateEnabled();
}

void QmitkRigidRegistration::Deactivated()
{
  // reset previous invisible nodes to invisible and previous visible nodes to visible
  mitk::DataTreeIteratorClone iter = this->GetDataTreeIterator();
  iter->GoToBegin();
  std::set<mitk::DataTreeNode*>::iterator setIter;
  while ( !iter->IsAtEnd() )
  {
    if ( (iter->Get().IsNotNull()) && (iter->Get()->GetProperty("visible")) )
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
  this->ClearTransformationLists();
  m_FixedNode = NULL;
  m_MovingNode = NULL;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Deactivated();
}

void QmitkRigidRegistration::FixedSelected(mitk::DataTreeIteratorClone imageIt)
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

void QmitkRigidRegistration::MovingSelected(mitk::DataTreeIteratorClone imageIt)
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
      this->ClearTransformationLists();
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

bool QmitkRigidRegistration::CheckCalculate()
{
  if(m_MovingNode==m_FixedNode)
    return false;
  return true;
}

void QmitkRigidRegistration::SaveModel()
{
  if(m_MovingNode != NULL)
  {
    mitk::BaseData::Pointer data=m_MovingNode->GetData();
    if (data.IsNotNull())
    {
      CommonFunctionality::SaveBaseData( data.GetPointer(), "RegistrationResult");
    }
  }
}

void QmitkRigidRegistration::AddNewTransformationToUndoList()
{
  mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
  m_UndoGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingData->GetGeometry(0)->Clone().GetPointer()));
  m_RedoGeometryList.clear();
  m_Controls->SetUndoEnabled(true);
  m_Controls->SetRedoEnabled(false);
}

void QmitkRigidRegistration::UndoTransformation()
{
  if(!m_UndoGeometryList.empty())
  {
    mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
    m_RedoGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingData->GetGeometry(0)->Clone().GetPointer()));
    movingData->SetGeometry(m_UndoGeometryList.back());
    m_UndoGeometryList.pop_back();
    //\FIXME when geometry is substituted the matrix referenced by the actor created by the mapper
    //is still pointing to the old one. Workaround: delete mapper
    m_MovingNode->SetMapper(1, NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_MultiWidget->mitkWidget4->GetRenderWindow());

    movingData->GetTimeSlicedGeometry()->UpdateInformation();
    m_Controls->SetRedoEnabled(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();    
  }
  if(!m_UndoGeometryList.empty())
  {
    m_Controls->SetUndoEnabled(true);
  }
  else
  {
    m_Controls->SetUndoEnabled(false);
  }
}

void QmitkRigidRegistration::RedoTransformation()
{
  if(!m_RedoGeometryList.empty())
  {
    mitk::BaseData::Pointer movingData = m_MovingNode->GetData();
    m_UndoGeometryList.push_back(static_cast<mitk::Geometry3D *>(movingData->GetGeometry(0)->Clone().GetPointer()));
    movingData->SetGeometry(m_RedoGeometryList.back());
    m_RedoGeometryList.pop_back();
    //\FIXME when geometry is substituted the matrix referenced by the actor created by the mapper
    //is still pointing to the old one. Workaround: delete mapper
    m_MovingNode->SetMapper(1, NULL);
    mitk::RenderingManager::GetInstance()->RequestUpdate(m_MultiWidget->mitkWidget4->GetRenderWindow());

    movingData->GetTimeSlicedGeometry()->UpdateInformation();
    m_Controls->SetUndoEnabled(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  if(!m_RedoGeometryList.empty())
  {
    m_Controls->SetRedoEnabled(true);
  }
  else
  {
    m_Controls->SetRedoEnabled(false);
  }
}

void QmitkRigidRegistration::ShowRedGreen(bool redGreen)
{
  m_ShowRedGreen = redGreen;
  this->SetImageColor(m_ShowRedGreen);
}

void QmitkRigidRegistration::ShowFixedImage()
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

void QmitkRigidRegistration::ShowMovingImage()
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

void QmitkRigidRegistration::ShowBothImages()
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

void QmitkRigidRegistration::SetImageColor(bool redGreen)
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

void QmitkRigidRegistration::OpacityUpdate(float opacity)
{
  m_Opacity = opacity;
  if (m_MovingNode != NULL)
  {
    m_MovingNode->SetOpacity(m_Opacity);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkRigidRegistration::ClearTransformationLists()
{
  m_Controls->SetUndoEnabled(false);
  m_Controls->SetRedoEnabled(false);
  m_UndoGeometryList.clear();
  m_RedoGeometryList.clear();
}

void QmitkRigidRegistration::Translate(int* translateVector)
{ 
  if (m_MovingNode != NULL)
  {
    float* oldPos = new float[3];

    oldPos[0] = m_MovingNode->GetVtkTransform()->GetMatrix()->GetElement(0,3);
    oldPos[1] = m_MovingNode->GetVtkTransform()->GetMatrix()->GetElement(1,3);
    oldPos[2] = m_MovingNode->GetVtkTransform()->GetMatrix()->GetElement(2,3);

    mitk::Vector3D translateVec; 
    translateVec[0] = translateVector[0] - oldPos[0];
    translateVec[1] = translateVector[1] - oldPos[1];
    translateVec[2] = translateVector[2] - oldPos[2];
    m_MovingNode->GetData()->GetGeometry()->Translate( translateVec );

    m_MovingNode->GetData()->Modified();
    m_RedoGeometryList.clear();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkRigidRegistration::AlignCenters()
{
  if (m_FixedNode != NULL && m_MovingNode != NULL)
  {
    mitk::Point3D fixedPoint = m_FixedNode->GetData()->GetGeometry()->GetCenter();
    mitk::Point3D movingPoint = m_MovingNode->GetData()->GetGeometry()->GetCenter();
    mitk::Vector3D translateVec;
    translateVec =  fixedPoint - movingPoint;
    m_Controls->m_XTransSlider->setValue((int)m_Controls->m_XTransSlider->value() + (int)translateVec[0]);
    m_Controls->m_YTransSlider->setValue((int)m_Controls->m_YTransSlider->value() + (int)translateVec[1]);
    m_Controls->m_ZTransSlider->setValue((int)m_Controls->m_ZTransSlider->value() + (int)translateVec[2]);
  }
}

void QmitkRigidRegistration::CalculateTransformation()
{
  if (m_FixedNode != NULL && m_MovingNode != NULL)
  {
  }
}
