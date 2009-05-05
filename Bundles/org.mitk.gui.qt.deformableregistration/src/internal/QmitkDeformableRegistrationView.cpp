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

#include "QmitkDeformableRegistrationView.h"
#include "ui_QmitkDeformableRegistrationViewControls.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkCommonFunctionality.h"
#include "qinputdialog.h"
#include "qmessagebox.h"
#include "qcursor.h"
#include "qapplication.h"
#include "qradiobutton.h"
#include "qslider.h"

#include <vtkTransform.h>

#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateNOT.h"

QmitkDeformableRegistrationView::QmitkDeformableRegistrationView(QObject *parent, const char *name)
: QmitkFunctionality() , m_MultiWidget(NULL), m_ShowRedGreen(false),
  m_ShowBothImages(true), m_ShowFixedImage(false), m_ShowMovingImage(false), m_Opacity(0.5), m_OriginalOpacity(1.0), m_FixedNode(NULL), m_MovingNode(NULL),
  m_OldMovingLayerSet(false), m_NewMovingLayerSet(false), m_NewMovingLayer(0), m_OldMovingLayer(0), m_Deactivated(false)
{
}

QmitkDeformableRegistrationView::~QmitkDeformableRegistrationView()
{
}

void QmitkDeformableRegistrationView::CreateQtPartControl(QWidget* parent)
{
  m_Controls.setupUi(parent);
  m_Parent->setEnabled(false);
  this->CreateConnections();

  // define data type for fixed image combobox
  m_Controls.m_FixedSelector->SetDataStorage( this->GetDefaultDataStorage() );
  m_Controls.m_FixedSelector->SetPredicate( mitk::NodePredicateDataType::New("Image") );

  // define data type for moving image combobox
  m_Controls.m_MovingSelector->SetDataStorage( this->GetDefaultDataStorage() );
  m_Controls.m_MovingSelector->SetPredicate( this->GetMovingImagePredicate() );
  connect(this,SIGNAL(calculateDemonsRegistration()),m_Controls.m_QmitkDemonsRegistrationViewControls,SLOT(CalculateTransformation()));
  this->CheckCalculateEnabled();
}

mitk::NodePredicateBase::Pointer QmitkDeformableRegistrationView::GetMovingImagePredicate()
{
  mitk::NodePredicateProperty::Pointer isFixedImage = mitk::NodePredicateProperty::New("selectedFixedImage", mitk::BoolProperty::New(true));
  mitk::NodePredicateNOT::Pointer notFixedImage = mitk::NodePredicateNOT::New(isFixedImage);
  mitk::NodePredicateDataType::Pointer isImage(mitk::NodePredicateDataType::New("Image"));
  mitk::NodePredicateAND::Pointer predicate = mitk::NodePredicateAND::New( notFixedImage, isImage );
  return predicate.GetPointer();
}

void QmitkDeformableRegistrationView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_Parent->setEnabled(true);
  m_MultiWidget = &stdMultiWidget;
  m_MultiWidget->SetWidgetPlanesVisibility(true);
  connect(this, SIGNAL( reinitFixed(const mitk::Geometry3D*) ), m_MultiWidget, SLOT( InitializeStandardViews(const mitk::Geometry3D*) ));
  connect(this, SIGNAL( reinitMoving(const mitk::Geometry3D*) ), m_MultiWidget, SLOT( InitializeStandardViews(const mitk::Geometry3D*) ));
}

void QmitkDeformableRegistrationView::StdMultiWidgetNotAvailable()
{
  m_Parent->setEnabled(false);
  m_MultiWidget = NULL;
}

