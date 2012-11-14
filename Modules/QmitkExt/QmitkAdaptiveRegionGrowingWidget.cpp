/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#include "QmitkAdaptiveRegionGrowingWidget.h"

#include "QmitkStdMultiWidget.h"

#include <qmessagebox.h>

#include "mitkNodePredicateDataType.h"
#include "mitkGlobalInteraction.h"
#include "mitkPointSetInteractor.h"
#include "mitkProperties.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkImageTimeSelector.h"

#include "mitkImageStatisticsHolder.h"

#include <itkConnectedAdaptiveThresholdImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkImageIterator.h>

QmitkAdaptiveRegionGrowingWidget::QmitkAdaptiveRegionGrowingWidget(QWidget * parent) :
QWidget(parent), m_MultiWidget(NULL), m_UseVolumeRendering(false), m_UpdateSuggestedThreshold(true), m_SuggestedThValue(0.0)
{
  m_Controls.setupUi(this);

  // muellerm, 8.8.12.: assure no limits for thresholding (there can be images with pixel values above 4000!!!)
  m_Controls.m_LowerThresholdSpinBox->setMinimum( std::numeric_limits<int>::min() );
  m_Controls.m_UpperThresholdSpinBox->setMinimum( std::numeric_limits<int>::min() );

  m_Controls.m_LowerThresholdSpinBox->setMaximum( std::numeric_limits<int>::max() );
  m_Controls.m_UpperThresholdSpinBox->setMaximum( std::numeric_limits<int>::max() );

  m_NAMEFORSEEDPOINT = "Seed Point";
  this->CreateConnections();
  this->SetDataNodeNames("labeledRGSegmentation","RGResult","RGFeedbackSurface","RGSeedpoint");

}

QmitkAdaptiveRegionGrowingWidget::~QmitkAdaptiveRegionGrowingWidget()
{
    //Removing the observer of the PointSet node
    mitk::DataNode* node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);
    if (node != NULL)
    {
        this->DeactivateSeedPointMode();
        dynamic_cast<mitk::PointSet*>(node->GetData())->RemoveObserver(m_PointSetAddObserverTag);
    }

    this->RemoveHelperNodes();

}

void QmitkAdaptiveRegionGrowingWidget::RemoveHelperNodes()
{
  mitk::DataNode::Pointer seedNode = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);
  if( seedNode.IsNotNull() )
  {
    m_DataStorage->Remove(seedNode);
  }

  mitk::DataNode::Pointer imageNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if( imageNode.IsNotNull() )
  {
    m_DataStorage->Remove(imageNode);
  }
}

void QmitkAdaptiveRegionGrowingWidget::CreateConnections()
{
    //Connecting GUI components
    connect( (QObject*) (m_Controls.m_pbDefineSeedPoint), SIGNAL( toggled(bool) ), this, SLOT( SetSeedPointToggled(bool)) );
    connect( (QObject*) (m_Controls.m_pbRunSegmentation), SIGNAL(clicked()), this, SLOT(RunSegmentation()));
    connect( (QObject*) (m_Controls.m_Slider), SIGNAL(valueChanged(int)), this, SLOT(ChangeLevelWindow(int)) );
    connect( (QObject*) (m_Controls.m_DecreaseTH), SIGNAL(clicked()), this, SLOT(DecreaseSlider()));
    connect( (QObject*) (m_Controls.m_IncreaseTH), SIGNAL(clicked()), this,SLOT(IncreaseSlider()));
    connect( (QObject*) (m_Controls.m_pbConfirmSegementation), SIGNAL(clicked()), this, SLOT(ConfirmSegmentation()));
    connect( (QObject*) (m_Controls.m_cbVolumeRendering), SIGNAL(toggled(bool)), this, SLOT(UseVolumeRendering(bool) ));

    connect( m_Controls.m_LowerThresholdSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SetLowerThresholdValue(double)));
    connect( m_Controls.m_UpperThresholdSpinBox, SIGNAL(valueChanged(double)), this, SLOT(SetUpperThresholdValue(double)));
}

