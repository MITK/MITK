/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-07-16 17:53:57 +0200 (Do, 16 Jul 2009) $
Version:   $Revision: 11185 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkAdaptiveRegionGrowingView.h"

#include <qaction.h>
#include "QmitkCommonFunctionality.h"
#include "QmitkStdMultiWidget.h"

#include <qpushbutton.h>
#include <qmessagebox.h>
#include <qapplication.h>
#include <qcursor.h>
#include <qslider.h>
#include <qlabel.h>

#include "mitkNodePredicateDataType.h"
#include "mitkDataNodeFactory.h"
#include "mitkGlobalInteraction.h"
#include "mitkPointSet.h"
#include "mitkPointSetInteractor.h"
#include "mitkProgressBar.h"
#include "mitkStatusBar.h"
#include "mitkProperties.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"
//#include "Algorithms/mitkLabeledSegmentationImageToVesselTreeWrapper.h"
#include "mitkGPUVolumeMapper3D.h"
#include "mitkTransferFunctionProperty.h"
//#include "mitkImageToTracheaSeedPointSetFilter.h"
#include "mitkImageTimeSelector.h"

#include <itkConnectedAdaptiveThresholdImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>

QmitkAdaptiveRegionGrowingView::QmitkAdaptiveRegionGrowingView(QWidget * parent) :
QWidget(parent), m_MultiWidget(NULL), m_UseVolumeRendering(false), m_UpdateSuggestedThreshold(true), m_SuggestedThValue(0.0)
{
  m_Controls.setupUi(this);
  m_NAMEFORSEEDPOINT = "Seed Point";
  this->CreateConnections();
  this->SetDataNodeNames("labeledRGSegmentation","RGResult","RGFeedbackSurface","RGSeedpoint");

}

QmitkAdaptiveRegionGrowingView::~QmitkAdaptiveRegionGrowingView()
{
    //Removing the observer of the PointSet node
    mitk::DataNode* node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);
    if (node != NULL)
    {
        this->DeactivateSeedPointMode();
        dynamic_cast<mitk::PointSet*>(node->GetData())->RemoveObserver(m_PointSetAddObserverTag);
    }    
}

void QmitkAdaptiveRegionGrowingView::CreateConnections()
{
    //Connecting GUI components
    connect( (QObject*) (m_Controls.m_pbDefineSeedPoint), SIGNAL( toggled(bool) ), this, SLOT( SetSeedPointToggled(bool)) );
    connect( (QObject*) (m_Controls.m_pbRunSegmentation), SIGNAL(clicked()), this, SLOT(RunSegmentation()));
    connect( (QObject*) (m_Controls.m_Slider), SIGNAL(valueChanged(int)), this, SLOT(ChangeLevelWindow(int)) );
    connect( (QObject*) (m_Controls.m_DecreaseTH), SIGNAL(clicked()), this, SLOT(DecreaseSlider()));
    connect( (QObject*) (m_Controls.m_IncreaseTH), SIGNAL(clicked()), this,SLOT(IncreaseSlider()));
    connect( (QObject*) (m_Controls.m_pbConfirmSegementation), SIGNAL(clicked()), this, SLOT(ConfirmSegmentation()));
    connect( (QObject*) (m_Controls.m_cbVolumeRendering), SIGNAL(toggled(bool)), this, SLOT(UseVolumeRendering(bool) ));
    connect( (QObject*) (m_Controls.rb_Upwards), SIGNAL(toggled(bool)), this, SLOT(SetRegionGrowingDirectionUpwards(bool)));
    connect( (QObject*) (m_Controls.cb_DefineThresholdBoundaries), SIGNAL(toggled(bool)), this, SLOT(OnDefineThresholdBoundaries(bool)));

    connect( m_Controls.m_LowerThresholdSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetLowerThresholdValue(int)));
    connect( m_Controls.m_UpperThresholdSpinBox, SIGNAL(valueChanged(int)), this, SLOT(SetUpperThresholdValue(int)));
}

void QmitkAdaptiveRegionGrowingView::SetDataNodeNames(std::string labledSegmentation, std::string binaryImage, std::string surface, std::string seedPoint)
{
  m_NAMEFORLABLEDSEGMENTATIONIMAGE = labledSegmentation;
  m_NAMEFORBINARYIMAGE = binaryImage;
  m_NAMEFORSURFACE = surface;
  m_NAMEFORSEEDPOINT = seedPoint;
}

