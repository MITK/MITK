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
#include "QmitkPixelGreyValueManipulatorComponent.h"
#include "QmitkPixelGreyValueManipulatorComponentGUI.h"

#include "QmitkSeedPointSetComponent.h"

#include <QmitkDataTreeComboBox.h>
#include <QmitkPointListWidget.h>//for the PointWidget

#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"
#include <mitkCuboid.h>
#include <mitkCone.h>
#include <mitkEllipsoid.h>
#include <mitkCylinder.h>
#include <mitkExtrudedContour.h>
#include "QmitkStdMultiWidget.h"
#include <mitkDataTreeNodeFactory.h>

#include <mitkImageAccessByItk.h>

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcursor.h>
#include <qapplication.h>
#include <qcheckbox.h>
#include <qstring.h>

const char* NAMEFORBOUNDINGOBJECT = "Bounding Object for Pixel Manipulation";

/***************       CONSTRUCTOR      ***************/
QmitkPixelGreyValueManipulatorComponent::QmitkPixelGreyValueManipulatorComponent(QObject *parent, const char *parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget * mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector),
m_MultiWidget(mitkStdMultiWidget),
m_PixelGreyValueManipulatorComponentGUI(NULL),
m_MitkImage(NULL),
m_MitkImageIterator(NULL),
m_PixelChangedImageCounter(0),
m_ManipulationMode(0),
m_ManipulationArea(0),
m_PointSet(NULL),
/*m_BoundingObjectGroup(NULL), // BoundingObjectGroup that includes all created BoundingObjects to calculate a big one*/
/*m_BoundingObjectList(NULL),  // Vector that includes all to the node added BoundingObjects*/
m_DataIt(it),
m_BoundingObjectExistingFlag(false)
{
    SetDataTreeIterator(it);
    SetAvailability(true);
    SetComponentName("PixelGreyValueManipulatorComponent");
    m_UpdateSelector = updateSelector; 
    m_ShowSelector = showSelector;
    m_CheckBoxChecked = false;
}

/***************        DESTRUCTOR      ***************/
QmitkPixelGreyValueManipulatorComponent::~QmitkPixelGreyValueManipulatorComponent()
{

}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkPixelGreyValueManipulatorComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
    m_DataTreeIterator = it;
}

QmitkSeedPointSetComponent* QmitkPixelGreyValueManipulatorComponent::GetPointSet()
{
    return m_PointSet;
}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox*  QmitkPixelGreyValueManipulatorComponent::GetImageContent()
{
    return (QGroupBox*) m_PixelGreyValueManipulatorComponentGUI->GetImageContent();
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkPixelGreyValueManipulatorComponent::GetTreeNodeSelector()
{
    return m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector();
}

/************ Update DATATREECOMBOBOX(ES) *************/
void QmitkPixelGreyValueManipulatorComponent::UpdateDataTreeComboBoxes()
{
    m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->Update();
    m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->Update();
}

/***************       CONNECTIONS      ***************/
void QmitkPixelGreyValueManipulatorComponent::CreateConnections()
{
    if ( m_PixelGreyValueManipulatorComponentGUI )
    {
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));        
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(SegmentationSelected(const mitk::DataTreeFilter::Item *)));        


        //connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetPixelGreyValueManipulatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowPixelGreyValueManipulatorContent(bool)));
        //connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool)));

        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetPixelGreyValueManipulatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(HideOrShow()));
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(HideOrShow()));
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetShowBaseThresholdGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(HideOrShow()));
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelectorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(HideOrShow()));

        //Button "create new manipulated image" pressed
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetCreateNewManipulatedImageButton()), SIGNAL(clicked()), (QObject*) this, SLOT(PipelineControllerToCreateManipulatedImage()));
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()), SIGNAL(activated (int)), (QObject*) this, SLOT(HideOrShowValue2(int))); 
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetManipulationAreaComboBox()), SIGNAL(activated (int)), (QObject*) this, SLOT(HandleSegmentationArea(int))); 
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetManipulationAreaComboBox()), SIGNAL(activated (int)), (QObject*) this, SLOT(HandleSegmentationArea(int))); 

        //to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetBoundingObjectTypeComboBox()),  SIGNAL(activated(int)), (QObject*) this, SLOT(CreateBoundingBox(int))); 

        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetInverseCheckBox(bool)));

        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->m_Value1LineEdit_2), SIGNAL(textChanged(const QString&)), (QObject*) this, SLOT(Repaste(const QString&)));
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->m_Value1LineEdit_3), SIGNAL(textChanged(const QString&)), (QObject*) this, SLOT(Repaste(const QString&)));
        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->m_Value1LineEdit_4), SIGNAL(textChanged(const QString&)), (QObject*) this, SLOT(Repaste(const QString&)));

        connect( (QObject*)(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()), SIGNAL(activated (int)), (QObject*) this, SLOT(CheckModeForPointSet(int)));

    }
}



void QmitkPixelGreyValueManipulatorComponent::Repaste(const QString& text)
{
    m_PixelGreyValueManipulatorComponentGUI->m_Value1LineEdit->setText(text);
}

void QmitkPixelGreyValueManipulatorComponent::CheckModeForPointSet(int mode)
{
    if(mode == 2)
    { 
        m_PointSet->GetContentContainer()->setShown(true);
        m_PointSet->GetMainCheckBoxContainer()->setChecked(true);
        m_PointSet->Activated();


    }
    else
    {
        m_PointSet->GetContentContainer()->setShown(false);
        m_PointSet->Deactivated();
    }
}



void QmitkPixelGreyValueManipulatorComponent::SetInverseCheckBox(bool check)
{
    m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->setChecked(check);
}


/***************      HIDE OR SHOW      ***************/
void QmitkPixelGreyValueManipulatorComponent::HideOrShow()
{
    //the entir component
    m_PixelGreyValueManipulatorComponentGUI->GetPixelManipulatorContentGroupBox()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetPixelGreyValueManipulatorGroupBox()->isChecked());

    //the TreeNodeSelector
    if(m_ShowSelector)
    {
        m_PixelGreyValueManipulatorComponentGUI->GetImageContent()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()->isChecked());
    }
    else
    {
        m_PixelGreyValueManipulatorComponentGUI->GetImageContent()->setShown(false);
    }
    m_PixelGreyValueManipulatorComponentGUI->GetBaseThresholdGroupBox()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetShowBaseThresholdGroupBox()->isChecked());
    m_PixelGreyValueManipulatorComponentGUI->GetSegmentationContent()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelectorGroupBox()->isChecked());
}

