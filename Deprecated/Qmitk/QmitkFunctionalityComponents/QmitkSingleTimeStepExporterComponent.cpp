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
#include "QmitkSingleTimeStepExporterComponent.h"
#include "QmitkSingleTimeStepExporterComponentGUI.h"

#include <QmitkDataTreeComboBox.h>

#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"
#include "QmitkStepperAdapter.h"

#include <mitkDataTreeNodeFactory.h>
#include <QmitkStdMultiWidget.h>
#include "QmitkSliderNavigator.h"
#include <qlayout.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

#include <mitkImage.h>

/***************       CONSTRUCTOR      ***************/
QmitkSingleTimeStepExporterComponent::QmitkSingleTimeStepExporterComponent(QObject * parent, const char * parentName, bool /*updateSelector*/, bool /*showSelector*/, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkInteractionFunctionalityComponent(parent, parentName, mitkStdMultiWidget, it),
m_SingleTimeStepExporterComponentGUI(NULL),
m_DataIt(it),
m_Stepper(NULL),
m_ImageTimeSelector(NULL)
{
  SetDataTreeIterator(it);
  SetAvailability(true);
  m_Active=false;
  m_CurrentItem= NULL;
  m_4D = false;
  SetComponentName("QmitkSingleTimeStepExporterComponent");
}

/***************        DESTRUCTOR      ***************/
QmitkSingleTimeStepExporterComponent::~QmitkSingleTimeStepExporterComponent()
{

}


/*************** SET DATA TREE ITERATOR ***************/
void QmitkSingleTimeStepExporterComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkSingleTimeStepExporterComponent::GetTreeNodeSelector()
{
  return m_SingleTimeStepExporterComponentGUI->GetTreeNodeSelector();
}


/***************   GET IMAGE CONTENT   ***************/
QGroupBox*  QmitkSingleTimeStepExporterComponent::GetImageContent()
{
  return (QGroupBox*) m_SingleTimeStepExporterComponentGUI->GetImageContent();
}

/************ Update DATATREECOMBOBOX(ES) *************/
void QmitkSingleTimeStepExporterComponent::UpdateDataTreeComboBoxes()
{
  if(GetTreeNodeSelector() != NULL)
  {
    GetTreeNodeSelector()->Update();
  }
}

/***************       CONNECTIONS      ***************/
void QmitkSingleTimeStepExporterComponent::CreateConnections()
{
  if ( m_SingleTimeStepExporterComponentGUI )
  {
    connect( (QObject*)(m_SingleTimeStepExporterComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
    connect((QObject*)(m_SingleTimeStepExporterComponentGUI->GetShowTreeNodeSelectorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowTreeNodeSelector()));
    connect((QObject*)(m_SingleTimeStepExporterComponentGUI->GetShowComponent()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowComponentContent()));


    //to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
    connect( (QObject*)(m_SingleTimeStepExporterComponentGUI->GetShowComponent()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 

    connect( (QObject*)(m_SingleTimeStepExporterComponentGUI->GetExportButton()), SIGNAL(pressed()), (QObject*) this, SLOT(AddSliceImage())); 
  }
}

/*************** SHOW COMPONENT CONTENT ***************/
void QmitkSingleTimeStepExporterComponent::ShowComponentContent()
{
  m_SingleTimeStepExporterComponentGUI->GetComponentContent()->setShown(m_SingleTimeStepExporterComponentGUI->GetShowComponent()->isChecked());
}

/*************** SHOW TREE NODE SELECTOR **************/
void QmitkSingleTimeStepExporterComponent::ShowTreeNodeSelector()
{
  m_SingleTimeStepExporterComponentGUI->GetImageContent()->setShown(m_SingleTimeStepExporterComponentGUI->GetShowTreeNodeSelectorGroupBox()->isChecked());
}

/***************     IMAGE SELECTED     ***************/
void QmitkSingleTimeStepExporterComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedItem = imageIt;
  //mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_SingleTimeStepExporterComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_SingleTimeStepExporterComponentGUI->GetTreeNodeSelector()->GetFilter())
    {
      if(imageIt)
      {
        m_CurrentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( imageIt->GetNode() ) );
      }
    }
  }

  if(static_cast<mitk::Image*>(m_SelectedItem->GetNode()->GetData())->GetDimension() > 2)
  {   
    m_4D = true;
    m_ImageTimeSelector->SetInput(static_cast<mitk::Image*>(m_SelectedItem->GetNode()->GetData()));
    m_Stepper->SetSteps(static_cast<mitk::Image*>(m_SelectedItem->GetNode()->GetData())->GetDimension(3));
    m_ImageTimeSelector->SetTimeNr(m_Stepper->GetPos());
  }
  else
  {
    m_4D = false;
  }

  TreeChanged();
}

void QmitkSingleTimeStepExporterComponent::AddSliceImage()
{
  if(m_Active)
  {
    //mitk::DataTreeFilter::Item* m_currentItem(NULL);
    //if(m_SingleTimeStepExporterComponentGUI)
    //{
    //  if(mitk::DataTreeFilter* filter = m_SingleTimeStepExporterComponentGUI->GetTreeNodeSelector()->GetFilter())
    //  {
    //    if(m_SelectedItem)
    //    {
    //      currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( m_SelectedItem->GetNode() ) );
    //    }
    //  }
    //}
    if(m_CurrentItem)
    {
      m_CurrentItem->SetSelected(true);

      //    m_ImageTimeSelector->SetInput(static_cast<mitk::Image*>(m_SelectedItem->GetNode()->GetData()));


      if(m_4D)//(static_cast<mitk::Image*>(m_SelectedItem->GetNode()->GetData())->GetDimension() > 2)
      {
        //m_Stepper->SetSteps(static_cast<mitk::Image*>(m_SelectedItem->GetNode()->GetData())->GetDimension(3));
        m_ImageTimeSelector->SetTimeNr(m_Stepper->GetPos());

        mitk::DataTreeNode::Pointer timeSliceNode = mitk::DataTreeNode::New(); 
        timeSliceNode->SetData(m_ImageTimeSelector->GetOutput() );


        std::ostringstream buffer;
        unsigned int pos = m_Stepper->GetPos();
        buffer << pos;
        QString timeSliceName = m_SingleTimeStepExporterComponentGUI->GetTreeNodeSelector()->currentText();
        buffer << "_";
        buffer << timeSliceName.ascii();
        //buffer << "Slice_";


        std::string timeSliceNodeName = "Slice_" + buffer.str();

        timeSliceNode->SetProperty("timeSlice", mitk::BoolProperty::New(true));
        timeSliceNode->SetProperty("name", mitk::StringProperty::New(timeSliceNodeName));
        //timeSliceNode->SetProperty("name", mitk::StringProperty::New("3DImage"));

        m_DataIt->Add(timeSliceNode);
      }
      else
      {
        mitk::DataTreeNode::Pointer timeSliceNode = mitk::DataTreeNode::New(); 
        timeSliceNode->SetData(m_ImageTimeSelector->GetOutput() );

        timeSliceNode->SetProperty("timeSlice", mitk::BoolProperty::New(true));
        timeSliceNode->SetProperty("name", mitk::StringProperty::New("timeSliceImage"));

        m_DataIt->Add(timeSliceNode);
        m_Stepper->SetSteps(0);
      }
    }
  }
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkSingleTimeStepExporterComponent::CreateControlWidget(QWidget* parent)
{
  m_SingleTimeStepExporterComponentGUI = new QmitkSingleTimeStepExporterComponentGUI(parent);
  m_GUI = m_SingleTimeStepExporterComponentGUI;

  m_SingleTimeStepExporterComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
  m_SingleTimeStepExporterComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));

  m_ImageTimeSelector = mitk::ImageTimeSelector::New();

  m_Stepper = mitk::Stepper::New();  
  //QmitkStepperAdapter* stepperAdapter = new QmitkStepperAdapter(((QObject*)(m_SingleTimeStepExporterComponentGUI->GetQmitkSliderNavigator())), m_Stepper, "stepperAdapter");  

  // new QmitkStepperAdapter((QObject*)m_SliceSliderComponentGUI->GetTransversalSlider(), m_MultiWidget->mitkWidget1->GetSliceNavigationController()->GetSlice(), "sliceNavigatorTransversal");


  typedef itk::SimpleMemberCommand<QmitkSingleTimeStepExporterComponent> CommandType;
  CommandType::Pointer command = CommandType::New();
  command->SetCallbackFunction(this, &QmitkSingleTimeStepExporterComponent::Refetch);
  m_Stepper->AddObserver(itk::ModifiedEvent(), command);

  return m_SingleTimeStepExporterComponentGUI;
}

void QmitkSingleTimeStepExporterComponent::Refetch()
{
  if((m_ImageTimeSelector.IsNotNull()) && (m_Stepper.IsNotNull()) && (m_Stepper->GetPos()))
  {
    m_ImageTimeSelector->SetTimeNr(m_Stepper->GetPos());
  }
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkSingleTimeStepExporterComponent::GetContentContainer()
{
  return m_SingleTimeStepExporterComponentGUI->GetComponentContent();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkSingleTimeStepExporterComponent::GetMainCheckBoxContainer()
{
  return m_SingleTimeStepExporterComponentGUI->GetShowComponent();
}

/*********** SET CONTENT CONTAINER VISIBLE ************/
void QmitkSingleTimeStepExporterComponent::SetContentContainerVisibility(bool)
{
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
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    if(m_AddedChildList[i]->GetContentContainer() != NULL)
    {
      m_AddedChildList[i]->GetContentContainer()->setShown(GetMainCheckBoxContainer()->isChecked());
    }
    if(m_AddedChildList[i]->GetMainCheckBoxContainer() != NULL)
    {
      m_AddedChildList[i]->GetMainCheckBoxContainer()->setChecked(GetMainCheckBoxContainer()->isChecked());
    }
    m_AddedChildList[i]->SetContentContainerVisibility(GetMainCheckBoxContainer()->isChecked());
  } 
}

/***************        ACTIVATED       ***************/
void QmitkSingleTimeStepExporterComponent::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;
  TreeChanged();
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Activated();
  } 
}

/***************       DEACTIVATED      ***************/
void QmitkSingleTimeStepExporterComponent::Deactivated()
{
  QmitkBaseFunctionalityComponent::Deactivated();
  m_Active = false;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Deactivated();
  } 
}