void QmitkAdaptiveRegionGrowingView::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkAdaptiveRegionGrowingView::SetMultiWidget(QmitkStdMultiWidget* multiWidget)
{
  m_MultiWidget = multiWidget;
//  itk::SimpleMemberCommand< QmitkAdaptiveRegionGrowingView >::Pointer timeStepperChangedCommand = itk::SimpleMemberCommand< QmitkAdaptiveRegionGrowingView >::New();

//  timeStepperChangedCommand->SetCallbackFunction(
//    this, &QmitkAdaptiveRegionGrowingView::TimeStepChanged );

//  m_Time->AddObserver( itk::ModifiedEvent(), timeStepperChangedCommand );
}

void QmitkAdaptiveRegionGrowingView::SetInputImageNode(mitk::DataNode* node)
{
  m_InputImageNode = node;
}

void QmitkAdaptiveRegionGrowingView::SetSeedPointToggled(bool toggled)
{
  if (m_InputImageNode.IsNull())
  {
    if (m_Controls.m_pbDefineSeedPoint->isChecked())
    {
      QMessageBox::information( NULL, "Adaptive Region Growing functionality", "Please specify the image in Datamanager!");
      m_Controls.m_pbDefineSeedPoint->toggle();
    }
    return;
  }

  //check that a pointset node with the given name exists in data storage
  mitk::DataNode* node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);
  if (node == NULL) //no pointSet present
  {
    // new node and data item
    mitk::DataNode::Pointer pointSetNode = mitk::DataNode::New();
    pointSetNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New(m_NAMEFORSEEDPOINT));
    pointSetNode->GetPropertyList()->SetProperty("helper object", mitk::BoolProperty::New(true));
    pointSetNode->GetPropertyList()->SetProperty("layer", mitk::IntProperty::New(2));

    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    pointSetNode->SetData(pointSet);

    //add to DataStorage
    m_DataStorage->Add(pointSetNode, m_InputImageNode);

    //Watch for point added or modified
//    itk::SimpleMemberCommand<QmitkAdaptiveRegionGrowingView>::Pointer pointAddedCommand = itk::SimpleMemberCommand<QmitkAdaptiveRegionGrowingView>::New();
//    pointAddedCommand->SetCallbackFunction(this, &QmitkAdaptiveRegionGrowingView::OnPointAdded);
//    m_PointSetAddObserverTag = pointSet->AddObserver( mitk::PointSetAddEvent(), pointAddedCommand);
  }

  mitk::NodePredicateDataType::Pointer imagePred = mitk::NodePredicateDataType::New("Image");
  mitk::DataStorage::SetOfObjects::ConstPointer setOfImages = m_DataStorage->GetSubset(imagePred);

  //no image node found
  if (setOfImages->Size() < 1)
  {
    QMessageBox::information(NULL, "Segmentation functionality", "Please load an image before setting a seed point.");
    return;
  }
  else
  {
    //get PointSet node from DataStorage
    mitk::DataNode* node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);
    if (node == NULL)
    {
      QMessageBox::critical(NULL, "QmitkAdaptiveRegionGrowingView", "No seed point node found!");
      return;
    }

   
    if (toggled == true) // button is down
    {
      this->ActivateSeedPointMode();  //add pointSet Interactor if there is none 
    }
    else
    {
      this->DeactivateSeedPointMode();  //disable pointSet Interactor
    }

    //enable the Run Segmentation button once the set seed point button is pressed
    m_Controls.m_pbRunSegmentation->setEnabled(true);

  }
}