/***************     IMAGE SELECTED     ***************/
void QmitkPixelGreyValueManipulatorComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
    m_SelectedItem = imageIt;
    mitk::DataTreeFilter::Item* currentItem(NULL);
    if(m_PixelGreyValueManipulatorComponentGUI)
    {
        if(mitk::DataTreeFilter* filter = m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->GetFilter())
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
    if(m_PixelGreyValueManipulatorComponentGUI)
    {
        mitk::DataTreeFilter* filter = m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->GetFilter();
        m_MitkImageIterator = filter->GetIteratorToSelectedItem();

        if(m_MitkImageIterator.GetPointer())
        {
            m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
        }
    }

    if(m_PixelGreyValueManipulatorComponentGUI != NULL)
    {

        for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
        {
            QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
            if (functionalityComponent != NULL)
                functionalityComponent->ImageSelected(m_SelectedItem);
        }
    }
    TreeChanged();
}

/*************** SEGMENTATION SELECTED  ***************/
void QmitkPixelGreyValueManipulatorComponent::SegmentationSelected(const mitk::DataTreeFilter::Item * segmentationIt)
{
    mitk::DataTreeFilter::Item* currentItem(NULL);
    if(m_PixelGreyValueManipulatorComponentGUI)
    {
        if(mitk::DataTreeFilter* filter = m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter())
        {
            if(segmentationIt)
            {
                currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( segmentationIt->GetNode() ) );
            }
        }
    }
    if(currentItem)
    {
        currentItem->SetSelected(true);
    }

    if(m_PixelGreyValueManipulatorComponentGUI)
    {
        mitk::DataTreeFilter* filter = m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter();

        mitk::DataTreeIteratorClone segmentationIterator = filter->GetIteratorToSelectedItem();

        if(segmentationIterator.GetPointer())
        {
            m_Segmentation = static_cast<mitk::Image*> (segmentationIterator->Get()->GetData());
        }
    }

}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkPixelGreyValueManipulatorComponent::CreateControlWidget(QWidget* parent)
{
    m_PixelGreyValueManipulatorComponentGUI = new QmitkPixelGreyValueManipulatorComponentGUI(parent);
    m_GUI = m_PixelGreyValueManipulatorComponentGUI;
    m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
    m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->SetDataTree(GetDataTreeIterator());

    //m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->insertItem("-- Segmentation / Mask --", 0);

    if(!m_ShowSelector)
    {
        m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()->setShown(false);
    }
    m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));
    m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithBoolProperty<mitk::Image>("segmentation"));

    CreatePointSet();
    m_CheckBoxChecked = m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked();

    ////disable global operationmodes that only work for the whole image at the beginning
    //m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->Get

    return m_PixelGreyValueManipulatorComponentGUI;
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkPixelGreyValueManipulatorComponent::GetContentContainer()
{
    return m_PixelGreyValueManipulatorComponentGUI->GetPixelManipulatorContentGroupBox();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkPixelGreyValueManipulatorComponent::GetMainCheckBoxContainer()
{
    return m_PixelGreyValueManipulatorComponentGUI->GetPixelGreyValueManipulatorGroupBox();
}

/*************** CREATE SEEDPOINT WIDGET **************/
void QmitkPixelGreyValueManipulatorComponent::CreatePointSet()
{
    m_PointSet = new QmitkSeedPointSetComponent(GetParent(), GetFunctionalityName(), GetMultiWidget(), m_DataIt);
    m_PointSet->CreateControlWidget(m_PixelGreyValueManipulatorComponentGUI);
    m_AddedChildList.push_back(m_PointSet);

    m_PixelGreyValueManipulatorComponentGUI->layout()->add(m_PointSet->GetGUI());
    m_PointSet->CreateConnections();
    m_PixelGreyValueManipulatorComponentGUI->repaint();
    m_PointSet->GetContentContainer()->setShown(false);
    m_PointSet->Deactivated();
}


/************** SET SELECTOR VISIBILITY ***************/
void QmitkPixelGreyValueManipulatorComponent::SetSelectorVisibility(bool visibility)
{
    if(m_PixelGreyValueManipulatorComponentGUI)
    {
        m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()->setShown(visibility);
    }
    m_ShowSelector = visibility;
}


/***************        ACTIVATED       ***************/
void QmitkPixelGreyValueManipulatorComponent::Activated()
{
    m_Active = true;
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
    {
        m_AddedChildList[i]->Activated();
    } 
    if(m_BoundingObjectInteractor.IsNotNull())
    {
        mitk::GlobalInteraction::GetInstance()->AddInteractor(m_BoundingObjectInteractor);
    }
}

/***************       DEACTIVATED      ***************/
void QmitkPixelGreyValueManipulatorComponent::Deactivated()
{
    m_Active = false;
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
    {
        m_AddedChildList[i]->Deactivated();
    } 
    if(m_BoundingObjectInteractor)
    {
        mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_BoundingObjectInteractor);
    }
}

/***************      SET THRESHOLD     ***************/
void QmitkPixelGreyValueManipulatorComponent::SetThreshold(const QString& threshold)
{
    m_PixelGreyValueManipulatorComponentGUI->SetThreshold(atoi(threshold));
}

///********** SHOW PIXEL MANIPULATOR CONTENT **********/
void QmitkPixelGreyValueManipulatorComponent::ShowPixelGreyValueManipulatorContent(bool)
{
    m_PixelGreyValueManipulatorComponentGUI->GetPixelManipulatorContentGroupBox()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetPixelGreyValueManipulatorGroupBox()->isChecked());
}

///**************    SHOW IMAGE CONTENT   *************/
void QmitkPixelGreyValueManipulatorComponent::ShowImageContent(bool)
{
    m_PixelGreyValueManipulatorComponentGUI->GetImageContent()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetSelectDataGroupBox()->isChecked());
}

///**************   HIDE OR SHOW VALUE "  *************/
void QmitkPixelGreyValueManipulatorComponent::HideOrShowValue2(int /*index*/)
{ 
    if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem() == 2)
    {
        m_PixelGreyValueManipulatorComponentGUI->GetValue2GroupBox()->setShown(true);
    }
    else if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem() != 2)
    {
        m_PixelGreyValueManipulatorComponentGUI->GetValue2GroupBox()->setShown(false);
    }
}


