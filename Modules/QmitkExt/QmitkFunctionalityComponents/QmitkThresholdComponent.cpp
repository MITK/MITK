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
#include "mitkDataNodeFactory.h"
#include "mitkImageTimeSelector.h"
#include "mitkLevelWindowProperty.h"

#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <QIntValidator>

#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>
//#include <mitkIpPicTypeMultiplex.h>
#include <itkImageConstIteratorWithIndex.h>


/***************       CONSTRUCTOR      ***************/
QmitkThresholdComponent::QmitkThresholdComponent(QObject * parent, const char * parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget * /*mitkStdMultiWidget*/)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector),
m_ThresholdImageNode(NULL),
m_ThresholdComponentGUI(NULL),
m_ThresholdNodeExisting(false)
{
  SetAvailability(true);

  SetComponentName("ThresholdFinder");
}

/***************        DESTRUCTOR      ***************/
QmitkThresholdComponent::~QmitkThresholdComponent()
{

}


/************** SET SELECTOR VISIBILITY ***************/
void QmitkThresholdComponent::SetSelectorVisibility(bool  /*visibility*/)
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
    connect( (QObject*)(m_TreeNodeSelector), SIGNAL(OnSelectionChanged (const mitk::DataNode *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataNode *)));
    connect( (QObject*)(m_ThresholdFinder), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowThresholdFinderContent(bool)));     
    connect( (QObject*)(m_ThresholdSelectDataGroupBox), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool))); 

    connect( (QObject*)(m_ThresholdInputSlider), SIGNAL(sliderMoved(int)), (QObject*) this, SLOT(ThresholdSliderChanged(int)));
    connect( (QObject*)(m_ThresholdInputNumber), SIGNAL(returnPressed()), (QObject*) this, SLOT(ThresholdValueChanged()));    
    //connect( (QObject*)( m_ShowThresholdGroupBox), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowThreshold(bool)));  

    //to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
    connect( (QObject*)(m_ThresholdFinder),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 
    connect( (QObject*)(m_CreateSegmentationButton),  SIGNAL(released()), (QObject*) this, SLOT(CreateThresholdSegmentation())); 
  }
}

/***************     DATA STORAGE CHANGED     ***************/
void  QmitkThresholdComponent::DataStorageChanged(mitk::DataStorage::Pointer ds)
{
  if(!ds)
    return;
  m_DataStorage = ds;
  m_TreeNodeSelector->SetDataStorage(ds);

  if(m_ThresholdComponentGUI != NULL)
  {
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->DataStorageChanged(ds);
    }
  }
  if(!ds)
    return;

  DataObjectSelected();
  SetSliderRange();
  ShowThreshold();
}

