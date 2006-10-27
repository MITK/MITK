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
#include "QmitkSeedPointSetComponent.h"
#include "QmitkSeedPointSetComponentGUI.h"


#include <QmitkDataTreeComboBox.h>

#include <QmitkPointListWidget.h>

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>


/***************       CONSTRUCTOR      ***************/
QmitkSeedPointSetComponent::QmitkSeedPointSetComponent(QObject * parent, const char * parentName, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector)
: QmitkInteractionFunctionalityComponent(parent, parentName, mitkStdMultiWidget, it),
m_PointSetComponentGUI(NULL)
{
  SetDataTreeIterator(it);
  SetAvailability(true);

  SetComponentName("PointSet");
}

/***************        DESTRUCTOR      ***************/
QmitkSeedPointSetComponent::~QmitkSeedPointSetComponent()
{

}

///*************** GET DATA TREE ITERATOR ***************/
//mitk::DataTreeIteratorBase* QmitkPixelGreyValueManipulatorComponent::GetDataTreeIterator()
//{
//  return m_DataTreeIterator.GetPointer();
//}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkSeedPointSetComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkSeedPointSetComponent::GetTreeNodeSelector()
{
  return m_PointSetComponentGUI->GetTreeNodeSelector();
}

/***************   GET POINT SET NODE   ***************/
 mitk::DataTreeNode::Pointer QmitkSeedPointSetComponent::GetPointSetNode()
 {
  return  m_PointSetNode;
 }

 /************ Update DATATREECOMBOBOX(ES) *************/
void QmitkSeedPointSetComponent::UpdateDataTreeComboBoxes()
{
  if(GetTreeNodeSelector() != NULL)
  {
   GetTreeNodeSelector()->Update();
  }
}

/***************       CONNECTIONS      ***************/
void QmitkSeedPointSetComponent::CreateConnections()
{
  if ( m_PointSetComponentGUI )
  {
    connect( (QObject*)(m_PointSetComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
    connect((QObject*)(m_PointSetComponentGUI->GetShowTreeNodeSelectorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowTreeNodeSelector()));
    connect((QObject*)(m_PointSetComponentGUI->GetShowComponent()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowComponentContent()));

  }
}

/*************** SHOW COMPONENT CONTENT ***************/
void QmitkSeedPointSetComponent::ShowComponentContent()
{
 m_PointSetComponentGUI->GetComponentContent()->setShown(m_PointSetComponentGUI->GetShowComponent()->isChecked());
}

/*************** SHOW TREE NODE SELECTOR **************/
void QmitkSeedPointSetComponent::ShowTreeNodeSelector()
{
m_PointSetComponentGUI->GetTreeNodeSelectorContentGroupBox()->setShown(m_PointSetComponentGUI->GetShowTreeNodeSelectorGroupBox()->isChecked());
}

/***************     IMAGE SELECTED     ***************/
void QmitkSeedPointSetComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedImage = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_PointSetComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_PointSetComponentGUI->GetTreeNodeSelector()->GetFilter())
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

  TreeChanged();
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkSeedPointSetComponent::CreateControlWidget(QWidget* parent)
{
  m_PointSetComponentGUI = new QmitkSeedPointSetComponentGUI(parent);
  m_GUI = m_PointSetComponentGUI;

  m_PointSetComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
  //m_PointSetComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithProperty<mitk::Image>("Points"));
  m_PointSetComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithProperty<mitk::PointSet>("SeedPoints"));

  return m_PointSetComponentGUI;
}

/***************        ACTIVATED       ***************/
void QmitkSeedPointSetComponent::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;

    //BEGIN ONLY FOR POINTS******************************************************************************************************************************************
  if (m_PointSetNode.IsNull())
  {
    //Points are to define the two Points for the ThresholdGradient
    //add Point with crtl + leftMouseButton
    m_Seeds = mitk::PointSet::New();

    m_PointSetNode = mitk::DataTreeNode::New();
    m_PointSetNode->SetData(m_Seeds);
    mitk::ColorProperty::Pointer color = new mitk::ColorProperty(0.2, 0.0, 0.8);
    mitk::Point3D colorTwo; 
    mitk::FillVector3D(colorTwo, 0.2, 0.0, 0.8);
    m_PointSetComponentGUI->GetPointListWidget()->SwitchInteraction(&m_PointSetInteractor, &m_PointSetNode, 2, colorTwo,"Points ");  //-1 for unlimited points
    m_PointSetNode->SetProperty("color",color);
    m_PointSetNode->SetIntProperty("layer", 101);
    m_PointSetNode->SetProperty("name", new mitk::StringProperty("SeedPoints"));

    m_PointSetNode->SetProperty( "SeedPoints", new mitk::BoolProperty(true));

    m_PointSetInteractor = new mitk::PointSetInteractor("pointsetinteractor", m_PointSetNode, 2);

    m_DataTreeIterator.GetPointer()->Add(m_PointSetNode);

    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
  }
  else 
  {
    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
  }
  //END ONLY FOR POINTS******************************************************************************************************************************************
}

/***************       DEACTIVATED      ***************/
void QmitkSeedPointSetComponent::Deactivated()
{
  //BEGIN ONLY FOR SEEDPOINTS******************************************************************************************************************************************
  //deactivate m_SeedPointSetNode when leaving SurfaceCreator
  if (m_PointSetNode.IsNotNull())
  {
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  }
  //END ONLY FOR SEEDPOINTS******************************************************************************************************************************************
}