///************   HANDLE SEGMENTATION AREA  ***********/
void QmitkPixelGreyValueManipulatorComponent::HandleSegmentationArea(int area)
{ 
    m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelectorGroupBox()->setChecked(false);
    switch(area)
    {
    case 0: 
        //nothing selected
        if(m_BoundingObjectInteractor)
        {
            mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_BoundingObjectInteractor);
        }
        break;
    case 1: 
        //whole image
        if(m_BoundingObjectInteractor)
        {
            mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_BoundingObjectInteractor);
        }
        m_PixelGreyValueManipulatorComponentGUI->GetSegmentationContent()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelectorGroupBox()->isChecked());
        break;
    case 2: 
        if(m_BoundingObjectInteractor)
        {
            mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_BoundingObjectInteractor);
        }
        //segmentation
        m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelectorGroupBox()->setChecked(true);
        m_PixelGreyValueManipulatorComponentGUI->GetSegmentationContent()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelectorGroupBox()->isChecked());
        break;
    case 3:
        //bounding object
        m_PixelGreyValueManipulatorComponentGUI->GetSegmentationContent()->setShown(m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelectorGroupBox()->isChecked());
        break;
    default:
        if(m_BoundingObjectInteractor)
        {
            mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_BoundingObjectInteractor);
        }
        break;
    }
}

///***************  CREATE BOUNDING BOX   *************/
void QmitkPixelGreyValueManipulatorComponent::CreateBoundingBox(int boundingObjectType)
{
    switch(boundingObjectType)
    {
    case 0: 
        m_BoundingObject = mitk::Cuboid::New();
        break;
    case 1: 
        m_BoundingObject = mitk::Cuboid::New();
        break;
    case 2: 
        m_BoundingObject = mitk::Cone::New();
        break;
    case 3:
        m_BoundingObject = mitk::Cylinder::New();
        break;
    case 4:
        m_BoundingObject = mitk::Ellipsoid::New();
        break;
    case 5:
        m_BoundingObject = mitk::ExtrudedContour::New();
        break;
    default:
        m_BoundingObject = mitk::Cuboid::New();
        break;
    }




    if(!m_BoundingObjectGroup)
    {
        m_BoundingObjectGroup = mitk::BoundingObjectGroup::New();
        m_BoundingObjectGroup->SetCSGMode(mitk::BoundingObjectGroup::Union);
    }

    m_BoundingObjectGroup->AddBoundingObject(m_BoundingObject);
    m_BoundingObjectGroup->UpdateOutputInformation();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    ////    if(!m_BoundingObjectExistingFlag)
    ////  {
    AddBoundingObjectToNode();
    //// }
    ////else
    ////{
    ////    m_BoundingObjectNode->SetData( m_BoundingObjectGroup );
    ////    mitk::Point3D currentCrossPosition = m_MultiWidget->GetCrossPosition();
    ////    m_BoundingObjectGroup->GetGeometry()->Translate(currentCrossPosition.GetVectorFromOrigin());
    ////    //m_BoundingObjectNode->SetData( m_BoundingObject );
    ////    //mitk::Point3D currentCrossPosition = m_MultiWidget->GetCrossPosition();
    ////    //m_BoundingObject->GetGeometry()->Translate(currentCrossPosition.GetVectorFromOrigin());
    ////}
    //mitk::GlobalInteraction::GetInstance()->AddInteractor(m_BoundingObjectInteractor);

    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_BoundingObjectInteractor);
    m_BoundingObjectExistingFlag = true;



    //mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    //if(!m_BoundingObjectExistingFlag)
    //{
    //	AddBoundingObjectToNode();
    //}
    //else
    //{
    //	m_BoundingObjectNode->SetData( m_BoundingObject );
    //	mitk::Point3D currentCrossPosition = m_MultiWidget->GetCrossPosition();
    //	m_BoundingObject->GetGeometry()->Translate(currentCrossPosition.GetVectorFromOrigin());
    //}
    //mitk::GlobalInteraction::GetInstance()->AddInteractor(m_BoundingObjectInteractor);

    //m_BoundingObjectExistingFlag = true;

}

///************* ADD BOUNDING BOX TO NODE  ************/
void QmitkPixelGreyValueManipulatorComponent::AddBoundingObjectToNode(/*mitk::DataTreeIteratorClone& iterToNode*/)
{
    mitk::DataTreeNode::Pointer boundingNode = mitk::DataTreeNode::New(); 
    boundingNode = mitk::DataTreeNode::New(); 
    mitk::Point3D currentCrossPosition = m_MultiWidget->GetCrossPosition();
    m_BoundingObject->GetGeometry()->Translate(currentCrossPosition.GetVectorFromOrigin());
    mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties( boundingNode );
    boundingNode->SetData( m_BoundingObject );
    boundingNode->SetProperty( "name", new mitk::StringProperty(NAMEFORBOUNDINGOBJECT) );
    boundingNode->SetProperty( "color", new mitk::ColorProperty(0.1, 0.57, 0.04) );
    boundingNode->SetProperty( "opacity", new mitk::FloatProperty(0.4) );
    boundingNode->SetProperty( "layer", new mitk::IntProperty(99) ); // arbitrary, copied from segmentation functionality
    boundingNode->SetProperty( "selected",  new mitk::BoolProperty(true) );

    boundingNode->SetVisibility(true);
    mitk::DataTreeIteratorClone iteratorBoundingObject = GetDataTreeIterator();
    iteratorBoundingObject->Add(boundingNode);

    m_BoundingObjectInteractor = mitk::AffineInteractor::New( "AffineInteractions ctrl-drag", m_BoundingObjectNode );
    boundingNode->SetInteractor(m_BoundingObjectInteractor);
    m_BoundingObjectList.push_back(boundingNode);

    //m_BoundingObjectNode = mitk::DataTreeNode::New(); 
    //mitk::Point3D currentCrossPosition = m_MultiWidget->GetCrossPosition();
    //m_BoundingObject->GetGeometry()->Translate(currentCrossPosition.GetVectorFromOrigin());
    //mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties( m_BoundingObjectNode );
    //m_BoundingObjectNode->SetData( m_BoundingObject );
    //m_BoundingObjectNode->SetProperty( "name", new mitk::StringProperty( NAMEFORBOUNDINGOBJECT ) );
    //m_BoundingObjectNode->SetProperty( "color", new mitk::ColorProperty(0.1, 0.57, 0.04) );
    //m_BoundingObjectNode->SetProperty( "opacity", new mitk::FloatProperty(0.4) );
    //m_BoundingObjectNode->SetProperty( "layer", new mitk::IntProperty(99) ); // arbitrary, copied from segmentation functionality
    //m_BoundingObjectNode->SetProperty( "selected",  new mitk::BoolProperty(true) );

    //m_BoundingObjectNode->SetVisibility(true);
    //mitk::DataTreeIteratorClone iteratorBoundingObject = GetDataTreeIterator();
    //iteratorBoundingObject->Add(m_BoundingObjectNode);
    //   
    //m_BoundingObjectInteractor = mitk::AffineInteractor::New( "AffineInteractions ctrl-drag", m_BoundingObjectNode );
    //m_BoundingObjectNode->SetInteractor(m_BoundingObjectInteractor);

    //   m_BoundingObjectList.push_back(m_BoundingObjectNode);
}