void QmitkAdaptiveRegionGrowingWidget::SetDataNodeNames(std::string labledSegmentation, std::string binaryImage, std::string surface, std::string seedPoint)
{
  m_NAMEFORLABLEDSEGMENTATIONIMAGE = labledSegmentation;
  m_NAMEFORBINARYIMAGE = binaryImage;
  m_NAMEFORSURFACE = surface;
  m_NAMEFORSEEDPOINT = seedPoint;
}

void QmitkAdaptiveRegionGrowingWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkAdaptiveRegionGrowingWidget::SetMultiWidget(QmitkStdMultiWidget* multiWidget)
{
  m_MultiWidget = multiWidget;
}

void QmitkAdaptiveRegionGrowingWidget::SetInputImageNode(mitk::DataNode* node)
{
  m_InputImageNode = node;
}

void QmitkAdaptiveRegionGrowingWidget::SetSeedPointToggled(bool toggled)
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

    //Watch for point added or modified
    itk::SimpleMemberCommand<QmitkAdaptiveRegionGrowingWidget>::Pointer pointAddedCommand = itk::SimpleMemberCommand<QmitkAdaptiveRegionGrowingWidget>::New();
    pointAddedCommand->SetCallbackFunction(this, &QmitkAdaptiveRegionGrowingWidget::OnPointAdded);
    m_PointSetAddObserverTag = pointSet->AddObserver( mitk::PointSetAddEvent(), pointAddedCommand);

    //add to DataStorage
    m_DataStorage->Add(pointSetNode, m_InputImageNode);
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
      QMessageBox::critical(NULL, "QmitkAdaptiveRegionGrowingWidget", "No seed point node found!");
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

void QmitkAdaptiveRegionGrowingWidget::OnPointAdded()
{
  mitk::DataNode* node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);

  if (node != NULL) //no pointSet present
  {
      mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());

      if (pointSet.IsNull())
      {
          QMessageBox::critical(NULL, "QmitkAdaptiveRegionGrowingWidget", "PointSetNode does not contain a pointset");
          return;
      }

      mitk::Image* image = dynamic_cast<mitk::Image*>(m_InputImageNode->GetData());

      mitk::Point3D seedPoint = pointSet->GetPointSet(m_MultiWidget->GetTimeNavigationController()->GetTime()->GetPos())->GetPoints()->ElementAt(0);

      m_SeedpointValue = image->GetPixelValueByWorldCoordinate(seedPoint);

      /* In this case the seedpoint is placed e.g. in the lung or bronchialtree
       * The lowerFactor sets the windowsize depending on the regiongrowing direction
       */
      m_CurrentRGDirectionIsUpwards = true;
      if (m_SeedpointValue < -500)
      {
          m_CurrentRGDirectionIsUpwards = false;
      }

      m_SeedPointValueMean = 0;

      mitk::Index3D currentIndex, runningIndex;
      mitk::ScalarType pixelValues[125];
      unsigned int pos (0);

      image->GetGeometry(0)->WorldToIndex(seedPoint, currentIndex);
      runningIndex = currentIndex;

      for(int i = runningIndex[0]-2; i <= runningIndex[0]+2; i++)
      {
        for(int j = runningIndex[1]-2; j <= runningIndex[1]+2; j++)
        {
          for(int k = runningIndex[2]-2; k <= runningIndex[2]+2; k++)
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

      //Now calculation mean of the pixelValues
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

      /*
       * Here the upper- and lower threshold is calculated:
       * The windowSize is 20% of the maximum range of the intensity values existing in the current image
       * If the RG direction is upwards the lower TH is meanSeedValue-0.15*windowSize and upper TH is meanSeedValue+0.85*windowsSize
       * if the RG direction is downwards the lower TH is meanSeedValue-0.85*windowSize and upper TH is meanSeedValue+0.15*windowsSize
      */
      mitk::ScalarType min = image->GetStatistics()->GetScalarValueMin();
      mitk::ScalarType max = image->GetStatistics()->GetScalarValueMax();
      mitk::ScalarType windowSize = max - min;

      windowSize = 0.15*windowSize;

      if (m_CurrentRGDirectionIsUpwards)
      {
        m_LOWERTHRESHOLD = m_SeedPointValueMean;
        if (m_SeedpointValue < m_SeedPointValueMean)
          m_LOWERTHRESHOLD = m_SeedpointValue;
        m_UPPERTHRESHOLD = m_SeedpointValue + windowSize;
      }
      else
      {
        m_UPPERTHRESHOLD = m_SeedPointValueMean;
        if (m_SeedpointValue > m_SeedPointValueMean)
          m_UPPERTHRESHOLD = m_SeedpointValue;
        m_LOWERTHRESHOLD = m_SeedpointValue - windowSize;
      }

      this->m_Controls.m_LowerThresholdSpinBox->setValue(m_LOWERTHRESHOLD);
      this->m_Controls.m_UpperThresholdSpinBox->setValue(m_UPPERTHRESHOLD);
  }
}



