/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-08-07 14:44:36 +0200 (Di, 07 Aug 2007) $
Version:   $Revision: 11473 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkDisplayPointSetComponent.h"
#include "QmitkDisplayPointSetComponentGUI.h"


#include <QmitkDataTreeComboBox.h>

#include <QmitkPointListWidget.h>

#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qstring.h>


/***************       CONSTRUCTOR      ***************/
QmitkDisplayPointSetComponent::QmitkDisplayPointSetComponent(QObject * parent, const char * parentName, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool /*updateSelector*/, bool /*showSelector*/)
: QmitkInteractionFunctionalityComponent(parent, parentName, mitkStdMultiWidget, it),
m_PointSetComponentGUI(NULL),
m_PointSetNodeExisting(false),
m_It(it),
m_PointSetNodeName("DisplayPoints")
{
  SetDataTreeIterator(it);
  SetAvailability(true);

  SetComponentName("PointSet");
}

/***************        DESTRUCTOR      ***************/
QmitkDisplayPointSetComponent::~QmitkDisplayPointSetComponent()
{

}

 /***************   GET IMAGE CONTENT   ***************/
QGroupBox*  QmitkDisplayPointSetComponent::GetImageContent()
{
  return (QGroupBox*) m_PointSetComponentGUI->GetImageContent();
}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkDisplayPointSetComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkDisplayPointSetComponent::GetTreeNodeSelector()
{
  return m_PointSetComponentGUI->GetTreeNodeSelector();
}

/***************   GET POINT SET NODE   ***************/
 mitk::DataTreeNode::Pointer QmitkDisplayPointSetComponent::GetPointSetNode()
 {
  return  m_PointSetNode;
 }

  /************GET SEED POINT SET COMPONENT GUI**********/
 QmitkDisplayPointSetComponentGUI* QmitkDisplayPointSetComponent::GetDisplayPointSetComponentGUI()
 {
   return m_PointSetComponentGUI;
 }

 /************ Update DATATREECOMBOBOX(ES) *************/
void QmitkDisplayPointSetComponent::UpdateDataTreeComboBoxes()
{
  if(GetTreeNodeSelector() != NULL)
  {
   GetTreeNodeSelector()->Update();
  }
}

