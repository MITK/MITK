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
#include "QmitkThresholdComponent.h"
#include "ui_QmitkThresholdComponentControls.h"


#include <QmitkDataStorageComboBox.h>
#include <mitkDataStorage.h>

#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"
#include "mitkDataTreeNodeFactory.h"
#include "mitkLevelWindowProperty.h"

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>

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
  SetAvailability(true);

  SetComponentName("ThresholdFinder");
  m_Node = this->GetDefaultDataStorage()->GetNode();
}

/***************        DESTRUCTOR      ***************/
QmitkThresholdComponent::~QmitkThresholdComponent()
{

}


/************** SET SELECTOR VISIBILITY ***************/
void QmitkThresholdComponent::SetSelectorVisibility(bool visibility)
{
  if(m_ThresholdComponentGUI)
  {
    //m_ImageContent->setShown(visibility);
    m_ImageContent->setShown(true);
  }
}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox* QmitkThresholdComponent::GetImageContent()
{
  return (QGroupBox*) m_ImageContent;
}


/*************** GET TREE NODE SELECTOR ***************/
QmitkDataStorageComboBox* QmitkThresholdComponent::GetTreeNodeSelector()
{
  return m_TreeNodeSelector;
}

/***************       CONNECTIONS      ***************/
void QmitkThresholdComponent::CreateConnections()
{
  if ( m_ThresholdComponentGUI )
  {
    connect( (QObject*)(m_TreeNodeSelector), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
    connect( (QObject*)(m_ThresholdFinder), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowThresholdFinderContent(bool)));     
    connect( (QObject*)(m_ThresholdSelectDataGroupBox), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool))); 

    connect( (QObject*)(m_ThresholdInputSlider), SIGNAL(sliderMoved(int)), (QObject*) this, SLOT(ThresholdSliderChanged(int)));
    connect( (QObject*)(m_ThresholdInputNumber), SIGNAL(returnPressed()), (QObject*) this, SLOT(ThresholdValueChanged()));    
    //connect( (QObject*)( m_ShowThresholdGroupBox), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowThreshold(bool)));  

    //to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
    connect( (QObject*)(m_ThresholdFinder),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 
    connect( (QObject*)(m_CreateSegmentationButton),  SIGNAL(pressed()), (QObject*) this, SLOT(CreateThresholdSegmentation())); 
  }
}

/***************     DATA STORAGE CHANGED     ***************/
void  QmitkThresholdComponent::DataStorageChanged(mitk::DataStorage::Pointer ds)
{

}

/***************     IMAGE SELECTED     ***************/
void QmitkThresholdComponent::ImageSelected(mitk::DataTreeNode::Pointer item)
{
  if(m_ThresholdComponentGUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(item);
    }
  }
  m_Node = item;
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

/** \brief Method to set the DataStorage*/
void QmitkThresholdComponent::SetDataStorage(mitk::DataStorage::Pointer dataStorage)
{
  m_DataStorage = dataStorage;
}

/** \brief Method to get the DataStorage*/
mitk::DataStorage::Pointer QmitkThresholdComponent::GetDataStorage()
{
 return m_DataStorage;
}