/****PIPELINE CONTROLLER TO CREATE MANIPULATED IMAGE***/
void QmitkPixelGreyValueManipulatorComponent::PipelineControllerToCreateManipulatedImage()
{
    if(!(m_BoundingObjectExistingFlag))
    {
        CreateBoundingBox(0);
    }

    int manipulationMode;
    int manipulationArea;
    GetManipulationModeAndAreaFromGUI(manipulationMode, manipulationArea);

    m_ManipulationMode = manipulationMode;
    m_ManipulationArea = manipulationArea;

    bool boolSegmentation = false;
    if(manipulationArea == 2)
    {
        boolSegmentation = true;
    }

    switch(manipulationMode)
    {
    case 0: 
        //nothing selected
        break;
    case 1: 
        LinearShift();
        break;
    case 2: 
        GradientShift();
        break;
    case 3:
        ChangeGreyValue();
        break;
    case 4:
        LightenOrShade();
        break;
    default:
        break;
    }
}


/*************** GET MANIPULATION MODE  ***************/
void QmitkPixelGreyValueManipulatorComponent::GetManipulationModeAndAreaFromGUI(int & manipulationMode, int & manipulationArea)
{
    manipulationMode = m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem(); // 0 = Nothing selected, 1 = linear shift, 2 = gradient shift, 3 = change grey value, 4 = lighten / shade
    manipulationArea = m_PixelGreyValueManipulatorComponentGUI->GetManipulationAreaComboBox()->currentItem(); // 0 = Nothing selected, 1 = on Image, 2 = on Segmentation, 3 = inside BoundingObject

    //default is Mode = linear, Area = Segmentation
    if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationModeComboBox()->currentItem() == 0)
    {
        manipulationMode = 1;
    }

    if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationAreaComboBox()->currentItem() == 0)
    {
        manipulationArea = 1;
    }
}

/*************** GET MANIPULATION AREA  ***************/
void QmitkPixelGreyValueManipulatorComponent::GetManipulationValueFromGUI(int & value1, int & value2, int & baseValue)
{
    value1 = atoi(m_PixelGreyValueManipulatorComponentGUI->GetValue1LineEdit()->text());
    value2 = atoi(m_PixelGreyValueManipulatorComponentGUI->GetValue2LineEdit()->text());
    baseValue = m_PixelGreyValueManipulatorComponentGUI->GetBaseThreshold();

}


/***************      LINEAR SHIFT      ***************/
void QmitkPixelGreyValueManipulatorComponent::LinearShift()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    mitk::Image*  image = m_MitkImage;
    AccessFixedDimensionByItk_1(image, CreateLinearShiftedImage, 3, m_Segmentation.GetPointer()); 
    QApplication::restoreOverrideCursor();
}

/***************     GRADIENT SHIFT     ***************/
void QmitkPixelGreyValueManipulatorComponent::GradientShift()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    mitk::Image*  image = m_MitkImage;
    AccessFixedDimensionByItk_1(image, CreateGradientShiftedImage, 3, m_Segmentation.GetPointer()); 
    QApplication::restoreOverrideCursor();
}

/***************   CHANGE GREY VALUE    ***************/
void QmitkPixelGreyValueManipulatorComponent::ChangeGreyValue()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    mitk::Image*  image = m_MitkImage;
    // if(m_PixelGreyValueManipulatorComponentGUI->GetManipulationAreaComboBox()->currentItem() == 3)
    // {
    //AccessFixedDimensionByItk_1(image, CreateChangedGreyValueMitkImage, 3, m_Segmentation.GetPointer()); 
    // }
    // else
    // {
    AccessFixedDimensionByItk_1(image, CreateChangedGreyValueImage, 3, m_Segmentation.GetPointer()); 
    //}
    QApplication::restoreOverrideCursor();
}

/***************   LIGHTEN OR SHADE     ***************/
void QmitkPixelGreyValueManipulatorComponent::LightenOrShade()
{
    QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
    mitk::Image*  image = m_MitkImage;
    AccessFixedDimensionByItk_1(image, CreateLightenOrShadeImage, 3, m_Segmentation.GetPointer()); 
    QApplication::restoreOverrideCursor();
}


/***************     LINEAR TEMPLATE    ***************/
template < typename TPixel, unsigned int VImageDimension >    
void QmitkPixelGreyValueManipulatorComponent::CreateLinearShiftedImage( itk::Image< TPixel, VImageDimension >* itkImage, const mitk::Image* segmentation)
{
    typedef itk::Image< TPixel, VImageDimension > ItkImageType;
    itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );

    typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
    typename ItkSegmentationImageType::Pointer itkSegmentation;

    if(segmentation != NULL)
    {
        mitk::CastToItkImage(segmentation, itkSegmentation); 
    }

    typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
    itkShiftedImage->SetRegions(itkImage->GetLargestPossibleRegion());
    itkShiftedImage->Allocate();
    itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );

    const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();

    int imageDim = 1;

    for(unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
    {
        imageDim *= imageRegion.GetSize(dimension); //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden m�ssen
    }

    int value1;
    int value2;
    int baseValue;

    GetManipulationValueFromGUI(value1, value2, baseValue);

    int baseThreshold = baseValue;
    int shiftedThresholdOne = value1;

    if(m_ManipulationArea == 2) //if manipulation shall be only on segmented parts
    {
        if(m_Segmentation.IsNotNull())
        {
            itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
            if(!(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()))//manipulate inside the segmentation
            {
                while(!(it.IsAtEnd()))
                {
                    if(itSeg.Get()!=0)
                        itShifted.Set(it.Get()+ (baseThreshold -shiftedThresholdOne));
                    else
                        itShifted.Set(it.Get());
                    ++it;
                    ++itShifted;
                    ++itSeg;

                    --imageDim;//Kontrollausgabe
                    if(imageDim % 1000 == 0)
                    {
                        std::cout<<imageDim<<std::endl;
                    }
                }//end of while
            }
            else//manipulate outside the segmentation
            {
                while(!(it.IsAtEnd()))
                {
                    if(itSeg.Get()==0)
                        itShifted.Set(it.Get()+ (baseThreshold -shiftedThresholdOne));
                    else
                        itShifted.Set(it.Get());
                    ++it;
                    ++itShifted;
                    ++itSeg;

                    --imageDim;//Kontrollausgabe
                    if(imageDim % 1000 == 0)
                    {
                        std::cout<<imageDim<<std::endl;
                    }
                }//end of while
            }
        }//if(m_Segmentation != NULL)
    }//end of manipulation on segmented area

    else if(m_ManipulationArea == 1) //manipulation shall be on entire image
    {
        while(!(it.IsAtEnd()))
        {
            itShifted.Set(it.Get()+ (baseThreshold -shiftedThresholdOne));
            ++it;
            ++itShifted;

            --imageDim;//Kontrollausgabe
            if(imageDim % 1000 == 0)
            {
                std::cout<<imageDim<<std::endl;
            }
        }//end of while
    }//end of manipulation on entire image

    else if(m_ManipulationArea == 3) //if manipulation shall be only inside bounding object
    {
        if( m_BoundingObjectGroup)
        {
            while(!(it.IsAtEnd()))
            {
                mitk::Point3D point3D;
                itkImage->TransformIndexToPhysicalPoint(it.GetIndex(),point3D);
                if(!(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()))//manipulate inside the bounding box
                {
                    if( m_BoundingObjectGroup->IsInside(point3D))
                    {
                        itShifted.Set(it.Get()+ (baseThreshold -shiftedThresholdOne));
                    }
                    else
                    {
                        itShifted.Set(it.Get());
                    }
                }
                else //manipulate outsidethe bounding box
                {
                    if(!( m_BoundingObjectGroup->IsInside(point3D)))
                    {
                        itShifted.Set(it.Get()+ (baseThreshold -shiftedThresholdOne));
                    }
                    else
                    {
                        itShifted.Set(it.Get());
                    }
                }
                ++it;
                ++itShifted;
            }//end of while
        }//if(m_BoundingObject)
    }//end of manipulation inside BoundingObject area

    AddManipulatedImageIntoTree<ItkImageType>(itkShiftedImage);
}// end of CreateLinearShiftedImage


