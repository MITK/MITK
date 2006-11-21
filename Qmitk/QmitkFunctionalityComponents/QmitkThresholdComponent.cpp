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
#include "QmitkThresholdComponent.h"
#include "QmitkThresholdComponentGUI.h"


#include <QmitkDataTreeComboBox.h>

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>


/***************       CONSTRUCTOR      ***************/
QmitkThresholdComponent::QmitkThresholdComponent(QObject * parent, const char * parentName, QmitkStdMultiWidget * /*mitkStdMultiWidget*/, mitk::DataTreeIteratorBase* it, bool /*updateSelector*/, bool /*showSelector*/)
: QmitkFunctionalityComponentContainer(parent, parentName),
m_ThresholdImageNode(NULL),
m_ThresholdComponentGUI(NULL),
m_ThresholdNodeExisting(false)
{
  SetDataTreeIterator(it);
  SetAvailability(true);

  SetComponentName("ThresholdFinder");
  m_Node = it->Get();
}

/***************        DESTRUCTOR      ***************/
QmitkThresholdComponent::~QmitkThresholdComponent()
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkThresholdComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
  m_Node = m_DataTreeIterator->Get();
}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkThresholdComponent::SetSelectorVisibility(bool visibility)
{
  if(m_ThresholdComponentGUI)
  {
    m_ThresholdComponentGUI->GetSelectDataGroupBox()->setShown(visibility);
  }
  m_ShowSelector = visibility;
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkThresholdComponent::GetTreeNodeSelector()
{
  return m_ThresholdComponentGUI->GetTreeNodeSelector();
}

/***************       CONNECTIONS      ***************/
void QmitkThresholdComponent::CreateConnections()
{
  if ( m_ThresholdComponentGUI )
  {
    connect( (QObject*)(m_ThresholdComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
    connect( (QObject*)(m_ThresholdComponentGUI->GetThresholdFinderGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowThresholdFinderContent(bool)));     
    connect( (QObject*)(m_ThresholdComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool))); 

    connect( (QObject*)(m_ThresholdComponentGUI->GetThresholdInputSlider()), SIGNAL(sliderMoved(int)), (QObject*) this, SLOT(ThresholdSliderChanged(int)));
    connect( (QObject*)(m_ThresholdComponentGUI->GetThresholdInputNumber()), SIGNAL(returnPressed()), (QObject*) this, SLOT(ThresholdValueChanged()));    
    connect( (QObject*)(m_ThresholdComponentGUI->GetShowThresholdGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowThreshold(bool)));  

    //to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
    connect( (QObject*)(m_ThresholdComponentGUI->GetThresholdFinderGroupBox()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 
  }
}

/***************     IMAGE SELECTED     ***************/
void QmitkThresholdComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedItem = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_ThresholdComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_ThresholdComponentGUI->GetTreeNodeSelector()->GetFilter())
    {
      if(imageIt)
      {
        currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( imageIt->GetNode() ) );
      }
    }
  }
  if(currentItem)
  {
    currentItem->SetSelected(true);
  }
  if(m_ThresholdComponentGUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      m_AddedChildList[i]->ImageSelected(m_SelectedItem);
    }
  }
  m_Node = const_cast<mitk::DataTreeNode*>(m_SelectedItem->GetNode());
  DataObjectSelected();
  SetSliderRange();
  ShowThreshold();
}

/***************  DATA OBJECT SELECTED   **************/
void QmitkThresholdComponent::DataObjectSelected()
{
  if(m_Active)
  {
    if(m_ThresholdNodeExisting)
    {
      m_ThresholdImageNode->SetData(m_Node->GetData());
    }
    else
    {
      CreateThresholdImageNode();
      m_ThresholdImageNode->SetData(m_Node->GetData());
    }
    ShowThreshold();
  }
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkThresholdComponent::CreateControlWidget(QWidget* parent)
{
  m_ThresholdComponentGUI = new QmitkThresholdComponentGUI(parent);
  m_GUI = m_ThresholdComponentGUI;

  m_ThresholdComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());

  if(!m_ShowSelector)
  {
    m_ThresholdComponentGUI->GetSelectDataGroupBox()->setShown(false);
  }

  m_ThresholdComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));
  //CreateConnections();

  return m_ThresholdComponentGUI;
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkThresholdComponent::GetContentContainer()
{
 return m_ThresholdComponentGUI->GetContainerContent();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkThresholdComponent::GetMainCheckBoxContainer()
{
 return m_ThresholdComponentGUI->GetThresholdFinderGroupBox();
}

///*********** SET CONTENT CONTAINER VISIBLE ************/
//void QmitkThresholdComponent::SetContentContainerVisibility()
//{
//     for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
//    {
//      if(m_AddedChildList[i]->GetContentContainer() != NULL)
//      {
//        m_AddedChildList[i]->GetContentContainer()->setShown(GetMainCheckBoxContainer()->isChecked());
//      }
//    } 
//}

/***************        ACTIVATED       ***************/
void QmitkThresholdComponent::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Activated();
  } 
  CreateThresholdImageNode();
  ShowThreshold();
}

/***************       DEACTIVATED      ***************/
void QmitkThresholdComponent::Deactivated()
{
  QmitkBaseFunctionalityComponent::Deactivated();
  m_Active = false;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Deactivated();
  } 
  ShowThreshold();
  if(m_ThresholdComponentGUI->GetDeleateImageIfDeactivatedCheckBox()->isChecked())
  {
    DeleteThresholdNode();
  }
}