void QmitkAdaptiveRegionGrowingView::OnPointAdded()
{
    mitk::DataNode* node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);

    if (node != NULL) //no pointSet present
    {
        mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());

        if (pointSet.IsNull())
        {
            QMessageBox::critical(NULL, "QmitkAdaptiveRegionGrowingView", "PointSetNode does not contain a pointset");
            return;
        }

        mitk::Point3D seedPoint = pointSet->GetPointSet(0)->GetPoints()->ElementAt(0);

        if (!m_Controls.cb_DefineThresholdBoundaries->isChecked())
        {
            // a node itself is not very useful, we need its data item (the image)
            mitk::BaseData* data = m_InputImageNode->GetData();
            if (data)
            {
              // test if this data item is an image or not (could also be a surface or something totally different)
              mitk::Image* image = dynamic_cast<mitk::Image*>( data );
              if (image)
              {
                  mitk::Index3D seedPointInIndex;
                  mitk::Index3D currentIndex;
                  mitk::ScalarType pixelValues[125];
                  unsigned int pos (0);
                  image->GetGeometry()->WorldToIndex(seedPoint, seedPointInIndex);

                  for(int i = seedPointInIndex[0]-2; i <= seedPointInIndex[0]+2; i++)
                  {
                      for(int j = seedPointInIndex[1]-2; j <= seedPointInIndex[1]+2; j++)
                      {
                          for(int k = seedPointInIndex[2]-2; k <= seedPointInIndex[2]+2; k++)
                          {
                              currentIndex[0] = i;
                              currentIndex[1] = j;
                              currentIndex[2] = k;
                              if(image->GetGeometry()->IsIndexInside(currentIndex))
                              {
                                  pixelValues[pos] = image->GetPixelValueByIndex(currentIndex);
                                  pos++;
                              }
                              else
                              {
                                  pixelValues[pos] = -10000000;
                                  pos++;
                              }
                          }
                      }
                  }

                  //Now calculation mean and deviation of the pixelValues
                  unsigned int numberOfValues(0);
                  for (unsigned int i = 0; i < 125; i++)
                  {
                      if(pixelValues[i] > -10000000)
                      {
                          m_SeedPointValueMean += pixelValues[i];
                          numberOfValues++;
                      }
                  }

                  m_SeedPointValueMean = m_SeedPointValueMean/numberOfValues;
                  MITK_INFO<<"Mean: "<<m_SeedPointValueMean;

                  mitk::ScalarType deviation(0);

                  for (unsigned int i = 0; i < 125; i++)
                  {
                      if(pixelValues[i] > -10000000)
                      {
                          deviation += pow((m_SeedPointValueMean - pixelValues[i]),2);
                      }
                  }

                  deviation = deviation/(numberOfValues-1);
                  deviation = sqrt(deviation);

                  //MITK_INFO<<"Deviation: "<<deviation
                  if (image->GetDimension() == 4)
                  {
                      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
                      timeSelector->SetInput(image);
                      timeSelector->SetTimeNr( m_MultiWidget->GetTimeNavigationController()->GetTime()->GetPos() );
                      timeSelector->UpdateLargestPossibleRegion();
                      //MITK_INFO<<"Time= "<<timeStep;
                      //image = timeSelector->GetOutput();
                  }

                  //mitk::ScalarType value = timeSelector->GetOutput()->GetPixelValueByWorldCoordinate(seedPoint);
//                  m_Controls.m_LowerThresholdSpinBox->setValue((value-(3*deviation)));
//                  m_Controls.m_UpperThresholdSpinBox->setValue((value+(3*deviation)));
//                  m_UPPERTHRESHOLD = (value+(3*deviation));
//                  m_LOWERTHRESHOLD = (value-(3*deviation));

//                  mitk::ScalarType windowSize =  timeSelector->GetOutput()->GetScalarValueMax()*0.15;

//                  m_UPPERTHRESHOLD = value+windowSize;

//                  if (m_UPPERTHRESHOLD > timeSelector->GetOutput()->GetScalarValueMax())
//                      m_UPPERTHRESHOLD = timeSelector->GetOutput()->GetScalarValueMax();

//                  m_LOWERTHRESHOLD = value-windowSize;

//                  if (m_LOWERTHRESHOLD < timeSelector->GetOutput()->GetScalarValueMin())
//                      m_LOWERTHRESHOLD = timeSelector->GetOutput()->GetScalarValueMin();

//                  MITK_INFO<<"PointAdded: Value: "<<value<<", Mean: "<<m_SeedPointValueMean<<", Lower/Upper: "<<m_LOWERTHRESHOLD<<", "<<m_UPPERTHRESHOLD;
              }
            }
        }
        else
        {
            if (m_InputImageNode.IsNotNull())
            {

                m_LOWERTHRESHOLD = m_Controls.m_LowerThresholdSpinBox->value();
                m_UPPERTHRESHOLD = m_Controls.m_UpperThresholdSpinBox->value();
            }
        }
    }
}

//void QmitkAdaptiveRegionGrowingView::TimeStepChanged()
//{

//}