/***************    GRADIENT TEMPLATE   ***************/
template < typename TPixel, unsigned int VImageDimension >    
void QmitkPixelGreyValueManipulatorComponent::CreateGradientShiftedImage( itk::Image< TPixel, VImageDimension >* itkImage, const mitk::Image* segmentation)
{
    typedef itk::Image< TPixel, VImageDimension > ItkImageType;
    itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );

    typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
    typename ItkSegmentationImageType::Pointer itkSegmentation;

    if(segmentation != NULL)
    {
        mitk::CastToItkImage(segmentation, itkSegmentation);
    }

    typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
    itkShiftedImage->SetRegions(itkImage->GetLargestPossibleRegion());
    itkShiftedImage->Allocate();
    itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );

    const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();

    int imageDim = 1;

    for(unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
    {
        imageDim *= imageRegion.GetSize(dimension); //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden m�ssen
    }

    int value1;
    int value2;
    int baseValue;

    GetManipulationValueFromGUI(value1, value2, baseValue);

    int baseThreshold = baseValue;
    int shiftedThresholdOne = value1;
    int shiftedThresholdTwo = value2;

    if(m_PointSet->GetPointSetNode().IsNotNull())
    {
        mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(m_PointSet->GetPointSetNode()->GetData());
        int numberOfPoints = pointSet->GetSize();

        mitk::PointSet::PointType pointOne = pointSet->GetPoint(0);
        mitk::PointSet::PointType pointTwo = pointSet->GetPoint(1);

        if(numberOfPoints == 2)
        {
            if(m_ManipulationArea == 2) //if manipulation shall be only on segmented parts
            {    

                if(m_Segmentation.IsNotNull())
                {
                    itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
                    if(!(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()))//manipulate inside the segmentation
                    {
                        while(!(it.IsAtEnd()))
                        {
                            if(itSeg.Get()!=0)//da wo segmentiert ist
                            {
                                InternalGradientShiftCalculation(shiftedThresholdOne, shiftedThresholdTwo, baseThreshold, itShifted, it, pointOne, pointTwo );
                            }
                            else
                            {
                                itShifted.Set(it.Get());
                            }
                            ++it;
                            ++itShifted;
                            ++itSeg;

                            --imageDim;//Kontrollausgabe
                            if(imageDim % 1000 == 0)
                            {
                                std::cout<<imageDim<<std::endl;
                            }
                        }//end of while
                    }
                    else//manipulate outside the segmentation
                    {
                        while(!(it.IsAtEnd()))
                        {
                            if(itSeg.Get()==0)//da wo segmentiert ist
                            {
                                InternalGradientShiftCalculation(shiftedThresholdOne, shiftedThresholdTwo, baseThreshold, itShifted, it, pointOne, pointTwo );
                            }
                            else
                            {
                                itShifted.Set(it.Get());
                            }
                            ++it;
                            ++itShifted;
                            ++itSeg;

                            --imageDim;//Kontrollausgabe
                            if(imageDim % 1000 == 0)
                            {
                                std::cout<<imageDim<<std::endl;
                            }
                        }//end of while
                    }
                }//if(m_Segmentation != NULL)

            }// end of if manipulation shall be only on segmented parts

            else if(m_ManipulationArea == 1)//if manipulation area is entier image
            {
                while(!(it.IsAtEnd()))
                {
                    InternalGradientShiftCalculation(shiftedThresholdOne, shiftedThresholdTwo, baseThreshold, itShifted, it, pointOne, pointTwo );
                    ++it;
                    ++itShifted;

                    //control output: how many Pixels are still to change
                    --imageDim;
                    if(imageDim % 1000 == 0)
                    {
                        std::cout<<imageDim<<std::endl;
                    }
                }//end of while(!(it.IsAtEnd())
            }//end of else -> manipulation area is entier image
        }// end of if (numberOfPoints == 2)

        else if(m_ManipulationArea == 3) //if manipulation shall be only inside bounding object
        {
            if( m_BoundingObjectGroup)
            {
                while(!(it.IsAtEnd()))
                {
                    mitk::Point3D point3D;
                    //itkImage->TransformIndexToPhysicalPoint(it.GetIndex(),point3D);
                    mitk::vtk2itk(it.GetIndex(),point3D);
                    m_MitkImage->GetGeometry()->IndexToWorld(point3D,point3D);

                    if(!(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()))//manipulate inside the bounding box
                    {
                        if( m_BoundingObjectGroup->IsInside(point3D))
                        {
                            InternalGradientShiftCalculation(shiftedThresholdOne, shiftedThresholdTwo, baseThreshold, itShifted, it, pointOne, pointTwo );
                        }
                        else
                        {
                            itShifted.Set(it.Get());
                        }
                    }
                    else //manipulate outsidethe bounding box
                    {
                        if(!( m_BoundingObjectGroup->IsInside(point3D)))
                        {
                            InternalGradientShiftCalculation(shiftedThresholdOne, shiftedThresholdTwo, baseThreshold, itShifted, it, pointOne, pointTwo );
                        }
                        else
                        {
                            itShifted.Set(it.Get());
                        }
                    }
                    ++it;
                    ++itShifted;
                }//end of while
            }//if(m_BoundingObject)
        }//end of manipulation inside BoundingObject area

    } //end of if(m_PointSetNode != NULL)
    AddManipulatedImageIntoTree<ItkImageType>(itkShiftedImage);

}// end of CreateGradientShiftedImage