/***************     IMAGE SELECTED     ***************/
void QmitkThresholdComponent::ImageSelected(const mitk::DataNode* item)
{
  if(m_ThresholdComponentGUI != NULL)
  {
    mitk::DataNode::Pointer selectedItem = const_cast< mitk::DataNode*>(item);
    m_TreeNodeSelector->SetSelectedNode(selectedItem);
    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      QmitkBaseFunctionalityComponent* functionalityComponent = dynamic_cast<QmitkBaseFunctionalityComponent*>(m_AddedChildList[i]);
      if (functionalityComponent != NULL)
        functionalityComponent->ImageSelected(item);
    }
  }
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
      m_ThresholdImageNode->SetData(m_TreeNodeSelector->GetSelectedNode()->GetData());
    }
    else
    {
      CreateThresholdImageNode();
      m_ThresholdImageNode->SetData(m_TreeNodeSelector->GetSelectedNode()->GetData());
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
void QmitkThresholdComponent::CreateQtPartControl(QWidget * /*parent*/, mitk::DataStorage::Pointer dataStorage)
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
  m_ThresholdInputSlider->setOrientation(Qt::Horizontal);
  m_ThresholdValueContent = new QGroupBox("m_ThresholdValueContent", m_ShowThresholdGroupBox);
  m_CreateSegmentationButton = new QPushButton("Create Segmentation", m_ThresholdValueContent);
  m_DeleateImageIfDeactivatedCheckBox = new QCheckBox("Deleate Threshold Image if \nComponent is deactivated", m_ThresholdValueContent);

  m_ThresholdFinder->setCheckable(true);
  m_ThresholdFinder->setChecked(true);
  
  m_ThresholdSelectDataGroupBox->setCheckable(true);
  m_ThresholdSelectDataGroupBox->setChecked(true);
  
  m_ThresholdInputNumber->setFixedSize(40, 20);
  QIntValidator* intValid = new QIntValidator(-32000, 5000, m_ThresholdInputNumber);
  m_ThresholdInputNumber->setValidator(intValid);
  m_ThresholdInputNumber->setText("0");

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
  int value = atoi( m_ThresholdInputNumber->text().toLocal8Bit().constData() );
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
      if(!m_TreeNodeSelector->GetSelectedNode())
        return;
      if(!m_TreeNodeSelector->GetSelectedNode()->GetData())
        return;
      mitk::Image* currentImage = dynamic_cast<mitk::Image*>(m_TreeNodeSelector->GetSelectedNode()->GetData());
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

        QIntValidator* intValid = new QIntValidator(min-150, max+150, m_ThresholdInputNumber);
        m_ThresholdInputNumber->setValidator(intValid);
        m_ThresholdInputNumber->setText("1");
        
        m_ThresholdInputSlider->setMinimum(min-150);
        m_ThresholdInputSlider->setMaximum(max+150);
        m_ThresholdInputSlider->setRange(min-150, max+150);
        m_ThresholdInputSlider->setPageStep(1);
        m_ThresholdInputSlider->setValue(1);

        m_GUI->repaint();
        //m_ThresholdInputSlider->resize();

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

    mitk::DataNode::Pointer foundNode = m_DataStorage->GetNamedNode("Thresholdview image");
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
    if( m_ThresholdNodeExisting)
      return;
    if(!m_TreeNodeSelector)
      return;
    if(!m_TreeNodeSelector->GetSelectedNode())
      return;
    m_ThresholdImageNode = mitk::DataNode::New();
    mitk::StringProperty::Pointer nameProp = mitk::StringProperty::New("Thresholdview image" );
    m_ThresholdImageNode->SetProperty( "name", nameProp );
    mitk::BoolProperty::Pointer componentThresholdImageProp = mitk::BoolProperty::New(true);
    m_ThresholdImageNode->SetProperty( "isComponentThresholdImage", componentThresholdImageProp );
    m_ThresholdImageNode->SetData(m_TreeNodeSelector->GetSelectedNode()->GetData());
    m_ThresholdImageNode->SetColor(0.0,1.0,0.0);
    m_ThresholdImageNode->SetOpacity(.25);
    int layer = 0;
    m_ThresholdImageNode->GetIntProperty("layer", layer);
    m_ThresholdImageNode->SetIntProperty("layer", layer+1);
    m_ThresholdImageNode->SetLevelWindow(mitk::LevelWindow(atoi( m_ThresholdInputNumber->text().toLocal8Bit().constData()),1));
    m_ThresholdNodeExisting = true;
    m_DataStorage->Add(m_ThresholdImageNode);
    
  }
}