void QmitkAdaptiveRegionGrowingView::RunSegmentation()
{
  
  if (m_InputImageNode.IsNull())
  {
    QMessageBox::information( NULL, "Adaptive Region Growing functionality", "Please specify the image in Datamanager!");
    return;
  }

  //release the "define seed point"-button if it is still pressed
  if (m_Controls.m_pbDefineSeedPoint->isChecked())
    m_Controls.m_pbDefineSeedPoint->toggle();

 
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);

  if (node.IsNull())
  {
    QMessageBox::information( NULL, "Adaptive Region Growing functionality", "Please insert a seed point inside the image.\n\nFirst press the \"Define Seed Point\" button,\nthen click left mouse button inside the image.");
    return;
  }

  //safety if no pointSet or pointSet empty
  mitk::PointSet::Pointer seedPointSet = dynamic_cast<mitk::PointSet*> (node->GetData());
  if (seedPointSet.IsNull())
  {
    m_Controls.m_pbRunSegmentation->setEnabled(true);
    QMessageBox::information( NULL, "Adaptive Region Growing functionality", "The seed point is empty! Please choose a new seed point.");
    return;
  }
  if (!(seedPointSet->GetSize(m_MultiWidget->GetTimeNavigationController()->GetTime()->GetPos()) > 0))
  {
    m_Controls.m_pbRunSegmentation->setEnabled(true);
    m_Controls.m_pbDefineSeedPoint->setHidden(false);
    QMessageBox::information( NULL, "Adaptive Region Growing functionality", "The seed point is empty! Please choose a new seed point.");
    return;
  }

  int timeStep = m_MultiWidget->GetTimeNavigationController()->GetTime()->GetPos();
  mitk::PointSet::PointType seedPoint = seedPointSet->GetPointSet(timeStep)->GetPoints()->Begin().Value();

  mitk::Image::Pointer orgImage = dynamic_cast<mitk::Image*> (m_InputImageNode->GetData());

  if (orgImage.IsNotNull())
  {
      if (orgImage->GetDimension() == 4)
      {
          mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
          timeSelector->SetInput(orgImage);
          timeSelector->SetTimeNr( timeStep );
          timeSelector->UpdateLargestPossibleRegion();
          MITK_INFO<<"Time= "<<timeStep;
          mitk::Image* timedImage = timeSelector->GetOutput();
          AccessByItk_2( timedImage , StartRegionGrowing, timedImage->GetGeometry(), seedPoint);
      }
      else if (orgImage->GetDimension() == 3)
      {
          QApplication::setOverrideCursor(QCursor(Qt::WaitCursor)); //set the cursor to waiting
          AccessByItk_2(orgImage, StartRegionGrowing, orgImage->GetGeometry(), seedPoint);
          QApplication::restoreOverrideCursor();//reset cursor
      }
      else
      {
          QMessageBox::information( NULL, "Adaptive Region Growing functionality", "Only images of dimension 3 or 4 can be processed!");
          return;
      }
  }

}