void QmitkDeformableRegistrationView::CreateConnections()
{
  connect( m_Controls.m_FixedSelector, SIGNAL(activated(int)), this, SLOT(FixedSelected(int)));
  connect( m_Controls.m_MovingSelector, SIGNAL(activated(int)), this, SLOT(MovingSelected(int)));
  connect(m_Controls.m_ShowBothImages, SIGNAL(clicked()), this, SLOT(ShowBothImages()));
  connect(m_Controls.m_ShowFixedImage, SIGNAL(clicked()), this, SLOT(ShowFixedImage()));
  connect(m_Controls.m_ShowMovingImage, SIGNAL(clicked()), this, SLOT(ShowMovingImage()));
  connect(m_Controls.m_ShowRedGreenValues, SIGNAL(toggled(bool)), this, SLOT(ShowRedGreen(bool)));
  connect(m_Controls.m_OpacitySlider, SIGNAL(sliderMoved(int)), this, SLOT(OpacityUpdate(int)));
  connect(m_Controls.m_CalculateTransformation, SIGNAL(clicked()), this, SLOT(Calculate()));
    
  connect(m_Controls.m_FixedReinit, SIGNAL(clicked()), this, SLOT(reinitFixedClicked()));
  connect(m_Controls.m_MovingReinit, SIGNAL(clicked()), this, SLOT(reinitMovingClicked()));
  connect(m_Controls.m_GlobalReinit, SIGNAL(clicked()), this, SLOT(globalReinitClicked()));
}