/*************CREAET THRESHOLD SEGMENTATION************/
void QmitkThresholdComponent::CreateThresholdSegmentation()
{
  mitk::Image::Pointer original = dynamic_cast<mitk::Image*>(m_TreeNodeSelector->GetSelectedNode()->GetData());
  // we NEED a reference image for size etc.
  if (!original) return;

  // actually create a new empty segmentation
  mitk::PixelType pixelType( mitk::MakeScalarPixelType<DefaultSegmentationDataType>() );
  mitk::Image::Pointer segmentation = mitk::Image::New();
  //segmentation->SetProperty( "organ type", OrganTypeProperty::New( organType ) );

  segmentation->Initialize( pixelType, original->GetDimension(), original->GetDimensions() );

  unsigned int byteSize = sizeof(DefaultSegmentationDataType);
  for (unsigned int dim = 0; dim < segmentation->GetDimension(); ++dim) 
  {
    byteSize *= segmentation->GetDimension(dim);
  }
  memset( segmentation->GetData(), 0, byteSize );

  if (original->GetTimeSlicedGeometry() )
  {
    mitk::AffineGeometryFrame3D::Pointer originalGeometryAGF = original->GetTimeSlicedGeometry()->Clone();
    mitk::TimeSlicedGeometry::Pointer originalGeometry = dynamic_cast<mitk::TimeSlicedGeometry*>( originalGeometryAGF.GetPointer() );
    segmentation->SetGeometry( originalGeometry );
  }
  else
  {
    MITK_INFO<<"Original image does not have a 'Time sliced geometry'! Cannot create a segmentation.";
    return ;
  }

  mitk::DataNode::Pointer emptySegmentationNode = CreateSegmentationNode( segmentation);

  if (emptySegmentationNode)
  {
    // actually perform a thresholding and ask for an organ type
    for (unsigned int timeStep = 0; timeStep < original->GetTimeSteps(); ++timeStep)
    {
      try
      {
        mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
        timeSelector->SetInput( original );
        timeSelector->SetTimeNr( timeStep );
        timeSelector->UpdateLargestPossibleRegion();
        mitk::Image::Pointer image3D = timeSelector->GetOutput();

        AccessFixedDimensionByItk_2( image3D, ITKThresholding, 3, dynamic_cast<mitk::Image*>(emptySegmentationNode->GetData()), timeStep );
      }
      catch(...)
      {
        MITK_INFO<<"Error accessing single time steps of the original image. Cannot create segmentation.";
      }
    }

    mitk::DataNode::Pointer originalNode = m_TreeNodeSelector->GetSelectedNode();
    m_DataStorage->Add( emptySegmentationNode, originalNode ); // add as a child, because the segmentation "derives" from the original
   
  }
}

mitk::DataNode::Pointer QmitkThresholdComponent::CreateSegmentationNode( mitk::Image* image)
{

  if (!image) return NULL;

  // decorate the datatreenode with some properties
  mitk::DataNode::Pointer segmentationNode = mitk::DataNode::New();
  segmentationNode->SetData( image );

  // name
  segmentationNode->SetProperty( "name", mitk::StringProperty::New( "TH segmentation from ThresholdFinder" ) );

  // visualization properties
  segmentationNode->SetProperty( "binary", mitk::BoolProperty::New(true) );
  segmentationNode->SetProperty( "color", mitk::ColorProperty::New(0.0, 1.0, 0.0) );
  segmentationNode->SetProperty( "texture interpolation", mitk::BoolProperty::New(false) );
  segmentationNode->SetProperty( "layer", mitk::IntProperty::New(10) );
  segmentationNode->SetProperty( "levelwindow", mitk::LevelWindowProperty::New( mitk::LevelWindow(0.5, 1) ) );
  segmentationNode->SetProperty( "opacity", mitk::FloatProperty::New(0.3) );
  segmentationNode->SetProperty( "segmentation", mitk::BoolProperty::New(true) );
  segmentationNode->SetProperty( "showVolume", mitk::BoolProperty::New( false ) );

  return segmentationNode;
}

template <typename TPixel, unsigned int VImageDimension>
void QmitkThresholdComponent::ITKThresholding( itk::Image<TPixel, VImageDimension>* originalImage, mitk::Image* segmentation, unsigned int timeStep )
{
  mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
  timeSelector->SetInput( segmentation );
  timeSelector->SetTimeNr( timeStep );
  timeSelector->UpdateLargestPossibleRegion();
  mitk::Image::Pointer segmentation3D = timeSelector->GetOutput();

  typedef itk::Image< DefaultSegmentationDataType, 3> SegmentationType; // this is sure for new segmentations
  SegmentationType::Pointer itkSegmentation;
  CastToItkImage( segmentation3D, itkSegmentation );

  // iterate over original and segmentation
  typedef itk::ImageRegionConstIterator< itk::Image<TPixel, VImageDimension> >     InputIteratorType;
  typedef itk::ImageRegionIterator< SegmentationType >     SegmentationIteratorType;

  InputIteratorType inputIterator( originalImage, originalImage->GetLargestPossibleRegion() );
  SegmentationIteratorType outputIterator( itkSegmentation, itkSegmentation->GetLargestPossibleRegion() );

  inputIterator.GoToBegin();
  outputIterator.GoToBegin();

  while (!outputIterator.IsAtEnd())
  {
    if ( (signed)inputIterator.Get() >= atoi( m_ThresholdInputNumber->text().toLocal8Bit().constData()) )
    {
      outputIterator.Set( 1 );
    }
    else
    {
      outputIterator.Set( 0 );
    }

    ++inputIterator;
    ++outputIterator;
  }
}