/**************INTERNAL GRADIENT SHIFT CALC************/
template < typename ItkImageType >  
void QmitkPixelGreyValueManipulatorComponent::InternalGradientShiftCalculation(int & shiftedThresholdOne, int & shiftedThresholdTwo, int & normalThreshold, itk::ImageRegionIterator<ItkImageType> & itShifted, itk::ImageRegionConstIterator<ItkImageType> & it, mitk::PointSet::PointType & pointOne, mitk::PointSet::PointType & pointTwo)
{
    typename ItkImageType::IndexType thisPointIndex = it.GetIndex();
    //to change index to worldcoordinates
    mitk::Image* image = static_cast<mitk::Image*>(m_MitkImage);
    mitk::Point3D thisPoint3DWorldIndex;
    mitk::Point3D thisPoint3DWorldChangedIndex;

    thisPoint3DWorldIndex[0]=thisPointIndex[0];
    thisPoint3DWorldIndex[1]=thisPointIndex[1];
    thisPoint3DWorldIndex[2]=thisPointIndex[2];

    image->GetGeometry(0)->IndexToWorld(thisPoint3DWorldIndex, thisPoint3DWorldChangedIndex);

    double x1 = pointOne[0];
    double y1 = pointOne[1];
    double z1 = pointOne[2];

    double x3 = thisPoint3DWorldChangedIndex[0];
    double y3 = thisPoint3DWorldChangedIndex[1];
    double z3 = thisPoint3DWorldChangedIndex[2]; 

    mitk::Vector3D pMinusP1;
    pMinusP1[0]= x3 - x1;
    pMinusP1[1]= y3 - y1;
    pMinusP1[2]= z3 - z1;

    mitk::Vector3D v;
    v = pointTwo-pointOne;
    mitk::ScalarType vq;
    vq = 1.0/v.GetSquaredNorm();

    mitk::ScalarType f;
    f=v * (pMinusP1)*vq;

    double gradientShiftThresh; 
    gradientShiftThresh = shiftedThresholdOne * (1-f) + shiftedThresholdTwo * f;

    double min;
    double max;
    if (shiftedThresholdOne < shiftedThresholdTwo)
    {
        min = shiftedThresholdOne;
        max = shiftedThresholdTwo;
    }
    else 
    {
        max = shiftedThresholdOne;
        min = shiftedThresholdTwo;
    }

    if(gradientShiftThresh < min)
    {
        gradientShiftThresh = min;
    }
    if(gradientShiftThresh > max)
    {
        gradientShiftThresh = max;
    }

    double shiftValue = normalThreshold - gradientShiftThresh;
    itShifted.Set((typename ItkImageType::PixelType)(it.Get() + shiftValue));
}//end of InternalGradientShiftCalculation


/**************CHANGED GREY VALUE TEMPLATE*************/
template < typename TPixel, unsigned int VImageDimension >    
void QmitkPixelGreyValueManipulatorComponent::CreateChangedGreyValueImage( itk::Image< TPixel, VImageDimension >* itkImage, const mitk::Image* segmentation)
{
    m_CheckBoxChecked = m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked();
    std::cout<<"CheckBox: "<<m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()<<std::endl;
    typedef itk::Image< TPixel, VImageDimension > ItkImageType;
    itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );

    typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
    typename ItkSegmentationImageType::Pointer itkSegmentation;

    if(segmentation != NULL)
    {
        mitk::CastToItkImage(segmentation, itkSegmentation);
    }

    typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
    itkShiftedImage->SetRegions(itkImage->GetLargestPossibleRegion());
    itkShiftedImage->Allocate();
    itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );

    const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();

    int imageDim = 1;

    for(unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
    {
        imageDim *= imageRegion.GetSize(dimension); //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden m�ssen
    }

    int value1;
    int value2;
    int baseValue;

    GetManipulationValueFromGUI(value1, value2, baseValue);

    int pixelChangeValue = value1;

    if(m_ManipulationArea == 2) //if manipulation shall be only on segmented parts
    {
        if(m_Segmentation.IsNotNull())
        {
            itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
            std::cout<<"CheckBox: "<<m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()<<std::endl;

            if(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked())
            {
                //if(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked())//manipulate outside the segmentation
                //{
                while(!(it.IsAtEnd()))
                {
                    if(!(itSeg.IsAtEnd()))
                    {


                    if(itSeg.Get()== 0)
                    {
                        itShifted.Set(pixelChangeValue);
                    }

                    else
                    {
                        itShifted.Set(it.Get());
                    }
                    }//if not itSeg.IsAtEnd
                    ++it;
                    ++itShifted;
                    ++itSeg;

                    --imageDim;//Kontrollausgabe
                    if(imageDim % 1000 == 0)
                    {
                        //std::cout<<imageDim<<std::endl;
                    }
                   
                }//end of while
            }//end of if checked

            else //manipulate inside the segmentation
            {
                while(!(it.IsAtEnd()))
                {
                    if(itSeg.Get())
                    {
                        itShifted.Set(pixelChangeValue);
                    }
                    else
                    {
                        itShifted.Set(it.Get());
                    }
                    ++it;
                    ++itShifted;
                    ++itSeg;

                    --imageDim;//Kontrollausgabe
                    if(imageDim % 1000 == 0)
                    {
                        //std::cout<<imageDim<<std::endl;
                    }
                }//end of while
            }//end of else ! checked
        }//if(m_Segmentation != NULL)
    }//end of manipulation on segmented area

    else if(m_ManipulationArea == 1)//manipulation shall be on entire image
    {
        while(!(it.IsAtEnd()))
        {
            itShifted.Set(pixelChangeValue); //set the new pixelvalue

            ++it;
            ++itShifted;

            //control output: how many Pixels are still to change
            --imageDim;
            if(imageDim % 1000 == 0)
            {
                std::cout<<imageDim<<std::endl;
            }
        }//end of while (!(it.IsAtEnd()))
    }//end of manipulation on entire image

    else if(m_ManipulationArea == 3) //if manipulation shall be only inside bounding object
    {
        if( m_BoundingObjectGroup)
        {
            std::cout<<"CheckBox: "<<m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()<<std::endl;
            if(!(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()))//manipulate inside the bounding box
            {
                while(!(it.IsAtEnd()))
                {
                    mitk::Point3D point3D;
                    //itkImage->TransformIndexToPhysicalPoint(it.GetIndex(),point3D);
                    mitk::vtk2itk(it.GetIndex(),point3D);
                    m_MitkImage->GetGeometry()->IndexToWorld(point3D,point3D);
                    if( m_BoundingObjectGroup->IsInside(point3D))
                    {
                        itShifted.Set(pixelChangeValue);
                    }
                    else
                    {
                        itShifted.Set(it.Get());
                    }
                    ++it;
                    ++itShifted;
                    --imageDim;
                    if(imageDim % 1000 == 0)
                    {
                        //std::cout<<imageDim<<std::endl;
                    }
                }//end of while
            }//end of !checked

            else //manipulate outside the bounding box
            {
                while(!(it.IsAtEnd()))
                {
                    mitk::Point3D point3D;
                    //itkImage->TransformIndexToPhysicalPoint(it.GetIndex(),point3D);
                    mitk::vtk2itk(it.GetIndex(),point3D);
                    m_MitkImage->GetGeometry()->IndexToWorld(point3D,point3D);
                    if( m_BoundingObjectGroup->IsInside(point3D))
                    {
                        itShifted.Set(it.Get());
                        //itShifted.Set(pixelChangeValue);
                    }
                    else
                    {
                        itShifted.Set(pixelChangeValue);//itShifted.Set(it.Get());
                    }
                    ++it;
                    ++itShifted;
                    --imageDim;
                    if(imageDim % 1000 == 0)
                    {
                        //std::cout<<imageDim<<std::endl;
                    }
                }//end of while
            }//end of checked
        }//if(m_BoundingObject)
    }//end of manipulation inside BoundingObject area

    AddManipulatedImageIntoTree<ItkImageType>(itkShiftedImage);

}// end of CHANGED GREY VALUE IMAGE