void QmitkAdaptiveRegionGrowingWidget::RunSegmentation()
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
  EnableControls(true); // Segmentation ran successfully, so enable all controls.
  node->SetVisibility(true);
}

template<typename TPixel, unsigned int VImageDimension>
void QmitkAdaptiveRegionGrowingWidget::StartRegionGrowing(itk::Image<TPixel, VImageDimension>* itkImage, mitk::Geometry3D* imageGeometry, mitk::PointSet::PointType seedPoint)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef typename InputImageType::IndexType IndexType;
  typedef itk::ConnectedAdaptiveThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();
  typedef itk::MinimumMaximumImageCalculator<InputImageType> MinMaxValueFilterType;

  if ( !imageGeometry->IsInside(seedPoint) )
  {
    QApplication::restoreOverrideCursor();//reset cursor to be able to click ok with the regular mouse cursor
    QMessageBox::information( NULL, "Segmentation functionality", "The seed point is outside of the image! Please choose a position inside the image!");
    return;
  }

  IndexType seedIndex;
  imageGeometry->WorldToIndex( seedPoint, seedIndex);// convert world coordinates to image indices



  if (m_SeedpointValue>m_UPPERTHRESHOLD || m_SeedpointValue<m_LOWERTHRESHOLD)
  {
    QApplication::restoreOverrideCursor();//reset cursor to be able to click ok with the regular mouse cursor
    QMessageBox::information( NULL, "Segmentation functionality", "The seed point is outside the defined thresholds! Please set a new seed point or adjust the thresholds.");
    MITK_INFO << "Mean: " <<m_SeedPointValueMean;
    return;
  }

  //Setting the direction of the regiongrowing. For dark structures e.g. the lung the regiongrowing
  //is performed starting at the upper value going to the lower one
  regionGrower->SetGrowingDirectionIsUpwards( m_CurrentRGDirectionIsUpwards );
  regionGrower->SetInput( itkImage );
  regionGrower->AddSeed( seedIndex );
  //In some cases we have to subtract 1 for the lower threshold and add 1 to the upper.
  //Otherwise no region growing is done. Maybe a bug in the ConnectiveAdaptiveThresholdFilter
  regionGrower->SetLower( m_LOWERTHRESHOLD-1 );
  regionGrower->SetUpper( m_UPPERTHRESHOLD+1);

  try
  {
    regionGrower->Update();
  }
    catch(itk::ExceptionObject &exc)
  {
    QMessageBox errorInfo;
    errorInfo.setWindowTitle("Adaptive RG Segmentation Functionality");
    errorInfo.setIcon(QMessageBox::Critical);
    errorInfo.setText("An error occurred during region growing!");
    errorInfo.setDetailedText(exc.what());
    errorInfo.exec();
    return; // can't work
  }
  catch( ... )
  {
    QMessageBox::critical( NULL, "Adaptive RG Segmentation Functionality", "An error occurred during region growing!");
    return;
  }


  this->m_SeedpointValue = regionGrower->GetSeedpointValue();

  //initialize slider
  if(m_CurrentRGDirectionIsUpwards)
  {
    this->m_Controls.m_Slider->setMinimum(m_LOWERTHRESHOLD);
    this->m_Controls.m_Slider->setMaximum(m_UPPERTHRESHOLD);
  }
  else
  {
    m_Controls.m_Slider->setMinimum(m_LOWERTHRESHOLD);
    m_Controls.m_Slider->setMaximum(m_UPPERTHRESHOLD);
  }
  this->m_SliderInitialized = true;
  this->m_DetectedLeakagePoint = regionGrower->GetLeakagePoint();

  mitk::Image::Pointer resultImage = mitk::ImportItkImage( regionGrower->GetOutput() );

  //create new node and then delete the old one if there is one
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData( resultImage );

  // set some properties
  newNode->SetProperty("name", mitk::StringProperty::New(m_NAMEFORLABLEDSEGMENTATIONIMAGE));
  newNode->SetProperty("helper object", mitk::BoolProperty::New(true));
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
  //Setting progress to finished
  mitk::ProgressBar::GetInstance()->Progress(357);
}

