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
#include "QmitkSliceSliderNavigationComponent.h"
#include "QmitkSliceSliderNavigationComponentGUI.h"


#include <QmitkDataTreeComboBox.h>

#include "QmitkStepperAdapter.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkSelectableGLWidget.h"
#include "QmitkSliderNavigator.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>


/***************       CONSTRUCTOR      ***************/
QmitkSliceSliderNavigationComponent::QmitkSliceSliderNavigationComponent(QObject * parent, const char * parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector), 
m_MultiWidget(mitkStdMultiWidget)
{
  ////test
  //mitk::DataTreeIteratorClone tempit;
  //tempit=dynamic_cast<mitk::DataTreeIteratorBase*>(it)->Clone();
  ////test

  SetDataTreeIterator(it);
  SetAvailability(true);

  SetComponentName("SliceSliderNavigation");
  m_Node = it->Get();
}

/***************        DESTRUCTOR      ***************/
QmitkSliceSliderNavigationComponent::~QmitkSliceSliderNavigationComponent()
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkSliceSliderNavigationComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  //  mitk::DataTreeIteratorClone* tempit = new mitk::DataTreeIteratorClone();
  //  tempit=dynamic_cast<mitk::DataTreeIteratorClone*>(it)->Clone();


  m_DataTreeIterator = it;
  //m_Node = m_DataTreeIterator->Get();
}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox* QmitkSliceSliderNavigationComponent::GetImageContent()
{
  return (QGroupBox*) m_SliceSliderComponentGUI->GetImageContent();
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkSliceSliderNavigationComponent::GetTreeNodeSelector()
{
  return m_SliceSliderComponentGUI->GetTreeNodeSelector();
}

/***************       CONNECTIONS      ***************/
void QmitkSliceSliderNavigationComponent::CreateConnections()
{
  if ( m_SliceSliderComponentGUI )
  {
    connect( (QObject*)(m_SliceSliderComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));

    connect( (QObject*)(m_SliceSliderComponentGUI->GetSliceSliderNavigator()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(HideOrShow()));  
    connect( (QObject*)(m_SliceSliderComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(HideOrShow()));  

    //to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
    connect( (QObject*)(m_SliceSliderComponentGUI->GetSliceSliderNavigator()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 
  }
}

/***************      HIDE OR SHOW      ***************/
void QmitkSliceSliderNavigationComponent::HideOrShow()
{
  //the entir component
  m_SliceSliderComponentGUI->GetSliceSliderNavigatorContentGroupBox()->setShown(m_SliceSliderComponentGUI->GetSliceSliderNavigator()->isChecked());

  if(m_ShowSelector)
  {
    m_SliceSliderComponentGUI->GetImageContent()->setShown(m_SliceSliderComponentGUI->GetSelectDataGroupBox()->isChecked());
  }
  else
  {
    m_SliceSliderComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
  }
}

/***************     IMAGE SELECTED     ***************/
void QmitkSliceSliderNavigationComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedItem = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_SliceSliderComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_SliceSliderComponentGUI->GetTreeNodeSelector()->GetFilter())
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
  if(m_SliceSliderComponentGUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(m_SelectedItem);      
    }
  }
  m_Node = const_cast<mitk::DataTreeNode*>(m_SelectedItem->GetNode());
  FourDimensionsOrNot();
}

/***************            4D             **************/
void QmitkSliceSliderNavigationComponent::FourDimensionsOrNot()
{
  if(m_SliceSliderComponentGUI)
  {
    mitk::DataTreeFilter* filter = m_SliceSliderComponentGUI->GetTreeNodeSelector()->GetFilter();
    mitk::DataTreeIteratorClone mitkImageIterator = filter->GetIteratorToSelectedItem();

    if(mitkImageIterator.GetPointer())
    {
      mitk::Image* mitkImage = static_cast<mitk::Image*> (mitkImageIterator->Get()->GetData());

      if(mitkImage->GetDimension() == 4)
      {
        m_SliceSliderComponentGUI->GetTemporalSlider()->setShown(true);
        m_SliceSliderComponentGUI->GetTextLabelTemporal()->setShown(true);
      }
      else
      {
        m_SliceSliderComponentGUI->GetTemporalSlider()->setShown(false);
        m_SliceSliderComponentGUI->GetTextLabelTemporal()->setShown(false);
      }
    }
  }
}

/***************   CREATE CONTROL WIDGET   **************/
QWidget* QmitkSliceSliderNavigationComponent::CreateControlWidget(QWidget* parent)
{
  m_SliceSliderComponentGUI = new QmitkSliceSliderNavigationComponentGUI(parent);
  m_GUI = m_SliceSliderComponentGUI;

  m_SliceSliderComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
  m_SliceSliderComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));


  new QmitkStepperAdapter((QObject*)m_SliceSliderComponentGUI->GetTransversalSlider(), m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice(), "sliceNavigatorTransversal");
  new QmitkStepperAdapter((QObject*)m_SliceSliderComponentGUI->GetSagittalSlider(), m_MultiWidget->mitkWidget2->GetSliceNavigationController()->GetSlice(), "sliceNavigatorSagittal");
  new QmitkStepperAdapter((QObject*)m_SliceSliderComponentGUI->GetCoronalSlider(), m_MultiWidget->mitkWidget3->GetSliceNavigationController()->GetSlice(), "sliceNavigatorCoronal");
  new QmitkStepperAdapter((QObject*)m_SliceSliderComponentGUI->GetTemporalSlider(), m_MultiWidget->GetTimeNavigationController()->GetTime(), "sliceNavigatorTemporal");

  SetContentContainerVisibility(GetMainCheckBoxContainer()->isChecked());

  if(m_ShowSelector)
  {
    m_SliceSliderComponentGUI->GetImageContent()->setShown(m_SliceSliderComponentGUI->GetSelectDataGroupBox()->isChecked());
  }
  else
  {
    m_SliceSliderComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
  }

  return m_SliceSliderComponentGUI;
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkSliceSliderNavigationComponent::GetContentContainer()
{
  return m_SliceSliderComponentGUI->GetSliceSliderNavigatorContentGroupBox();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkSliceSliderNavigationComponent::GetMainCheckBoxContainer()
{
  return m_SliceSliderComponentGUI->GetSliceSliderNavigator();
}