/***************       CONNECTIONS      ***************/
void QmitkDisplayPointSetComponent::CreateConnections()
{
  if ( m_PointSetComponentGUI )
  {
    connect( (QObject*)(m_PointSetComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
    connect((QObject*)(m_PointSetComponentGUI->GetShowTreeNodeSelectorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowTreeNodeSelector()));
    connect((QObject*)(m_PointSetComponentGUI->GetShowComponent()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowComponentContent()));

        //to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
    connect( (QObject*)(m_PointSetComponentGUI->GetShowComponent()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 

  }
}


/*************** SHOW COMPONENT CONTENT ***************/
void QmitkDisplayPointSetComponent::ShowComponentContent()
{
 m_PointSetComponentGUI->GetComponentContent()->setShown(m_PointSetComponentGUI->GetShowComponent()->isChecked());
 SetContentContainerVisibility(m_PointSetComponentGUI->GetShowComponent()->isChecked());
}

/*************** SHOW TREE NODE SELECTOR **************/
void QmitkDisplayPointSetComponent::ShowTreeNodeSelector()
{
m_PointSetComponentGUI->GetImageContent()->setShown(m_PointSetComponentGUI->GetShowTreeNodeSelectorGroupBox()->isChecked());
}

/***************     IMAGE SELECTED     ***************/
void QmitkDisplayPointSetComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedItem = imageIt;
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
QWidget* QmitkDisplayPointSetComponent::CreateControlWidget(QWidget* parent)
{
  m_PointSetComponentGUI = new QmitkDisplayPointSetComponentGUI(parent);
  m_GUI = m_PointSetComponentGUI;

  m_PointSetComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
  //m_PointSetComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithProperty<mitk::Image>("Points"));
  m_PointSetComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithProperty<mitk::PointSet>("DisplayPoints"));
  GetMainCheckBoxContainer()->setChecked(false);
  SetContentContainerVisibility(m_PointSetComponentGUI->GetShowComponent()->isChecked());

  return m_PointSetComponentGUI;
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkDisplayPointSetComponent::GetContentContainer()
{
 return m_PointSetComponentGUI->GetComponentContent();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkDisplayPointSetComponent::GetMainCheckBoxContainer()
{
 return m_PointSetComponentGUI->GetShowComponent();
}

/*********** SET CONTENT CONTAINER VISIBLE ************/
void QmitkDisplayPointSetComponent::SetContentContainerVisibility(bool visible)
{
   GetContentContainer()->setShown(visible);
  if(GetMainCheckBoxContainer() != NULL)
  {
    if(GetMainCheckBoxContainer()->isChecked())
    {
      Activated();
    }
    else
    {
      Deactivated();
    }
  }
}

/***************   SET DATA TREE NAME   ***************/
void QmitkDisplayPointSetComponent::SetDataTreeName(std::string pointSetNodeName)
{
  m_PointSetNodeName = pointSetNodeName;
}

/***************        ACTIVATED       ***************/
void QmitkDisplayPointSetComponent::Activated()
{
  if(m_PointSetComponentGUI->GetShowComponent()->isChecked())
  {
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;
  IsNodeExisting();
  CreatePointSetNode();

  }
}

void QmitkDisplayPointSetComponent::IsNodeExisting()
{
 //GetIteratorToNode(dataTree, m_PointSetNode, m_It); 
  mitk::DataTree* dataTree = (mitk::DataTree*)m_It->GetTree();//m_It->GetFirstNodeByType<mitk::PointSet>(GetDataTreeIterator());
   // mitk::DataTreeIteratorClone dataTreeIteratorClone = dataTree->GetIteratorToNode( dataTree, m_PointSetNode );
  mitk::DataTreeIteratorClone iTClone = dataTree->GetIteratorToNode(dataTree, m_PointSetNode); 
  if(iTClone->IsAtEnd())
  {  
   m_PointSetNodeExisting = false;
  }
  else
   {  
   m_PointSetNodeExisting = true;
  }
}

void QmitkDisplayPointSetComponent::CreatePointSetNode()
{  
  if(m_Active)
  {
    if(!m_PointSetNodeExisting)
    {

      //BEGIN ONLY FOR POINTS******************************************************************************************************************************************
      //if (m_PointSetNode.IsNull())
      //{
        //Points are to define the two Points for the ThresholdGradient
        //add Point with crtl + leftMouseButton
        m_Seeds = mitk::PointSet::New();

        m_PointSetNode = mitk::DataTreeNode::New();
        m_PointSetNode->SetData(m_Seeds);
        mitk::ColorProperty::Pointer color = new mitk::ColorProperty(0.2, 0.0, 0.8);
        mitk::Point3D colorTwo; 
        mitk::FillVector3D(colorTwo, 0.2, 0.0, 0.8);
        m_PointSetInteractor = mitk::DisplayPointSetInteractor::New("pointsetinteractor", m_PointSetNode, 2);
        m_PointSetInteractor->DebugOn();
        mitk::PointSetInteractor::Pointer tmpPointSetInteractor = m_PointSetInteractor.GetPointer(); // point list widget expects an mitk::PointSetInteractor::Pointer*, 
                                                                                                     // not an mitk::DisplayPointSetInteractor::Pointer. Thus, an
                                                                                                     // appropriate smart pointer is instantiated here. It seems that 
                                                                                                     // the point list widget makes a copy of the smart pointer, so
                                                                                                     // I think there should be no problem, when execution leaves the
                                                                                                     // scope of tmpPointSetInteractor.
        m_PointSetComponentGUI->GetPointListWidget()->SwitchInteraction(&tmpPointSetInteractor, &m_PointSetNode, -1, colorTwo,"Points ");  //-1 for unlimited points
        m_PointSetNode->SetProperty("color",color);
        m_PointSetNode->SetInteractor(m_PointSetInteractor);
        m_PointSetNode->SetIntProperty("layer", 101);
        m_PointSetNode->SetProperty("name", new mitk::StringProperty(m_PointSetNodeName));

        m_PointSetNode->SetProperty( "DisplayPoints", new mitk::BoolProperty(true));
        m_DataTreeIterator.GetPointer()->Add(m_PointSetNode);
        //m_PointSetNodeExisting = true;

        mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
      }
      else 
      {
        mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
      }    
  }
}
/***************       DEACTIVATED      ***************/
void QmitkDisplayPointSetComponent::Deactivated()
{
    QmitkBaseFunctionalityComponent::Deactivated();
    m_Active = false;
	GetMainCheckBoxContainer()->setChecked(false);
  //  if(m_PointSetNode.IsNotNull())
  //{
  //  mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
  //  while ( !iteratorClone->IsAtEnd() )
  //  {
  //    mitk::DataTreeNode::Pointer node = iteratorClone->Get();

  //    std::string name;
  //    node->GetName(name);

  //    if(name == m_PointSetNodeName)
  //    {
  //      iteratorClone->Disconnect();
  //      m_PointSetNodeExisting = false;
  //    }
  //    ++iteratorClone;
  //  }
  //}
  //BEGIN ONLY FOR DisplayPointS******************************************************************************************************************************************
  //deactivate m_DisplayPointSetNode when leaving SurfaceCreator
  //if (m_PointSetNode.IsNotNull())
  //{
    mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
  //}
  //END ONLY FOR DisplayPointS******************************************************************************************************************************************
}

mitk::DisplayPointSetInteractor* QmitkDisplayPointSetComponent::GetPointSetInteractor()
{
  return m_PointSetInteractor;
}