/**************CHANGED GREY VALUE MITK Image TEMPLATE*************/
//Only for using BoundingBoxes
//template < typename TPixel, unsigned int VImageDimension >    
//void QmitkPixelGreyValueManipulatorComponent::CreateChangedGreyValueMitkImage( mitk::Image * mitkImage, const mitk::Image* segmentation)
//{
//	m_CheckBoxChecked = m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked();
//	std::cout<<"CheckBox: "<<m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()<<std::endl;
//	//typedef itk::Image< TPixel, VImageDimension > ItkImageType;
//	//itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );
//
//	//typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
//	//typename ItkSegmentationImageType::Pointer itkSegmentation;
//
//	//if(segmentation != NULL)
//	//{
//	//	mitk::CastToItkImage(segmentation, itkSegmentation);
//	//}
//
//	//typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
//  mitk::Image::Pointer mitkShiftedImage = mitk::Image::New();
//
//  //mitk::Image::Pointer segmentation = mitk::Image::New();
//  //segmentation->Initialize( pixelType, image->GetDimension(), image->GetDimensions() );
//  //mitk::PixelType pixelType( typeid(SEGMENTATION_DATATYPE) );
//	mitkShiftedImage->Initialize(mitkImage->GetLargestPossibleRegion());
//	//itkShiftedImage->Allocate();
//	itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );
//
//	const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();
//
//	int imageDim = 1;
//
//	for(unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
//	{
//		imageDim *= imageRegion.GetSize(dimension); //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden m�ssen
//	}
//
//	int value1;
//	int value2;
//	int baseValue;
//
//	GetManipulationValueFromGUI(value1, value2, baseValue);
//
//	int pixelChangeValue = value1;
//
//	
//	
//		if(m_BoundingObject)
//		{
//			std::cout<<"CheckBox: "<<m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()<<std::endl;
//			if(!(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()))//manipulate inside the bounding box
//			{
//				while(!(it.IsAtEnd()))
//				{
//					mitk::Point3D point3D;
//					itkImage->TransformIndexToPhysicalPoint(it.GetIndex(),point3D);
//					if(m_BoundingObject->IsInside(point3D))
//					{
//						itShifted.Set(pixelChangeValue);
//					}
//					else
//					{
//						itShifted.Set(it.Get());
//					}
//					++it;
//					++itShifted;
//					--imageDim;
//					if(imageDim % 1000 == 0)
//					{
//						//std::cout<<imageDim<<std::endl;
//					}
//				}//end of while
//			}//end of !checked
//
//			else //manipulate outsidethe bounding box
//			{
//				while(!(it.IsAtEnd()))
//				{
//					mitk::Point3D point3D;
//					itkImage->TransformIndexToPhysicalPoint(it.GetIndex(),point3D);
//         
//					if(!(m_BoundingObject->IsInside(point3D)))
//					{
//						itShifted.Set(pixelChangeValue);
//					}
//					else
//					{
//						itShifted.Set(it.Get());
//					}
//					++it;
//					++itShifted;
//					--imageDim;
//					if(imageDim % 1000 == 0)
//					{
//						//std::cout<<imageDim<<std::endl;
//					}
//				}//end of while
//			}//end of checked
//		}//if(m_BoundingObject)
//	
//	AddManipulatedImageIntoTree<ItkImageType>(itkShiftedImage);
//
//}// end of CHANGED GREY VALUE IMAGE