///*************CREATE THRESHOLD IMAGE NODE************/
void QmitkThresholdComponent::CreateThresholdImageNode()
{ 
  if(m_Active)
  {
    if(!m_ThresholdNodeExisting)
    {
      if (m_Node)
      {
        m_ThresholdImageNode = mitk::DataTreeNode::New();
        mitk::StringProperty::Pointer nameProp = new mitk::StringProperty("Thresholdview image" );
        m_ThresholdImageNode->SetProperty( "name", nameProp );
        mitk::BoolProperty::Pointer componentThresholdImageProp = new mitk::BoolProperty(true);
        m_ThresholdImageNode->SetProperty( "isComponentThresholdImage", componentThresholdImageProp );

        m_ThresholdImageNode->SetData(m_Node->GetData());
        m_ThresholdImageNode->SetColor(0.0,1.0,0.0);
        m_ThresholdImageNode->SetOpacity(.25);
        int layer = 0;
        m_Node->GetIntProperty("layer", layer);
        m_ThresholdImageNode->SetIntProperty("layer", layer+1);
        m_ThresholdImageNode->SetLevelWindow(mitk::LevelWindow(m_ThresholdComponentGUI->GetNumberValue(),1));

        mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
        iteratorClone->GoToBegin();
        while ( !iteratorClone->IsAtEnd() )
        {
          mitk::DataTreeNode::Pointer node = iteratorClone->Get();
          if (  node == m_Node )
          {
            iteratorClone->Add(m_ThresholdImageNode);
          }
          ++iteratorClone;
          m_ThresholdNodeExisting = true;
        }
      }
    }
  }
}

///************ SHOW THRESHOLD FINDER CONTENT ***********/
void QmitkThresholdComponent::ShowThresholdFinderContent(bool)
{
  m_ThresholdComponentGUI->GetShowThresholdGroupBox()->setShown(m_ThresholdComponentGUI->GetThresholdFinderGroupBox()->isChecked());

  if(m_ShowSelector)
  {
    m_ThresholdComponentGUI->GetSelectDataGroupBox()->setShown(m_ThresholdComponentGUI->GetThresholdFinderGroupBox()->isChecked());
  }

  ShowThreshold();
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkThresholdComponent::ShowImageContent(bool)
{
  m_ThresholdComponentGUI->GetImageContent()->setShown(m_ThresholdComponentGUI->GetSelectDataGroupBox()->isChecked());
  m_ThresholdComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
}

///***************      SHOW THRESHOLD     **************/
void QmitkThresholdComponent::ShowThreshold(bool)
{
  if(m_ThresholdImageNode)
  {
    if(m_Active == true)
    {
      m_ThresholdImageNode->SetProperty("visible", new mitk::BoolProperty((m_ThresholdComponentGUI->GetShowThresholdGroupBox()->isChecked())) );
    }
    else
    {
      if(m_ThresholdComponentGUI->GetDeleateImageIfDeactivatedCheckBox()->isChecked())
      {
        m_ThresholdImageNode->SetProperty("visible", new mitk::BoolProperty((false)) );
      }
    }
    m_ThresholdComponentGUI->GetThresholdValueContent()->setShown(m_ThresholdComponentGUI->GetShowThresholdGroupBox()->isChecked());
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

///*************** THRESHOLD VALUE CHANGED **************/
//By Slider
void QmitkThresholdComponent::ThresholdSliderChanged(int)
{
  int value = m_ThresholdComponentGUI->GetThresholdInputSlider()->value();
  if (m_ThresholdImageNode)
  {
    m_ThresholdImageNode->SetLevelWindow(mitk::LevelWindow(value,1));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  m_ThresholdComponentGUI->GetThresholdInputNumber()->setText(QString::number(value)); 
}

///*************** THRESHOLD VALUE CHANGED **************/
//By LineEdit
void QmitkThresholdComponent::ThresholdValueChanged( )
{
  int value = m_ThresholdComponentGUI->GetNumberValue();
  if (m_ThresholdImageNode)
  {
    m_ThresholdImageNode->SetLevelWindow(mitk::LevelWindow(value,1));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  m_ThresholdComponentGUI->GetThresholdInputSlider()->setValue(value);
}


///***************     SET SLIDER RANGE    **************/
void QmitkThresholdComponent::SetSliderRange()
{
  if(m_Active)
  {
    if(m_ThresholdComponentGUI->GetThresholdFinderGroupBox()->isChecked()==true)
    {
      mitk::Image* currentImage = dynamic_cast<mitk::Image*>(m_ThresholdImageNode->GetData());
      if(currentImage)
      {
        m_ThresholdComponentGUI->GetThresholdInputSlider()->setMinValue((int)currentImage->GetScalarValueMin());
        m_ThresholdComponentGUI->GetThresholdInputSlider()->setMaxValue((int)currentImage->GetScalarValueMax());
      }
    }
  }
}

///***************  DELETE THRESHOLD NODE  **************/
void QmitkThresholdComponent::DeleteThresholdNode()
{
  if(m_ThresholdImageNode)
  {
    mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
    while ( !iteratorClone->IsAtEnd() )
    {
      mitk::DataTreeNode::Pointer node = iteratorClone->Get();

      std::string name;
      node->GetName(name);

      if(name == "Thresholdview image")
      {
        iteratorClone->Disconnect();
        m_ThresholdNodeExisting = false;
      }
      ++iteratorClone;
    }
  }
}