void QmitkDeformableRegistrationView::DataStorageChanged()
{
  if (!m_Deactivated)
  {
    mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects = this->GetDataStorage()->GetAll();
    for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
      ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each node
    {
      if ( (nodeIt->Value().IsNotNull()) && (nodeIt->Value()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(nodeIt->Value()->GetData())==NULL)
      {
        nodeIt->Value()->SetVisibility(false);
      }
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    this->FixedSelected();
    this->MovingSelected();
  }
}

void QmitkDeformableRegistrationView::Activated()
{
  m_Deactivated = false;
  bool visible = false;
  bool selectedFixedImage = false;
  m_InvisibleNodesList.clear();
  mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects = this->GetDataStorage()->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
    ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each node
  {
    if ( (nodeIt->Value().IsNotNull()) && (nodeIt->Value()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(nodeIt->Value()->GetData())==NULL)
    {
      nodeIt->Value()->GetBoolProperty("visible", visible);
      // safe all previous invisible nodes in a set
      if (visible == false)
      {
        m_InvisibleNodesList.insert(nodeIt->Value());
      }
      else
      {
        nodeIt->Value()->SetVisibility(false);
      }
      // reset m_FixedNode
      nodeIt->Value()->GetBoolProperty("selectedFixedImage", selectedFixedImage);
      if (selectedFixedImage == true)
      {
        m_FixedNode = nodeIt->Value();
      }
    }
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkFunctionality::Activated();
  this->FixedSelected();
  this->MovingSelected();
  this->OpacityUpdate(m_Opacity);  
}

void QmitkDeformableRegistrationView::Deactivated()
{
  m_Deactivated = true;
  // reset previous invisible nodes to invisible and previous visible nodes to visible
  std::set<mitk::DataTreeNode*>::iterator setIter;
  mitk::DataStorage::SetOfObjects::ConstPointer setOfObjects = this->GetDataStorage()->GetAll();
  for (mitk::DataStorage::SetOfObjects::ConstIterator nodeIt = setOfObjects->Begin()
    ; nodeIt != setOfObjects->End(); ++nodeIt)  // for each node
  {
    if ( (nodeIt->Value().IsNotNull()) && (nodeIt->Value()->GetProperty("visible")) && dynamic_cast<mitk::Geometry2DData*>(nodeIt->Value()->GetData())==NULL)
    {
      setIter = m_InvisibleNodesList.find(nodeIt->Value());
      if (setIter != m_InvisibleNodesList.end())
      {
        nodeIt->Value()->SetVisibility(false);
      }
      else
      {
        nodeIt->Value()->SetVisibility(true);
      }
    }
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

void QmitkDeformableRegistrationView::FixedSelected(int)
{
  if (m_Controls.m_FixedSelector->GetSelectedNode().IsNotNull())
  {
    m_Controls.m_ShowFixedImage->setEnabled(true);
    if (m_FixedNode != m_Controls.m_FixedSelector->GetSelectedNode())
    {
      // remove changes on previous selected node
      if (m_FixedNode != NULL)
      {
        this->SetImageColor(false);
        m_FixedNode->SetVisibility(false);
        m_FixedNode->SetProperty("selectedFixedImage", mitk::BoolProperty::New(false));
      }
      // get selected node
      m_FixedNode = m_Controls.m_FixedSelector->GetSelectedNode();
      mitk::ColorProperty::Pointer colorProperty;
      colorProperty = dynamic_cast<mitk::ColorProperty*>(m_FixedNode->GetProperty("color"));
      if ( colorProperty.IsNotNull() )
      {
        m_FixedColor = colorProperty->GetColor();
      }
      this->SetImageColor(m_ShowRedGreen);
      m_FixedNode->SetProperty("selectedFixedImage", mitk::BoolProperty::New(true));

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
      m_Controls.m_ShowBothImages->setChecked(true);
      this->ShowBothImages();
    }
    m_Controls.m_ShowFixedImage->setEnabled(false);
  }
  m_Controls.m_MovingSelector->SetPredicate( this->GetMovingImagePredicate() );
  this->CheckCalculateEnabled();
}

void QmitkDeformableRegistrationView::MovingSelected(int)
{
  if (m_Controls.m_MovingSelector->GetSelectedNode().IsNotNull())
  {
    m_Controls.m_ShowMovingImage->setEnabled(true);
    if (m_MovingNode != m_Controls.m_MovingSelector->GetSelectedNode())
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
      m_MovingNode = m_Controls.m_MovingSelector->GetSelectedNode();
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
      m_Controls.m_ShowBothImages->setChecked(true);
      this->ShowBothImages();
    }
    m_Controls.m_ShowMovingImage->setEnabled(false);
  }
  this->CheckCalculateEnabled();
}

bool QmitkDeformableRegistrationView::CheckCalculate()
{
  if(m_MovingNode==m_FixedNode)
    return false;
  return true;
}

void QmitkDeformableRegistrationView::ShowRedGreen(bool redGreen)
{
  m_ShowRedGreen = redGreen;
  this->SetImageColor(m_ShowRedGreen);
}

void QmitkDeformableRegistrationView::ShowFixedImage()
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

void QmitkDeformableRegistrationView::ShowMovingImage()
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

void QmitkDeformableRegistrationView::ShowBothImages()
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

void QmitkDeformableRegistrationView::SetImageColor(bool redGreen)
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

void QmitkDeformableRegistrationView::OpacityUpdate(float opacity)
{
  m_Opacity = opacity;
  if (m_MovingNode != NULL)
  {
    m_MovingNode->SetOpacity(m_Opacity);
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkDeformableRegistrationView::OpacityUpdate(int opacity)
{
  float fValue = ((float)opacity)/100.0f;
  this->OpacityUpdate(fValue);
}

void QmitkDeformableRegistrationView::reinitFixedClicked()
{
  if (m_Controls.m_FixedSelector->GetSelectedNode().IsNotNull())
  {
    mitk::DataTreeNode* node = m_Controls.m_FixedSelector->GetSelectedNode();
    if (node != NULL )
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      emit reinitFixed( basedata->GetTimeSlicedGeometry() );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkDeformableRegistrationView::reinitMovingClicked()
{
  if (m_Controls.m_MovingSelector->GetSelectedNode().IsNotNull())
  {
    mitk::DataTreeNode* node = m_Controls.m_MovingSelector->GetSelectedNode();
    if (node != NULL )
    {
      mitk::BaseData::Pointer basedata = node->GetData();
      emit reinitMoving( basedata->GetTimeSlicedGeometry() );
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void QmitkDeformableRegistrationView::globalReinitClicked()
{
  if(m_Controls.m_FixedSelector->GetSelectedNode().IsNotNull())
  {
    mitk::RenderingManager::GetInstance()->InitializeViews( this->GetDefaultDataStorage() );
  }
}

void QmitkDeformableRegistrationView::CheckCalculateEnabled()
{
  if (m_Controls.m_MovingSelector->GetSelectedNode().IsNotNull() && m_Controls.m_FixedSelector->GetSelectedNode().IsNotNull())
  {
    m_Controls.m_CalculateTransformation->setEnabled(true);
  }
  else
  {
    m_Controls.m_CalculateTransformation->setEnabled(false);
  }
}

void QmitkDeformableRegistrationView::Calculate()
{
  m_Controls.m_QmitkDemonsRegistrationViewControls->SetFixedNode(m_Controls.m_FixedSelector->GetSelectedNode());
  m_Controls.m_QmitkDemonsRegistrationViewControls->SetMovingNode(m_Controls.m_MovingSelector->GetSelectedNode());
  if (m_Controls.m_DeformableTransform->tabText(m_Controls.m_DeformableTransform->currentIndex()) == "Demons")
  {
    emit calculateDemonsRegistration();
  }
}
