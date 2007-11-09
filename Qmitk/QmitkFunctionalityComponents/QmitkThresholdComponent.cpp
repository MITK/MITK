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
#include "mitkDataTreeNodeFactory.h"
#include "mitkLevelWindowProperty.h"

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

#include <mitkImageAccessByItk.h>
#include <ipPicTypeMultiplex.h>
#include <itkImageConstIteratorWithIndex.h>


/***************       CONSTRUCTOR      ***************/
QmitkThresholdComponent::QmitkThresholdComponent(QObject * parent, const char * parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget * /*mitkStdMultiWidget*/, mitk::DataTreeIteratorBase* it)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector),
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
}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox* QmitkThresholdComponent::GetImageContent()
{
	return (QGroupBox*) m_ThresholdComponentGUI->GetImageContent();
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
		//connect( (QObject*)(m_ThresholdComponentGUI->GetShowThresholdGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowThreshold(bool)));  

		//to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
		connect( (QObject*)(m_ThresholdComponentGUI->GetThresholdFinderGroupBox()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 
		connect( (QObject*)(m_ThresholdComponentGUI->GetCreateSegmentationButton()),  SIGNAL(pressed()), (QObject*) this, SLOT(CreateThresholdSegmentation())); 
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
			QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(m_SelectedItem);
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

	if(m_ShowSelector)
	{
		m_ThresholdComponentGUI->GetImageContent()->setShown(m_ThresholdComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_ThresholdComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
	}

	m_ThresholdComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));

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
	SetSliderRange();
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
	if(m_ThresholdComponentGUI->GetDeleteImageIfDeactivatedCheckBox()->isChecked())
	{
		DeleteThresholdNode();
	}
}

///************ SHOW THRESHOLD FINDER CONTENT ***********/
void QmitkThresholdComponent::ShowThresholdFinderContent(bool)
{
	//m_ThresholdComponentGUI->GetShowThresholdGroupBox()->setShown(m_ThresholdComponentGUI->GetThresholdFinderGroupBox()->isChecked());
	m_ThresholdComponentGUI->GetContainerContent()->setShown(m_ThresholdComponentGUI->GetThresholdFinderGroupBox()->isChecked());


	if(m_ShowSelector)
	{
		m_ThresholdComponentGUI->GetSelectDataGroupBox()->setShown(m_ThresholdComponentGUI->GetThresholdFinderGroupBox()->isChecked());
	}

	//ShowThreshold();
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkThresholdComponent::ShowImageContent(bool)
{
	m_ThresholdComponentGUI->GetImageContent()->setShown(m_ThresholdComponentGUI->GetSelectDataGroupBox()->isChecked());

	if(m_ShowSelector)
	{
		m_ThresholdComponentGUI->GetImageContent()->setShown(m_ThresholdComponentGUI->GetSelectDataGroupBox()->isChecked());
	}
	else
	{
		m_ThresholdComponentGUI->GetSelectDataGroupBox()->setShown(m_ShowSelector);
	}
}

///***************      SHOW THRESHOLD     **************/
void QmitkThresholdComponent::ShowThreshold(bool)
{
	if(m_ThresholdImageNode)
	{
		if(m_Active == true)
		{
			m_ThresholdImageNode->SetProperty("visible", new mitk::BoolProperty((m_ThresholdComponentGUI->GetThresholdFinderGroupBox()->isChecked())) );
		}
		else
		{
			if(m_ThresholdComponentGUI->GetDeleteImageIfDeactivatedCheckBox()->isChecked())
			{
				m_ThresholdImageNode->SetProperty("visible", new mitk::BoolProperty((false)) );
			}
		}
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
				int min = (int) currentImage->GetScalarValueMin();
				int max = (int) currentImage->GetScalarValueMaxNoRecompute();
				//int realMax = currentImage->GetScalarValueMax();
				if(min < -32000)
				{
					min = (int) currentImage->GetScalarValue2ndMin();
					max = (int) currentImage->GetScalarValue2ndMaxNoRecompute();
				}
				m_ThresholdComponentGUI->GetThresholdInputSlider()->setMinValue(min);
				m_ThresholdComponentGUI->GetThresholdInputSlider()->setMaxValue(max);
				/*m_ThresholdComponentGUI->GetThresholdInputSlider()->setMinValue((int)currentImage->GetScalarValueMin());
				m_ThresholdComponentGUI->GetThresholdInputSlider()->setMaxValue((int)currentImage->GetScalarValueMaxNoRecompute());*/
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

/*************CREAET THRESHOLD SEGMENTATION************/
void QmitkThresholdComponent::CreateThresholdSegmentation()
{

	//mitk::DataTreeNode::Pointer segmentationNode = m_Controls->m_ToolDataSelectionBox->GetToolManager()->GetReferenceData(0);
	mitk::DataTreeNode::Pointer segmentationNode = mitk::DataTreeNode::New();
	segmentationNode->SetData(m_Node->GetData());
	m_MitkImage = dynamic_cast<mitk::Image*>(m_Node->GetData());
	if  
		(segmentationNode.IsNotNull())
	{
		mitk::StringProperty::Pointer nameProp = new mitk::StringProperty("TH segmentation" );
		segmentationNode->SetProperty( "name", nameProp );
		mitk::BoolProperty::Pointer thresholdBasedSegmentationProp = new mitk::BoolProperty(true);
		segmentationNode->SetProperty( "segmentation", thresholdBasedSegmentationProp );

		segmentationNode->SetColor(1.0,0.0,0.0);
		segmentationNode->SetOpacity(.25);

		mitk::Image::Pointer image = dynamic_cast<mitk::Image*>( segmentationNode->GetData() );
		if (image.IsNotNull())
		{


			// ask the user about an organ type and name, add this information to the image's (!) propertylist

			// create a new image of the same dimensions and smallest possible pixel type
			AccessFixedDimensionByItk_2(m_MitkImage, /*the actual selected image */
				ThresholdSegmentation, /*called template-method */
				3, /*dimension */
				image, /*passed segmentation */
				this /* the QmitkSurfaceCreator-object */);


			//mitk::DataTreeNode::Pointer segmentation = CreateEmptySegmentationNode(image);
			if (!m_ThresholdSegmentationImage) return; // could be aborted by user


			segmentationNode->SetData(m_ThresholdSegmentationImage);
			mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
			iteratorClone->GoToBegin();
			while ( !iteratorClone->IsAtEnd() )
			{
				mitk::DataTreeNode::Pointer node = iteratorClone->Get();
				if (  node == m_Node )
				{
					iteratorClone->Add(segmentationNode);
				}
				++iteratorClone;

			}
		}
	}

	mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

mitk::DataTreeNode::Pointer QmitkThresholdComponent::CreateEmptySegmentationNode( mitk::Image* image)
{
	//if (!image) return NULL;
	//// actually create a new empty segmentation
	//mitk::PixelType pixelType( typeid(short int) );
	//mitk::Image::Pointer segmentation = mitk::Image::New();
	//segmentation->Initialize( pixelType, image->GetDimension(), image->GetDimensions() );
	//memset( segmentation->GetData(), 0, sizeof(short int) * segmentation->GetDimension(0) * segmentation->GetDimension(1) * segmentation->GetDimension(2) );

	//if (image->GetGeometry() )
	//{
	//            mitk::AffineGeometryFrame3D::Pointer originalGeometryAGF =  image->GetGeometry()->Clone();
	//            mitk::Geometry3D::Pointer originalGeometry = dynamic_cast<mitk::Geometry3D*>( originalGeometryAGF.GetPointer() );
	//            segmentation->SetGeometry( originalGeometry );
	//}

	return CreateSegmentationNode(image);
}

mitk::DataTreeNode::Pointer QmitkThresholdComponent::CreateSegmentationNode(  
	mitk::Image* image)
{
	if (!image) return NULL;

	// decorate the datatreenode with some properties
	mitk::DataTreeNode::Pointer segmentationNode = mitk::DataTreeNode::New();
	segmentationNode->SetData( image );
	mitk::DataTreeNodeFactory::SetDefaultImageProperties ( segmentationNode );

	// visualization properties

	segmentationNode->SetProperty( "binary", new mitk::BoolProperty(true) );
	segmentationNode->SetProperty( "layer", new mitk::IntProperty(10) );
	segmentationNode->SetProperty( "segmentation", new mitk::BoolProperty(true) );
	segmentationNode->SetProperty( "opacity", new mitk::FloatProperty(0.3) );

	segmentationNode->SetProperty( "levelwindow", new mitk::LevelWindowProperty( mitk::LevelWindow(0, 1) ) );
	segmentationNode->SetProperty( "color", new mitk::ColorProperty(0.0, 1.0, 0.0) );

	return segmentationNode;
}

//*************************************TEMPLATE FOR THRESHOLDSEGMENTATION******************************
//
// to create a new segmentation that contains those areas above the threshold
// called from NewThresholdSegmentation

template < typename TPixel, unsigned int VImageDimension >
void QmitkThresholdComponent::ThresholdSegmentation(itk::Image< TPixel, VImageDimension >* itkImage, mitk::Image* segmentation, QmitkThresholdComponent * /*thresholdComponent*/)
{
	// iterator on m_MitkImage
	typedef itk::Image< TPixel, VImageDimension > ItkImageType;
	itk::ImageRegionConstIterator<ItkImageType> itMitkImage(itkImage, itkImage->GetLargestPossibleRegion() );

	// pointer on segmentation
	typedef itk::Image< unsigned char, VImageDimension > ItkSegmentationImageType;
	typename ItkSegmentationImageType::Pointer itkSegmentation;

	// cast segmentation from mitk-image to itk-image
	if(segmentation != NULL)
	{
		mitk::CastToItkImage(segmentation, itkSegmentation);
	}

	// new pointer on segmentation: itkThresholdSegmentedImage
	typename ItkSegmentationImageType::Pointer itkThresholdSegmentedImage = ItkSegmentationImageType::New();

	// properties for itkThresholdSegmentedImage:
	itkThresholdSegmentedImage->SetRegions(itkImage->GetLargestPossibleRegion());
	itkThresholdSegmentedImage->Allocate();

	// iterator on itkThresholdSegmentedImage: itSegmented
	itk::ImageRegionIterator<ItkSegmentationImageType> itSegmented(itkThresholdSegmentedImage, itkThresholdSegmentedImage->GetLargestPossibleRegion() );


	int thresholdValue(0);//Threshold above that the segmentation shall be created

	// threshold-input from GUI:
	thresholdValue = m_ThresholdComponentGUI->GetNumberValue();

	while(!(itMitkImage.IsAtEnd()))
	{
		if((signed)itMitkImage.Get() >= thresholdValue)
			//if the pixel-value of the m_Mitk-Image is higher or equals the threshold
		{
			itSegmented.Set(1);
			// set the pixel-value at the segmentation to "1"
		}
		else
		{
			itSegmented.Set(0);
			// else set the pixel-value at the segmentation to "0"
		}
		++itMitkImage;
		++itSegmented;
		//TODO: die Segmentierung aus der Methode returnen
	}//end of while (!(itMitkImage.IsAtEnd()))

	// create new mitk-Image: m_ThresholdSegmentationImage
	m_ThresholdSegmentationImage = mitk::Image::New(); 

	// fill m_ThresholdSegmentationImage with itkThresholdSegmentedImage:
	mitk::CastToMitkImage(itkThresholdSegmentedImage, m_ThresholdSegmentationImage);

}