/************** LIGHTEN OR SHADE TEMPLATE *************/
template < typename TPixel, unsigned int VImageDimension >    
void QmitkPixelGreyValueManipulatorComponent::CreateLightenOrShadeImage( itk::Image< TPixel, VImageDimension >* itkImage, const mitk::Image* segmentation)
{
    typedef itk::Image< TPixel, VImageDimension > ItkImageType;
    itk::ImageRegionConstIterator<ItkImageType> it(itkImage, itkImage->GetLargestPossibleRegion() );

    typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
    typename ItkSegmentationImageType::Pointer itkSegmentation;

    if(segmentation != NULL)
    {
        mitk::CastToItkImage(segmentation, itkSegmentation);
    }

    typename ItkImageType::Pointer itkShiftedImage = ItkImageType::New();
    itkShiftedImage->SetRegions(itkImage->GetLargestPossibleRegion());
    itkShiftedImage->Allocate();
    itk::ImageRegionIterator<ItkImageType> itShifted(itkShiftedImage, itkShiftedImage->GetLargestPossibleRegion() );

    const typename ItkImageType::RegionType & imageRegion = itkImage->GetLargestPossibleRegion();

    int imageDim = 1;

    for(unsigned int dimension = 0; dimension < VImageDimension; ++dimension)
    {
        imageDim *= imageRegion.GetSize(dimension); //Anzahl der Pixel des Bildes die in unten stehender while-Schleife durchlaufen werden m�ssen
    }

    int value1;
    int value2;
    int baseValue;

    GetManipulationValueFromGUI(value1, value2, baseValue);

    int pixelChangeValue = value1;

    if(m_ManipulationArea == 2) //if manipulation shall be only on segmented parts
    {
        if(m_Segmentation.IsNotNull())
        {
            itk::ImageRegionConstIterator<ItkSegmentationImageType> itSeg(itkSegmentation, itkSegmentation->GetLargestPossibleRegion());
            if(!(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()))//manipulate inside the segmentation
            {
                while(!(it.IsAtEnd()))
                {
                    if(itSeg.Get()!=0)// if there is a segmentation 
                    {
                        itShifted.Set(it.Get() + pixelChangeValue);//add the new pixel value on the old one
                    }
                    else // else leave the value the old one
                    {
                        itShifted.Set(it.Get());
                    }
                    ++it;
                    ++itShifted;
                    ++itSeg;

                    --imageDim;//Kontrollausgabe
                    if(imageDim % 1000 == 0)
                    {
                        std::cout<<imageDim<<std::endl;
                    }
                }//end of while
            }
            else//manipulate outside the segmentation
            {
                while(!(it.IsAtEnd()))
                {
                    if(itSeg.Get()==0)// if there is a segmentation 
                    {
                        itShifted.Set(it.Get() + pixelChangeValue);//add the new pixel value on the old one
                    }
                    else // else leave the value the old one
                    {
                        itShifted.Set(it.Get());
                    }
                    ++it;
                    ++itShifted;
                    ++itSeg;

                    --imageDim;//Kontrollausgabe
                    if(imageDim % 1000 == 0)
                    {
                        std::cout<<imageDim<<std::endl;
                    }
                }//end of while
            }
        }//if(m_Segmentation != NULL)
    }//end of manipulation on segmented area

    else if(m_ManipulationArea == 1)//manipulation shall be on entire image
    {
        while(!(it.IsAtEnd()))
        {
            itShifted.Set(it.Get() + pixelChangeValue);//add the new pixel value on the old one

            ++it;
            ++itShifted;

            //control output: how many Pixels are still to change
            --imageDim;
            if(imageDim % 1000 == 0)
            {
                std::cout<<imageDim<<std::endl;
            }
        }//end of while (!(it.IsAtEnd()))
    }//end of manipulation on entire image

    else if(m_ManipulationArea == 3) //if manipulation shall be only inside bounding object
    {
        if( m_BoundingObjectGroup)
        {
            while(!(it.IsAtEnd()))
            {
                mitk::Point3D point3D;
                //itkImage->TransformIndexToPhysicalPoint(it.GetIndex(),point3D);
                mitk::vtk2itk(it.GetIndex(),point3D);
                m_MitkImage->GetGeometry()->IndexToWorld(point3D,point3D);

                if(!(m_PixelGreyValueManipulatorComponentGUI->GetInverseCheckBox()->isChecked()))//manipulate inside the bounding box
                {
                    if( m_BoundingObjectGroup->IsInside(point3D))
                    {
                        itShifted.Set(it.Get() + pixelChangeValue);//add the new pixel value on the old one
                    }
                    else
                    {
                        itShifted.Set(it.Get());
                    }
                }
                else //manipulate outsidethe bounding box
                {
                    if(!( m_BoundingObjectGroup->IsInside(point3D)))
                    {
                        itShifted.Set(it.Get() + pixelChangeValue);//add the new pixel value on the old one
                    }
                    else
                    {
                        itShifted.Set(it.Get());
                    }
                }
                ++it;
                ++itShifted;

            }//end of while
        }//if(m_BoundingObject)
    }//end of manipulation inside BoundingObject area

    AddManipulatedImageIntoTree<ItkImageType>(itkShiftedImage);

}// end of CreateLightenOrShadeImage

template < typename ItkImageType >  
void QmitkPixelGreyValueManipulatorComponent::AddManipulatedImageIntoTree(typename ItkImageType::Pointer & itkShiftedImage)
{
    if(m_SegmentedShiftResultNode.IsNotNull())
    {
        m_SegmentedShiftResultNode->SetData(NULL);
    }

    m_PixelChangedImage = mitk::Image::New(); 
    mitk::CastToMitkImage(itkShiftedImage, m_PixelChangedImage);
    m_PixelChangedImage->SetGeometry(static_cast<mitk::Geometry3D*>(m_MitkImage->GetGeometry()->Clone().GetPointer()));
    m_ItNewBuildSeg = GetDataTreeIterator();

    mitk::DataTreeIteratorClone  selectedIterator; 

    if(m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter()->GetSelectedItem() != NULL)
    {
        selectedIterator = m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter()->GetIteratorToSelectedItem(); 
        m_SegmentationNode = m_PixelGreyValueManipulatorComponentGUI->GetSegmentationSelector()->GetFilter()->GetSelectedItem()->GetNode();
    }
    else
    {
        selectedIterator = m_PixelGreyValueManipulatorComponentGUI->GetTreeNodeSelector()->GetFilter()->GetIteratorToSelectedItem(); 
    }

    //check if current node is a image
    if ( m_SegmentationNode.IsNotNull() ) 
    {
        std::string nodeName;

        if( m_SegmentationNode->GetPropertyList()!=NULL )
        {
            if( m_SegmentationNode->GetPropertyList()->GetProperty("name")!= NULL )
            {
                nodeName = m_SegmentationNode->GetPropertyList()->GetProperty("name")->GetValueAsString();
            }//end of if( m_SegmentationNode->GetPropertyList()->GetProperty("name") != NULL)
        }//end of if( m_SegmentationNode->GetPropertyList() != NULL)
    }//end of if(m_SegmentationNode.IsNotNull())
    ++m_PixelChangedImageCounter;
    std::ostringstream buffer;
    buffer << m_PixelChangedImageCounter;
    std::string sPName = "PixelChanged Image " + buffer.str();
    m_PixelChangedImageNode = mitk::DataTreeNode::New();//m_GradientShiftedImageNode = mitk::DataTreeNode::New();
    m_PixelChangedImageNode->SetData(m_PixelChangedImage);
    mitk::DataTreeNodeFactory::SetDefaultImageProperties(m_PixelChangedImageNode);
    m_PixelChangedImageNode->SetProperty("name", new mitk::StringProperty(sPName ) );
    m_PixelChangedImageNode->SetIntProperty("layer", 2);
    selectedIterator->Add(m_PixelChangedImageNode);
}