void QmitkAdaptiveRegionGrowingWidget::InitializeLevelWindow()
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

  //get the suggested threshold from the detected leakage-point and adjust the slider

  if (m_CurrentRGDirectionIsUpwards)
  {
    this->m_Controls.m_Slider->setValue(this->m_SeedpointValue + this->m_DetectedLeakagePoint -1);
    *level = m_UPPERTHRESHOLD - (this->m_SeedpointValue + this->m_DetectedLeakagePoint -1) + 0.5;
  }
  else
  {
    this->m_Controls.m_Slider->setValue(this->m_SeedpointValue - this->m_DetectedLeakagePoint +1);
    *level = (this->m_SeedpointValue + this->m_DetectedLeakagePoint +1) - m_LOWERTHRESHOLD + 0.5;
  }

  tempLevelWindow.SetLevelWindow(*level, *window);
  newNode->SetLevelWindow(tempLevelWindow, NULL, "levelwindow");
  //update the widgets
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  m_SliderInitialized = true;

  //inquiry need to fix bug#1828
  static int lastSliderPosition = 0;
  if ((this->m_SeedpointValue + this->m_DetectedLeakagePoint - 1) == lastSliderPosition)
  {
    this->ChangeLevelWindow(lastSliderPosition);
  }
  lastSliderPosition = this->m_SeedpointValue + this->m_DetectedLeakagePoint-1;

  this->m_MultiWidget->levelWindowWidget->GetManager()->SetAutoTopMostImage(false);
  this->m_MultiWidget->levelWindowWidget->GetManager()->SetLevelWindowProperty(static_cast<mitk::LevelWindowProperty*>(newNode->GetProperty("levelwindow")));

  if (m_UseVolumeRendering)
  this->UpdateVolumeRenderingThreshold((int) (*level + 0.5));//lower threshold for labeled image

}

void QmitkAdaptiveRegionGrowingWidget::ChangeLevelWindow(int newValue)
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

    newNode->SetLevelWindow(tempLevelWindow, NULL, "levelwindow");

    if (m_UseVolumeRendering)
    this->UpdateVolumeRenderingThreshold((int) (level - 0.5));//lower threshold for labeled image
    this->m_Controls.m_SliderValueLabel->setNum(newValue);
    newNode->SetVisibility(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkAdaptiveRegionGrowingWidget::DecreaseSlider()
{
  //moves the slider one step to the left, when the "-"-button is pressed
  if (this->m_Controls.m_Slider->value() != this->m_Controls.m_Slider->minimum())
  {
    int newValue = this->m_Controls.m_Slider->value() - 1;
    this->ChangeLevelWindow(newValue);
    this->m_Controls.m_Slider->setValue(newValue);
  }
}

void QmitkAdaptiveRegionGrowingWidget::IncreaseSlider()
{
  //moves the slider one step to the right, when the "+"-button is pressed
  if (this->m_Controls.m_Slider->value() != this->m_Controls.m_Slider->maximum())
  {
    int newValue = this->m_Controls.m_Slider->value() + 1;
    this->ChangeLevelWindow(newValue);
    this->m_Controls.m_Slider->setValue(newValue);
  }
}

void QmitkAdaptiveRegionGrowingWidget::ConfirmSegmentation()
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

  mitk::DataNode::Pointer newNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if (newNode.IsNull())
    return;

  mitk::Image* img = dynamic_cast<mitk::Image*>(newNode->GetData());
  AccessByItk(img, ITKThresholding);

  // disable volume rendering preview after the segmentation node was created
  this->EnableVolumeRendering(false);
  newNode->SetVisibility(false);
  m_Controls.m_cbVolumeRendering->setChecked(false);

}