/*************** CREATE CONTAINER WIDGET **************/
void QmitkThresholdComponent::CreateQtPartControl(QWidget *parent, mitk::DataStorage::Pointer dataStorage)
{
  m_GUI = new QWidget;
  m_ThresholdComponentGUI = new Ui::QmitkThresholdComponentControls;
  m_ThresholdComponentGUI->setupUi(m_GUI);
  
  

  /*CREATE GUI ELEMENTS*/
  
  m_ThresholdFinder = new QGroupBox("2. Find Threshold", m_GUI);
  m_ThresholdSelectDataGroupBox = new QGroupBox("Show Image Selector", m_ThresholdFinder);
  m_TreeNodeSelector = new QmitkDataStorageComboBox(m_ThresholdSelectDataGroupBox);
  m_ImageContent = new QGroupBox("m_ImageContent", m_ThresholdSelectDataGroupBox);
  m_ContainerContent = new QGroupBox(m_ImageContent);
  m_ShowThresholdGroupBox = new QGroupBox("m_ShowThresholdGroupBox", m_ContainerContent);
  m_ThresholdInputNumber = new QLineEdit(m_ShowThresholdGroupBox);
  m_ThresholdInputSlider = new QSlider(m_ShowThresholdGroupBox);
  m_ThresholdInputSlider->setOrientation(Qt::Orientation::Horizontal);
  m_ThresholdValueContent = new QGroupBox("m_ThresholdValueContent", m_ShowThresholdGroupBox);
  m_CreateSegmentationButton = new QPushButton("Create Segmentation", m_ThresholdValueContent);
  m_DeleateImageIfDeactivatedCheckBox = new QCheckBox("Deleate Threshold Image if \nComponent is deactivated", m_ThresholdValueContent);

  m_ThresholdFinder->setCheckable(true);
  m_ThresholdFinder->setChecked(true);
  m_ThresholdSelectDataGroupBox->setCheckable(true);
  m_ThresholdSelectDataGroupBox->setChecked(true);
  m_ThresholdInputNumber->setFixedSize(40, 20);
  m_ThresholdValueContent->setMaximumHeight(90);

 // m_ThresholdSelectDataGroupBox->setContentsMargins(0,9,9,9);
 // m_ImageContent->setContentsMargins(0,9,9,9);
 // m_ContainerContent->setContentsMargins(0,9,9,9);
 // m_ShowThresholdGroupBox->setContentsMargins(0,9,9,9);
 //m_ThresholdValueContent->setContentsMargins(0,9,9,9);

 //m_ThresholdFinder->setFlat(true);
 //m_ThresholdSelectDataGroupBox->setFlat(true);
 //m_ImageContent->setFlat(true);
 //m_ContainerContent->setFlat(true);
 //m_ShowThresholdGroupBox->setFlat(true);
 //m_ThresholdValueContent->setFlat(true);
 


  QVBoxLayout* guiLayout = new QVBoxLayout(m_GUI);
  m_GUI->setLayout(guiLayout);
  guiLayout->addWidget(m_ThresholdFinder);

  QVBoxLayout* thresholdFinderLayout = new QVBoxLayout(m_ThresholdFinder);
  thresholdFinderLayout->setContentsMargins(0,9,0,9);
  m_ThresholdFinder->setLayout(thresholdFinderLayout);
  thresholdFinderLayout->addWidget(m_ThresholdSelectDataGroupBox);
  thresholdFinderLayout->addWidget(m_ImageContent);

  QVBoxLayout* thresholdSelectDataGroupBoxLayout = new QVBoxLayout(m_ThresholdSelectDataGroupBox);
  thresholdSelectDataGroupBoxLayout->setContentsMargins(0,9,0,9);
  m_ThresholdSelectDataGroupBox->setLayout(thresholdSelectDataGroupBoxLayout);
  thresholdSelectDataGroupBoxLayout->addWidget(m_TreeNodeSelector);
  

  QVBoxLayout* imageContentLayout = new QVBoxLayout(m_ImageContent);
  imageContentLayout->setContentsMargins(0,9,0,9);
  m_ImageContent->setLayout(imageContentLayout);
  imageContentLayout->addWidget(m_ContainerContent);

  QVBoxLayout* containerContentLayout = new QVBoxLayout(m_ContainerContent);
  containerContentLayout->setContentsMargins(0,9,0,9);
  m_ContainerContent->setLayout(containerContentLayout);
  containerContentLayout->addWidget(m_ShowThresholdGroupBox);

  QVBoxLayout* showThresholdGroupBoxLayout = new QVBoxLayout(m_ShowThresholdGroupBox);
  showThresholdGroupBoxLayout->setContentsMargins(0,9,0,9);
  m_ShowThresholdGroupBox->setLayout(showThresholdGroupBoxLayout);
  QHBoxLayout* thresholdInputLayout = new QHBoxLayout(m_ShowThresholdGroupBox);
  thresholdInputLayout->addWidget(m_ThresholdInputNumber);
  thresholdInputLayout->addWidget(m_ThresholdInputSlider);
  showThresholdGroupBoxLayout->addLayout(thresholdInputLayout);
  showThresholdGroupBoxLayout->addWidget(m_ThresholdValueContent);

  QVBoxLayout* thresholdValueContentLayout = new QVBoxLayout(m_ThresholdValueContent);
  thresholdValueContentLayout->setContentsMargins(0,9,0,9);
  m_ThresholdValueContent->setLayout(thresholdValueContentLayout);
  thresholdValueContentLayout->addWidget(m_DeleateImageIfDeactivatedCheckBox);
  thresholdValueContentLayout->addWidget(m_CreateSegmentationButton);

  this->CreateConnections();
  SetDataStorage(dataStorage);

  m_TreeNodeSelector->SetDataStorage(dataStorage);
  m_TreeNodeSelector->SetPredicate(mitk::NodePredicateDataType::New("Image"));
//  m_GUI = m_ThresholdComponentGUI;

  //   m_TreeNodeSelector->SetDataTree(GetDataTreeIterator());

  if(m_ShowSelector)
  {
    m_ImageContent->setShown( m_ThresholdSelectDataGroupBox->isChecked());
    //m_ImageContent->setShown(true);
  }
  else
  {
     m_ThresholdSelectDataGroupBox->setShown(m_ShowSelector);
     //m_ThresholdSelectDataGroupBox->setShown(true);
  }

  // m_TreeNodeSelector->GetFilter()->SetFilter(mitk::IsBaseDataTypeWithoutProperty<mitk::Image>("isComponentThresholdImage"));

}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkThresholdComponent::GetContentContainer()
{
  return  m_ContainerContent;
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkThresholdComponent::GetMainCheckBoxContainer()
{
  return  m_ThresholdFinder;
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
  if( m_DeleateImageIfDeactivatedCheckBox->isChecked())
  {
    DeleteThresholdNode();
  }
}

///************ SHOW THRESHOLD FINDER CONTENT ***********/
void QmitkThresholdComponent::ShowThresholdFinderContent(bool)
{
  // m_ShowThresholdGroupBox->setShown( m_ThresholdFinder->isChecked());
  // m_ContainerContent->setShown( m_ThresholdSelectDataGroupBox->isChecked());
  m_ContainerContent->setShown( m_ThresholdFinder->isChecked());


  if(m_ShowSelector)
  {
     //m_ThresholdSelectDataGroupBox->setShown( m_ThresholdSelectDataGroupBox->isChecked());
     //m_ThresholdSelectDataGroupBox->setShown( true);
    m_ThresholdSelectDataGroupBox->setShown(m_ThresholdFinder->isChecked());
  }

  //ShowThreshold();
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkThresholdComponent::ShowImageContent(bool)
{
   //m_ImageContent->setShown( m_ThresholdSelectDataGroupBox->isChecked());
  m_ImageContent->setShown( true);

  if(m_ShowSelector)
  {
     //m_ImageContent->setShown( m_ThresholdSelectDataGroupBox->isChecked());
     m_ImageContent->setShown( true);
  }
  else
  {
     //m_ThresholdSelectDataGroupBox->setShown(m_ShowSelector);
    m_ThresholdSelectDataGroupBox->setShown(true);
  }
}

///***************      SHOW THRESHOLD     **************/
void QmitkThresholdComponent::ShowThreshold(bool)
{
  if(m_ThresholdImageNode)
  {
    if(m_Active == true)
    {
      m_ThresholdImageNode->SetProperty("visible", mitk::BoolProperty::New(( m_ThresholdFinder->isChecked())) );
    }
    else
    {
      if( m_DeleateImageIfDeactivatedCheckBox->isChecked())
      {
        m_ThresholdImageNode->SetProperty("visible", mitk::BoolProperty::New((false)) );
      }
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

///*************** THRESHOLD VALUE CHANGED **************/
//By Slider
void QmitkThresholdComponent::ThresholdSliderChanged(int)
{
  int value =  m_ThresholdInputSlider->value();
  if (m_ThresholdImageNode)
  {
    m_ThresholdImageNode->SetLevelWindow(mitk::LevelWindow(value,1));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
   m_ThresholdInputNumber->setText(QString::number(value)); 
}

///*************** THRESHOLD VALUE CHANGED **************/
//By LineEdit
void QmitkThresholdComponent::ThresholdValueChanged( )
{
  int value = atoi( m_ThresholdInputNumber->text());
  if (m_ThresholdImageNode)
  {
    m_ThresholdImageNode->SetLevelWindow(mitk::LevelWindow(value,1));
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
   m_ThresholdInputSlider->setValue(value);
}


///***************     SET SLIDER RANGE    **************/
void QmitkThresholdComponent::SetSliderRange()
{
  if(m_Active)
  {
    if( m_ThresholdFinder->isChecked()==true)
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
         m_ThresholdInputSlider->setMinValue(min);
         m_ThresholdInputSlider->setMaxValue(max);
        /* m_ThresholdInputSlider->setMinValue((int)currentImage->GetScalarValueMin());
         m_ThresholdInputSlider->setMaxValue((int)currentImage->GetScalarValueMaxNoRecompute());*/
      }
    }
  }
}

///***************  DELETE THRESHOLD NODE  **************/
void QmitkThresholdComponent::DeleteThresholdNode()
{
  if(m_ThresholdImageNode)
  {

    mitk::DataTreeNode::Pointer foundNode = GetDefaultDataStorage()->GetNamedNode("Thresholdview image");
    foundNode->Delete();


    m_ThresholdNodeExisting = false;
    return;
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
          mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New("Thresholdview image" );
          m_ThresholdImageNode->SetProperty( "name", nameProp );
          mitk::BoolProperty::Pointer componentThresholdImageProp = mitk::BoolProperty::New(true);
          m_ThresholdImageNode->SetProperty( "isComponentThresholdImage", componentThresholdImageProp );

          m_ThresholdImageNode->SetData(m_Node->GetData());
          m_ThresholdImageNode->SetColor(0.0,1.0,0.0);
          m_ThresholdImageNode->SetOpacity(.25);
          int layer = 0;
          m_Node->GetIntProperty("layer", layer);
          m_ThresholdImageNode->SetIntProperty("layer", layer+1);
          m_ThresholdImageNode->SetLevelWindow(mitk::LevelWindow(atoi( m_ThresholdInputNumber->text()),1));

          GetDefaultDataStorage()->Add(m_ThresholdImageNode);
          m_ThresholdNodeExisting = true;

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
      mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New("TH segmentation" );
      segmentationNode->SetProperty( "name", nameProp );
      segmentationNode->GetPropertyList()->SetProperty("binary", mitk::BoolProperty::New(true));
      mitk::BoolProperty::Pointer thresholdBasedSegmentationProp = mitk::BoolProperty::New(true);
      segmentationNode->SetProperty( "segmentation", thresholdBasedSegmentationProp );
      segmentationNode->GetPropertyList()->SetProperty("layer",mitk::IntProperty::New(1));
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

        mitk::DataTreeNode::Pointer origNode =  m_TreeNodeSelector->GetSelectedNode();
        GetDefaultDataStorage()->Add(segmentationNode, origNode);

        //mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
        //iteratorClone->GoToBegin();
        //while ( !iteratorClone->IsAtEnd() )
        //{
        //  mitk::DataTreeNode::Pointer node = iteratorClone->Get();
        //  if (  node == m_Node )
        //  {
        //    iteratorClone->Add(segmentationNode);
        //  }
        //  ++iteratorClone;

        //}
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

    // visualization properties

    segmentationNode->SetProperty( "binary", mitk::BoolProperty::New(true) );
    segmentationNode->SetProperty( "layer", mitk::IntProperty::New(10) );
    segmentationNode->SetProperty( "segmentation", mitk::BoolProperty::New(true) );
    segmentationNode->SetProperty( "opacity", mitk::FloatProperty::New(0.3) );

    segmentationNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0, 1) ) );
    segmentationNode->SetProperty( "color", mitk::ColorProperty::New(0.0, 1.0, 0.0) );

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
    thresholdValue = atoi( m_ThresholdInputNumber->text());

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