template<typename TPixel, unsigned int VImageDimension>
void QmitkAdaptiveRegionGrowingView::StartRegionGrowing(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Geometry3D* imageGeometry, mitk::PointSet::PointType seedPoint)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef typename InputImageType::IndexType IndexType;
  typedef itk::ConnectedAdaptiveThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();
  typedef itk::MinimumMaximumImageCalculator<InputImageType> MinMaxValueFilterType;

    IndexType seedIndex2;
    imageGeometry->WorldToIndex( seedPoint, seedIndex2);

    MITK_INFO<<"Worldpoint: "<<seedPoint<<", IndexPoint: "<<seedIndex2;

  if ( !imageGeometry->IsInside(seedPoint) )
  {
    QApplication::restoreOverrideCursor();//reset cursor to be able to click ok with the regular mouse cursor
    QMessageBox::information( NULL, "Segmentation functionality", "The seed point is outside of the image! Please choose a position inside the image!");
    return;
  }

  IndexType seedIndex;
  imageGeometry->WorldToIndex( seedPoint, seedIndex);// convert world coordinates to image indices

  mitk::ScalarType seedValue = itkImage->GetPixel(seedIndex);
  typename MinMaxValueFilterType::Pointer minMaxFilter = MinMaxValueFilterType::New();
  minMaxFilter->SetImage(itkImage);
  minMaxFilter->ComputeMinimum();
  minMaxFilter->ComputeMaximum();

  mitk::ScalarType min = minMaxFilter->GetMinimum();
  mitk::ScalarType max = minMaxFilter->GetMaximum();
  mitk::ScalarType windowSize = max - min;
  mitk::LevelWindow window;
  m_InputImageNode->GetLevelWindow(window, NULL, "levelwindow");
  MITK_INFO << "Range by window: " << window.GetRangeMax() << " Range by ITK: " <<windowSize;
  windowSize = 0.15*windowSize;

  if (seedValue - 0.5*windowSize < min)
  {
      m_LOWERTHRESHOLD = min;
  }
  else
  {
      m_LOWERTHRESHOLD = seedValue - 0.5*windowSize;
  }

  if (seedValue + 0.5*windowSize > max)
  {
      m_UPPERTHRESHOLD = max;
  }
  else
  {
      m_UPPERTHRESHOLD = seedValue + 0.5*windowSize;
  }



  MITK_INFO<<"SeedpointValue: "<<seedValue<<", Lower/Upper: "<<m_LOWERTHRESHOLD<<"/"<<m_UPPERTHRESHOLD;
  if (seedValue>m_UPPERTHRESHOLD || seedValue<m_LOWERTHRESHOLD)
  {
    QApplication::restoreOverrideCursor();//reset cursor to be able to click ok with the regular mouse cursor
    QMessageBox::information( NULL, "Segmentation functionality", "The seed point is outside the defined thresholds! Please set a new seed point or adjust the thresholds.");
    return;
  }

  regionGrower->SetInput( itkImage );
  regionGrower->AddSeed( seedIndex );
  regionGrower->SetLower( m_LOWERTHRESHOLD );
  regionGrower->SetUpper( m_UPPERTHRESHOLD );
  regionGrower->SetGrowingDirectionIsUpwards( m_CurrentRGDirectionIsUpwards );

  try
  {
    regionGrower->Update();
  }
    catch(itk::ExceptionObject &exc)
  {
    QMessageBox errorInfo;
    errorInfo.setWindowTitle("Vesseltree Segmentation Functionality");
    errorInfo.setIcon(QMessageBox::Critical);
    errorInfo.setText("An error occurred during region growing!");
    errorInfo.setDetailedText(exc.what());
    errorInfo.exec();
    return; // can't work
  }
  catch( ... )
  {
    QMessageBox::critical( NULL, "Vesseltree Segmentation Functionality", "An error occurred during region growing!");
    return;
  }

  this->m_SeedpointValue = regionGrower->GetSeedpointValue();

  //initialize slider
  if(m_CurrentRGDirectionIsUpwards)
  {
    this->m_Controls.m_Slider->setMinimum(m_SeedpointValue);
    this->m_Controls.m_Slider->setMaximum(m_UPPERTHRESHOLD);
  }
  else
  {
    m_Controls.m_Slider->setMinimum(m_LOWERTHRESHOLD);
    m_Controls.m_Slider->setMaximum(m_SeedpointValue);
  }
  this->m_SliderInitialized = true;
  this->m_DetectedLeakagePoint = regionGrower->GetLeakagePoint();
  MITK_INFO<<"Detected LeakagePoint: "<<m_DetectedLeakagePoint;

  mitk::Image::Pointer resultImage = mitk::ImportItkImage( regionGrower->GetOutput() );

  //create new node and then delete the old one if there is one
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData( resultImage );

  // set some properties
  newNode->SetProperty("name", mitk::StringProperty::New(m_NAMEFORLABLEDSEGMENTATIONIMAGE));
  //newNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  newNode->SetProperty("color", mitk::ColorProperty::New(1.0,0.0,0.0));
  newNode->SetProperty("layer", mitk::IntProperty::New(1));
  newNode->SetProperty("opacity", mitk::FloatProperty::New(0.7));

  //delete the old image, if there was one:
  mitk::DataNode::Pointer binaryNode = m_DataStorage->GetNamedNode(m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  m_DataStorage->Remove(binaryNode);

  // now add result to data tree
  m_DataStorage->Add( newNode, m_InputImageNode );

  this->InitializeLevelWindow();

  if(m_UseVolumeRendering)
    this->EnableVolumeRendering(true);

  m_UpdateSuggestedThreshold = true;// reset first stored threshold value

}

void QmitkAdaptiveRegionGrowingView::InitializeLevelWindow()
{
  //get the preview from the datatree
  mitk::DataNode::Pointer newNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);

  mitk::LevelWindow tempLevelWindow;
  newNode->GetLevelWindow(tempLevelWindow, NULL, "levelwindow");

  mitk::ScalarType* level = new mitk::ScalarType(0.0);
  mitk::ScalarType* window = new mitk::ScalarType(1.0);

  int upper;
  if (m_CurrentRGDirectionIsUpwards)
  {
    upper = m_UPPERTHRESHOLD - m_SeedpointValue;
  }
  else
  {
    upper = m_SeedpointValue - m_LOWERTHRESHOLD;
  }

  tempLevelWindow.SetRangeMinMax(mitk::ScalarType(0), mitk::ScalarType(upper));
  tempLevelWindow.SetLevelWindow(*level, *window);

  newNode->SetLevelWindow(tempLevelWindow, NULL, "levelwindow");
  //update the widgets
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  //get the suggested threshold from the detected leakage-point and adjust the slider

  if (m_CurrentRGDirectionIsUpwards)
  {
    this->m_Controls.m_Slider->setValue(this->m_SeedpointValue + this->m_DetectedLeakagePoint -1);
  }
  else
  {
    this->m_Controls.m_Slider->setValue(this->m_SeedpointValue - this->m_DetectedLeakagePoint +1);
  }


  m_SliderInitialized = true;

  //inquiry need to fix bug#1828
  static int lastSliderPosition = 0;
  if ((this->m_SeedpointValue + this->m_DetectedLeakagePoint - 1) == lastSliderPosition)
  {
    //this->ChangeLevelWindow(lastSliderPosition);
  }
  lastSliderPosition = this->m_SeedpointValue + this->m_DetectedLeakagePoint-1;

}