template<typename TPixel, unsigned int VImageDimension>
void QmitkAdaptiveRegionGrowingWidget::ITKThresholding(itk::Image<TPixel, VImageDimension>* itkImage)
{
    /*
    typedef itk::Image<TPixel, VImageDimension> InputImageType;
    typedef itk::Image<unsigned char, VImageDimension> SegmentationType;
    typedef itk::BinaryThresholdImageFilter<InputImageType, SegmentationType> ThresholdFilterType;

    typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
    filter->SetInput(itkImage);
    filter->SetInsideValue(1);
    filter->SetOutsideValue(0);

    mitk::DataNode::Pointer newNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
    if (newNode.IsNull())
      return;

    mitk::LevelWindow tempLevelWindow;

    newNode->GetLevelWindow(tempLevelWindow, NULL, "levelwindow"); //get the levelWindow associated with the preview

    filter->SetUpperThreshold(tempLevelWindow.GetRangeMax());
    if (m_CurrentRGDirectionIsUpwards)
    {
        filter->SetLowerThreshold(tempLevelWindow.GetLevel()+0.5);
    }
    else
    {
        filter->SetLowerThreshold(tempLevelWindow.GetLevel()+0.5);
    }

    filter->Update();
    */

    // muellerm, 6.8. change:
    // instead of using the not working threshold filter, implemented a manual creation
    // of the binary image in an easy way: every pixel unequal zero in the preview (input)
    // image is a one in the resulting segmentation. at the same time overwrite the preview
    // with zeros to invalidate it
    // Allocate empty image
    typedef itk::Image<TPixel, VImageDimension> InputImageType;
    typedef itk::Image<unsigned char, VImageDimension> SegmentationType;
    typename SegmentationType::Pointer segmentationImage = SegmentationType::New();
    segmentationImage->SetRegions( itkImage->GetLargestPossibleRegion() );
    segmentationImage->SetOrigin( itkImage->GetOrigin() );
    segmentationImage->SetDirection( itkImage->GetDirection() );
    segmentationImage->SetSpacing( itkImage->GetSpacing() );
    segmentationImage->Allocate();

    itk::ImageRegionIterator<SegmentationType> itOutput( segmentationImage, segmentationImage->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<InputImageType> itInput( itkImage, itkImage->GetLargestPossibleRegion() );
    itOutput.GoToBegin();
    itInput.GoToBegin();
    while( !itOutput.IsAtEnd() && !itInput.IsAtEnd() )
    {
        if( itInput.Value() != 0 )
            itOutput.Set( 1 );
        else
            itOutput.Set( 0 );

        // overwrite preview, so it wont disturb as when we see our resulting segmentation
        //itInput.Set( 0 );
        ++itOutput;
        ++itInput;
    }

    mitk::Image::Pointer new_image = mitk::Image::New();
    mitk::CastToMitkImage( segmentationImage, new_image );

    mitk::DataNode::Pointer segNode = mitk::DataNode::New();
    segNode->SetData(new_image);
    segNode->SetName("RegionGrowing_Result");
    segNode->SetBoolProperty("binary", mitk::BoolProperty::New(true));

    //delete the old image, if there was one:
    mitk::DataNode::Pointer prevSegNode = m_DataStorage->GetNamedNode("RegionGrowing_Result");
    m_DataStorage->Remove(prevSegNode);

    m_DataStorage->Add(segNode, m_InputImageNode);
}

void QmitkAdaptiveRegionGrowingWidget::EnableControls(bool enable)
{
  //set the labels below the slider
  this->m_Controls.m_RSliderLabelLower->setText("Shrink");
  this->m_Controls.m_RGSliderLaberUpper->setText("Expand");

  this->m_Controls.m_RSliderLabelLower->setEnabled(enable);
  this->m_Controls.m_RGSliderLaberUpper->setEnabled(enable);

  this->m_Controls.m_pbDefineSeedPoint->setEnabled(enable);

  // Check if seed point is already set, if not leave RunSegmentation disabled
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode(m_NAMEFORSEEDPOINT);
  if (node.IsNull()) {
    this->m_Controls.m_pbRunSegmentation->setEnabled(false);
  }
  else
  {
    this->m_Controls.m_pbRunSegmentation->setEnabled(enable);
  }

  // Check if a segmentation exists, if not leave segmentation dependent disabled.
  node = m_DataStorage->GetNamedNode(m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if (node.IsNull()) {
      this->m_Controls.m_DecreaseTH->setEnabled(false);
      this->m_Controls.m_IncreaseTH->setEnabled(false);
      this->m_Controls.m_Slider->setEnabled(false);
      this->m_Controls.m_SliderValueLabel->setEnabled(false);
      this->m_Controls.m_pbConfirmSegementation->setEnabled(false);
  }
  else
  {
      this->m_Controls.m_DecreaseTH->setEnabled(enable);
      this->m_Controls.m_IncreaseTH->setEnabled(enable);
      this->m_Controls.m_Slider->setEnabled(enable);
      this->m_Controls.m_SliderValueLabel->setEnabled(enable);
      this->m_Controls.m_pbConfirmSegementation->setEnabled(enable);
  }

  this->m_Controls.m_cbVolumeRendering->setEnabled(enable);
}

void QmitkAdaptiveRegionGrowingWidget::EnableVolumeRendering(bool enable)
{
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);

  if(node.IsNull())
    return;

  if(m_MultiWidget)
     m_MultiWidget->SetWidgetPlanesVisibility(!enable);


  if (enable)
  {
    node->SetBoolProperty("volumerendering", enable);
    node->SetBoolProperty("volumerendering.uselod", true);
  }
  else
  {
    node->SetBoolProperty("volumerendering", enable);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkAdaptiveRegionGrowingWidget::UpdateVolumeRenderingThreshold(int thValue)
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

void QmitkAdaptiveRegionGrowingWidget::UseVolumeRendering(bool on)
{
  m_UseVolumeRendering = on;

  this->EnableVolumeRendering(on);
}

void QmitkAdaptiveRegionGrowingWidget::OnDefineThresholdBoundaries(bool status)
{
    m_Controls.m_LowerThresholdSpinBox->setEnabled(status);
    m_Controls.m_UpperThresholdSpinBox->setEnabled(status);
    m_Controls.lb_LowerTh->setEnabled(status);
    m_Controls.lb_UpperTh->setEnabled(status);
}

void QmitkAdaptiveRegionGrowingWidget::SetLowerThresholdValue( double lowerThreshold )
{
  m_LOWERTHRESHOLD = lowerThreshold;
}

void QmitkAdaptiveRegionGrowingWidget::SetUpperThresholdValue( double upperThreshold)
{
  m_UPPERTHRESHOLD = upperThreshold;
}

void QmitkAdaptiveRegionGrowingWidget::Deactivated()
{
  this->DeactivateSeedPointMode();

  // make the segmentation preview node invisible
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if( node.IsNotNull() )
  {
    node->SetVisibility(false);
  }

  // disable volume rendering preview after the segmentation node was created
  this->EnableVolumeRendering(false);
  m_Controls.m_cbVolumeRendering->setChecked(false);
}

void QmitkAdaptiveRegionGrowingWidget::Activated()
{

}

void QmitkAdaptiveRegionGrowingWidget::ActivateSeedPointMode()
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

void QmitkAdaptiveRegionGrowingWidget::DeactivateSeedPointMode()
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