void QmitkAdaptiveRegionGrowingView::ChangeLevelWindow(int newValue)
{
  if (m_SliderInitialized)
  {
    //do nothing, if no preview exists
    mitk::DataNode::Pointer newNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
    if (newNode.IsNull())
      return;

    mitk::LevelWindow tempLevelWindow;

    newNode->GetLevelWindow(tempLevelWindow, NULL, "levelwindow"); //get the levelWindow associated with the preview

    mitk::ScalarType level;// = this->m_UPPERTHRESHOLD - newValue + 0.5;
    mitk::ScalarType* window = new mitk::ScalarType(1);

    //adjust the levelwindow according to the position of the slider (newvalue)
    if (m_CurrentRGDirectionIsUpwards)
    {
      level = m_UPPERTHRESHOLD - newValue + 0.5;
      tempLevelWindow.SetLevelWindow(level, *window);
    }
    else
    {
      level = newValue - m_LOWERTHRESHOLD +0.5;
      tempLevelWindow.SetLevelWindow(level, *window);
    }

    //MITK_INFO<<"Lower/Upper/Level Window Bounds: "<<tempLevelWindow.GetLowerWindowBound()<<", "<<tempLevelWindow.GetUpperWindowBound()<<", "<<tempLevelWindow.GetLevel();

    newNode->SetLevelWindow(tempLevelWindow, NULL, "levelwindow");

    if (m_UseVolumeRendering)
    this->UpdateVolumeRenderingThreshold((int) (level - 0.5));//lower threshold for labeled image
    this->m_Controls.m_SliderValueLabel->setNum(newValue);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkAdaptiveRegionGrowingView::DecreaseSlider()
{
  //moves the slider one step to the left, when the "-"-button is pressed
  if (this->m_Controls.m_Slider->value() != this->m_Controls.m_Slider->minimum())
  {
    int newValue = this->m_Controls.m_Slider->value() - 1;
    this->ChangeLevelWindow(newValue);
    this->m_Controls.m_Slider->setValue(newValue);
  }
}

void QmitkAdaptiveRegionGrowingView::IncreaseSlider()
{
  //moves the slider one step to the right, when the "+"-button is pressed
  if (this->m_Controls.m_Slider->value() != this->m_Controls.m_Slider->maximum())
  {
    int newValue = this->m_Controls.m_Slider->value() + 1;
    this->ChangeLevelWindow(newValue);
    this->m_Controls.m_Slider->setValue(newValue);
  }
}

void QmitkAdaptiveRegionGrowingView::ConfirmSegmentation()
{
  //get image node
  if(m_InputImageNode.IsNull())
  {
    QMessageBox::critical( NULL, "Adaptive region growing functionality", "Please specify the image in Datamanager!");
    return;
  }
  //get image data
  mitk::Image::Pointer orgImage = dynamic_cast<mitk::Image*> (m_InputImageNode->GetData());
  if(orgImage.IsNull())
  {
    QMessageBox::critical( NULL, "Adaptive region growing functionality", "No Image found!");
    return;
  }
  //get labeled segmentation
  mitk::Image::Pointer labeledSeg = (mitk::Image*)m_DataStorage->GetNamedObject<mitk::Image>(m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if(labeledSeg.IsNull())
  {
    QMessageBox::critical( NULL, "Adaptive region growing functionality", "No Segmentation Preview found!");
    return;
  }


//  mitk::LabeledSegmentationImageToVesselTreeWrapper::Pointer wrapper = mitk::LabeledSegmentationImageToVesselTreeWrapper::New();
//  //mitk::LabeledSegmentationImageToSurfaceWrapper::Pointer wrapper = mitk::LabeledSegmentationImageToSurfaceWrapper::New();

//  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
//  mitk::Image::Pointer labledSegementationImage = dynamic_cast<mitk::Image*> (node->GetData());

//  std::string name = m_InputImageNode->GetPropertyList()->GetProperty("name")->GetValueAsString();
//  const char* orgImageName;
//  orgImageName = name.c_str();
//  wrapper->SetDataNodeNamesSetDataNodeNames(m_NAMEFORLABLEDSEGMENTATIONIMAGE,  m_NAMEFORBINARYIMAGE, m_NAMEFORSURFACE, m_NAMEFORVESSELTREE, m_NAMEFORSEEDPOINT, orgImageName);
//  //wrapper->SetDataNodeNamesSetDataNodeNames(m_NAMEFORLABLEDSEGMENTATIONIMAGE, m_NAMEFORBINARYIMAGE, m_NAMEFORSURFACE);
//  wrapper->SetDataStorage(m_DataStorage);

//  mitk::LevelWindow tempLevelWindow;
//  m_DataStorage->GetNamedNode(m_NAMEFORLABLEDSEGMENTATIONIMAGE)->GetLevelWindow( tempLevelWindow, NULL, "levelWindow");
//  int upperThresholdLabeledImage = (short int) tempLevelWindow.GetRangeMax();
//  int lowerThresholdLabeledImage = (short int) tempLevelWindow.GetLowerWindowBound() + 1;
//  node->SetFloatProperty("lowerThreshold", lowerThresholdLabeledImage);
//  node->SetFloatProperty("upperThreshold", upperThresholdLabeledImage);
//  wrapper->SetThresholdsLabeledImage(lowerThresholdLabeledImage, upperThresholdLabeledImage);
//  wrapper->SetOrganType(m_OrganType);

//  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));//change the cursor to waiting

//  try
//  {
//    wrapper->GenerateData();
//  }
//  catch(itk::ExceptionObject &exc)
//  {
//    QMessageBox errorInfo;
//    errorInfo.setWindowTitle("Segmentation functionality");
//    errorInfo.setIcon(QMessageBox::Critical);
//    errorInfo.setText("An error occurred during vessel tree or surface generation!");
//    errorInfo.setDetailedText(exc.what());
//    errorInfo.exec();
    
//    QApplication::restoreOverrideCursor(); //reset the cursor
//    mitk::ProgressBar::GetInstance()->AddStepsToDo(1);
//    mitk::ProgressBar::GetInstance()->Progress(1);

//    return; // can't work
//  }
//  catch (...)
//  {
//    QMessageBox::critical(NULL, "Segmentation functionality", "An error occurred during vessel tree or surface generation!");
    
//    QApplication::restoreOverrideCursor(); //reset the cursor
//    mitk::ProgressBar::GetInstance()->AddStepsToDo(1);
//    mitk::ProgressBar::GetInstance()->Progress(1);
//    return;
//  }

//  QApplication::restoreOverrideCursor(); //reset the cursor

//  this->EnableVolumeRendering(false);

//  m_Controls.m_cbVolumeRendering->setChecked(false);

//  if(m_MultiWidget)
//    m_MultiWidget->SetWidgetPlanesVisibility(false); // hide planes in 3D widget

//  emit EntireVesselTreeDeleted(true); //notify boosting view to reset thresholds

}

void QmitkAdaptiveRegionGrowingView::EnableControls(bool enable)
{
  //set the labels below the slider
  this->m_Controls.m_RSliderLabelLower->setText("Shrink");
  this->m_Controls.m_RGSliderLaberUpper->setText("Expand");

  this->m_Controls.m_RSliderLabelLower->setEnabled(enable);
  this->m_Controls.m_RGSliderLaberUpper->setEnabled(enable);
  this->m_Controls.m_pbRunSegmentation->setEnabled(enable);
  this->m_Controls.m_DecreaseTH->setEnabled(enable);
  this->m_Controls.m_IncreaseTH->setEnabled(enable);
  this->m_Controls.m_Slider->setEnabled(enable);
  this->m_Controls.m_pbConfirmSegementation->setEnabled(enable);
}

void QmitkAdaptiveRegionGrowingView::EnableVolumeRendering(bool enable)
{
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);

  if(node.IsNull())
    return;

  if(m_MultiWidget)
     m_MultiWidget->SetWidgetPlanesVisibility(!enable);


  if (enable)
  {
    //node->SetMapper(mitk::BaseRenderer::Standard3D, mitk::GPUVolumeMapper3D::New());
    node->SetBoolProperty("volumerendering", enable);
    node->SetBoolProperty("volumerendering.uselod", true);
  }
  else
  {
    node->SetBoolProperty("volumerendering", enable);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkAdaptiveRegionGrowingView::UpdateVolumeRenderingThreshold(int thValue)
{
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);

  mitk::TransferFunction::Pointer tf = mitk::TransferFunction::New();
      
  if (m_UpdateSuggestedThreshold)
  { 
    m_SuggestedThValue = thValue;
    m_UpdateSuggestedThreshold = false;
  }

  // grayvalue->opacity
  {
    vtkPiecewiseFunction *f = tf->GetScalarOpacityFunction();
    f->RemoveAllPoints();
    f->AddPoint(0, 0);
    f->AddPoint(thValue+0.5, 0);
    f->AddPoint(thValue+1.5, 1);
    f->AddPoint(1000, 1);
    f->ClampingOn();
    f->Modified();
  }

  // grayvalue->color
  {
    float a = 255.0;
    vtkColorTransferFunction *ctf = tf->GetColorTransferFunction();
    ctf->RemoveAllPoints();
    //ctf->AddRGBPoint(-1000, 0.0, 0.0, 0.0);
    ctf->AddRGBPoint(m_SuggestedThValue+1, 203/a, 104/a, 102/a);       
    ctf->AddRGBPoint(m_SuggestedThValue, 255/a, 0/a, 0/a);    
    ctf->ClampingOn();
    ctf->Modified();
  }

  // GradientOpacityFunction
  {
    vtkPiecewiseFunction *gof = tf->GetGradientOpacityFunction();
    gof->RemoveAllPoints();
    gof->AddPoint(-10000, 1);
    gof->AddPoint(10000, 1);
    gof->ClampingOn();
    gof->Modified();
  }

  mitk::TransferFunctionProperty::Pointer tfp = mitk::TransferFunctionProperty::New();
  tfp->SetValue(tf);
  node->SetProperty("TransferFunction", tfp);

}

void QmitkAdaptiveRegionGrowingView::UseVolumeRendering(bool on)
{
  m_UseVolumeRendering = on;

  this->EnableVolumeRendering(on);
}

void QmitkAdaptiveRegionGrowingView::OnDefineThresholdBoundaries(bool status)
{
    m_Controls.m_LowerThresholdSpinBox->setEnabled(status);
    m_Controls.m_UpperThresholdSpinBox->setEnabled(status);
    m_Controls.lb_LowerTh->setEnabled(status);
    m_Controls.lb_UpperTh->setEnabled(status);
}

void QmitkAdaptiveRegionGrowingView::SetLowerThresholdValue( int lowerThreshold )
{
  m_LOWERTHRESHOLD = lowerThreshold;
}

void QmitkAdaptiveRegionGrowingView::SetUpperThresholdValue( int upperThreshold)
{
  m_UPPERTHRESHOLD = upperThreshold;
}

void QmitkAdaptiveRegionGrowingView::SetRegionGrowingDirectionUpwards(bool status)
{
    this->m_CurrentRGDirectionIsUpwards = status;
    //MITK_INFO<<"DirectionUp: "<<status;
}

void QmitkAdaptiveRegionGrowingView::Deactivated()
{
  this->DeactivateSeedPointMode();
}

void QmitkAdaptiveRegionGrowingView::Activated()
{
  if(m_Controls.m_pbDefineSeedPoint->isChecked())
  {
    this->ActivateSeedPointMode();
  }
}

void QmitkAdaptiveRegionGrowingView::ActivateSeedPointMode()
{
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);
  if(node.IsNotNull())
  {
    //set the cursor to cross
    QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));

    mitk::PointSetInteractor::Pointer interactor = dynamic_cast<mitk::PointSetInteractor*> (node->GetInteractor());
    if (interactor.IsNull())
    {
      //create a new interactor and add it to node
      interactor = mitk::PointSetInteractor::New("singlepointinteractorwithoutshiftclick", node, 1);
    }
    mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);
  }
}

void QmitkAdaptiveRegionGrowingView::DeactivateSeedPointMode()
{
  //set the cursor to default again
  QApplication::restoreOverrideCursor();

  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);
  if(node.IsNotNull())
  {
    mitk::PointSetInteractor::Pointer
      interactor = dynamic_cast<mitk::PointSetInteractor*> (node->GetInteractor());
    if (interactor.IsNotNull())
    {
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor(interactor);
    }
  }
}
