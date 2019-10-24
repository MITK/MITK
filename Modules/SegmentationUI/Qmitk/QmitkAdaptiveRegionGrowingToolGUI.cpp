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
#include "QmitkAdaptiveRegionGrowingToolGUI.h"

#include "QmitkStdMultiWidget.h"

#include <qmessagebox.h>

#include "mitkNodePredicateDataType.h"
#include "mitkProperties.h"
#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"
#include "mitkTransferFunctionProperty.h"
#include "mitkImageTimeSelector.h"
#include <mitkImageVtkAccessor.h>

#include "mitkImageStatisticsHolder.h"

#include <itkConnectedAdaptiveThresholdImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include "itkNumericTraits.h"
#include <itkImageIterator.h>
#include "itkMaskImageFilter.h"

#include "itkOrImageFilter.h"
#include "mitkImageCast.h"
#include "mitkImageCaster.h"
#include "QmitkConfirmSegmentationDialog.h"

#include "mitkPixelTypeMultiplex.h"

#include "mitkImageCast.h"


MITK_TOOL_GUI_MACRO( , QmitkAdaptiveRegionGrowingToolGUI, "")

QmitkAdaptiveRegionGrowingToolGUI::QmitkAdaptiveRegionGrowingToolGUI(QWidget* parent)
  : QmitkToolGUI(),
    m_MultiWidget(nullptr),
    m_DataStorage(nullptr),
    m_UseVolumeRendering(false),
    m_UpdateSuggestedThreshold(true),
    m_SuggestedThValue(0.0)
{
  this->setParent(parent);

  m_Controls.setupUi(this);

  m_Controls.m_ThresholdSlider->setDecimals(1);
  m_Controls.m_ThresholdSlider->setSpinBoxAlignment(Qt::AlignVCenter);

  m_Controls.m_PreviewSlider->setEnabled(false);
  m_Controls.m_PreviewSlider->setSingleStep(0.5);
  //Not yet available
  //m_Controls.m_PreviewSlider->InvertedAppearance(true);

  this->CreateConnections();
  this->SetDataNodeNames("labeledRGSegmentation","RGResult","RGFeedbackSurface", "maskedSegmentation" );

  connect( this, SIGNAL(NewToolAssociated(mitk::Tool*)), this, SLOT(OnNewToolAssociated(mitk::Tool*)) );
}



QmitkAdaptiveRegionGrowingToolGUI::~QmitkAdaptiveRegionGrowingToolGUI()
{
    //Removing the observer of the PointSet node
    if (m_RegionGrow3DTool->GetPointSetNode().IsNotNull())
    {
        m_RegionGrow3DTool->GetPointSetNode()->GetData()->RemoveObserver(m_PointSetAddObserverTag);
        m_RegionGrow3DTool->GetPointSetNode()->GetData()->RemoveObserver(m_PointSetMoveObserverTag);
    }
    this->RemoveHelperNodes();
}


void QmitkAdaptiveRegionGrowingToolGUI::OnNewToolAssociated(mitk::Tool* tool)
{
  m_RegionGrow3DTool = dynamic_cast<mitk::AdaptiveRegionGrowingTool*> (tool);
  if(m_RegionGrow3DTool.IsNotNull())
  {
    SetInputImageNode( this->m_RegionGrow3DTool->GetReferenceData() );
    this->m_DataStorage = this->m_RegionGrow3DTool->GetDataStorage();
    this->EnableControls(true);

    //Watch for point added or modified
    itk::SimpleMemberCommand<QmitkAdaptiveRegionGrowingToolGUI>::Pointer pointAddedCommand = itk::SimpleMemberCommand<QmitkAdaptiveRegionGrowingToolGUI>::New();
    pointAddedCommand->SetCallbackFunction(this, &QmitkAdaptiveRegionGrowingToolGUI::OnPointAdded);
    m_PointSetAddObserverTag = m_RegionGrow3DTool->GetPointSetNode()->GetData()->AddObserver( mitk::PointSetAddEvent(), pointAddedCommand);
    m_PointSetMoveObserverTag = m_RegionGrow3DTool->GetPointSetNode()->GetData()->AddObserver( mitk::PointSetMoveEvent(), pointAddedCommand);
  }
  else
  {
    this->EnableControls(false);
  }
}


void QmitkAdaptiveRegionGrowingToolGUI::RemoveHelperNodes()
{
  mitk::DataNode::Pointer imageNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if( imageNode.IsNotNull() )
  {
    m_DataStorage->Remove(imageNode);
  }

  mitk::DataNode::Pointer maskedSegmentationNode = m_DataStorage->GetNamedNode( m_NAMEFORMASKEDSEGMENTATION);
  if( maskedSegmentationNode.IsNotNull() )
  {
    m_DataStorage->Remove(maskedSegmentationNode);
  }
}

void QmitkAdaptiveRegionGrowingToolGUI::CreateConnections()
{
    //Connecting GUI components
    connect( (QObject*) (m_Controls.m_pbRunSegmentation), SIGNAL(clicked()), this, SLOT(RunSegmentation()));
    connect( m_Controls.m_PreviewSlider, SIGNAL(valueChanged(double)), this, SLOT(ChangeLevelWindow(double)));
    connect( (QObject*) (m_Controls.m_pbConfirmSegementation), SIGNAL(clicked()), this, SLOT(ConfirmSegmentation()));
    connect( (QObject*) (m_Controls.m_cbVolumeRendering), SIGNAL(toggled(bool)), this, SLOT(UseVolumeRendering(bool) ));
    connect( m_Controls.m_ThresholdSlider, SIGNAL(maximumValueChanged(double)), this, SLOT(SetUpperThresholdValue(double)));
    connect( m_Controls.m_ThresholdSlider, SIGNAL(minimumValueChanged(double)), this, SLOT(SetLowerThresholdValue(double)));
}

void QmitkAdaptiveRegionGrowingToolGUI::SetDataNodeNames(std::string labledSegmentation, std::string binaryImage, std::string surface, std::string maskedSegmentation )
{
  m_NAMEFORLABLEDSEGMENTATIONIMAGE = labledSegmentation;
  m_NAMEFORBINARYIMAGE = binaryImage;
  m_NAMEFORSURFACE = surface;
  m_NAMEFORMASKEDSEGMENTATION = maskedSegmentation;
}

void QmitkAdaptiveRegionGrowingToolGUI::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkAdaptiveRegionGrowingToolGUI::SetMultiWidget(QmitkStdMultiWidget* multiWidget)
{
  m_MultiWidget = multiWidget;
}

void QmitkAdaptiveRegionGrowingToolGUI::SetInputImageNode(mitk::DataNode* node)
{
  m_InputImageNode = node;
  mitk::Image* inputImage = dynamic_cast<mitk::Image*>(m_InputImageNode->GetData());
  if (inputImage)
  {
    mitk::ScalarType max = inputImage->GetStatistics()->GetScalarValueMax();
    mitk::ScalarType min = inputImage->GetStatistics()->GetScalarValueMin();
    m_Controls.m_ThresholdSlider->setMaximum(max);
    m_Controls.m_ThresholdSlider->setMinimum(min);
    // Just for initialization
    m_Controls.m_ThresholdSlider->setMaximumValue(max);
    m_Controls.m_ThresholdSlider->setMinimumValue(min);
  }
}

template <typename TPixel>
static void AccessPixel(mitk::PixelType /*ptype*/, const mitk::Image::Pointer im, mitk::Point3D p, int & val)
{
  itk::Index<3> itkIndex;
  im->GetGeometry()->WorldToIndex(p, itkIndex);
  val = mitk::UnlockedSinglePixelAccess(im, itkIndex, 0, 0);
}

void QmitkAdaptiveRegionGrowingToolGUI::OnPointAdded()
{
  if (m_RegionGrow3DTool.IsNull())
    return;

  mitk::DataNode* node = m_RegionGrow3DTool->GetPointSetNode();

  if (node != NULL)
  {
      mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(node->GetData());

      if (pointSet.IsNull())
      {
          QMessageBox::critical(NULL, "QmitkAdaptiveRegionGrowingToolGUI", tr("PointSetNode does not contain a pointset"));
          return;
      }

      m_Controls.m_lblSetSeedpoint->setText("");

      mitk::Image* image = dynamic_cast<mitk::Image*>(m_InputImageNode->GetData());


      mitk::Point3D seedPoint = pointSet->GetPointSet(mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1") )->GetTimeStep())->GetPoints()->ElementAt(0);


      if (image->GetGeometry()->IsInside(seedPoint))
        mitkPixelTypeMultiplex3(AccessPixel,image->GetChannelDescriptor().GetPixelType(),image,seedPoint,m_SeedpointValue)
      else
        return;

      /* In this case the seedpoint is placed e.g. in the lung or bronchialtree
       * The lowerFactor sets the windowsize depending on the regiongrowing direction
       */
      m_CurrentRGDirectionIsUpwards = true;
      if (m_SeedpointValue < -500)
      {
          m_CurrentRGDirectionIsUpwards = false;
      }
      // Initializing the region by the area around the seedpoint
      m_SeedPointValueMean = 0;

      itk::Index<3> currentIndex, runningIndex;
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
              pixelValues[pos] = mitk::UnlockedSinglePixelAccess(image, currentIndex, 0);

              pos++;
            }
            else
            {
              pixelValues[pos] = std::numeric_limits< long >::min();
              pos++;
            }
          }
        }
      }

      //Now calculation mean of the pixelValues
      //Now calculation mean of the pixelValues
      unsigned int numberOfValues(0);
      for (auto & pixelValue : pixelValues)
      {
        if(pixelValue > std::numeric_limits< long >::min())
        {
          m_SeedPointValueMean += pixelValue;
          numberOfValues++;
        }
      }
      m_SeedPointValueMean = m_SeedPointValueMean/numberOfValues;

      mitk::ScalarType var = 0;
      if ( numberOfValues > 1 )
      {
        for (auto & pixelValue : pixelValues)
        {
          if(pixelValue > std::numeric_limits< mitk::ScalarType >::min())
          {
            var += ( pixelValue - m_SeedPointValueMean )*( pixelValue - m_SeedPointValueMean );
          }
        }
        var /= numberOfValues - 1;
      }
      mitk::ScalarType stdDev = sqrt( var );

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
        m_LOWERTHRESHOLD = m_SeedPointValueMean - stdDev;
        m_UPPERTHRESHOLD = m_SeedpointValue + windowSize;
        if (m_UPPERTHRESHOLD > max)
          m_UPPERTHRESHOLD = max;
        m_Controls.m_ThresholdSlider->setMaximumValue(m_UPPERTHRESHOLD);
        m_Controls.m_ThresholdSlider->setMinimumValue(m_LOWERTHRESHOLD);
      }
      else
      {
        m_UPPERTHRESHOLD = m_SeedPointValueMean;
        if (m_SeedpointValue > m_SeedPointValueMean)
          m_UPPERTHRESHOLD = m_SeedpointValue;
        m_LOWERTHRESHOLD = m_SeedpointValue - windowSize;
        if (m_LOWERTHRESHOLD < min)
          m_LOWERTHRESHOLD = min;
        m_Controls.m_ThresholdSlider->setMinimumValue(m_LOWERTHRESHOLD);
        m_Controls.m_ThresholdSlider->setMaximumValue(m_UPPERTHRESHOLD);
      }
  }
}



void QmitkAdaptiveRegionGrowingToolGUI::RunSegmentation()
{
  if (m_InputImageNode.IsNull())
  {
    QMessageBox::information( NULL, tr("Adaptive Region Growing functionality"), tr("Please specify the image in Datamanager!"));
    return;
  }

  mitk::DataNode::Pointer node = m_RegionGrow3DTool->GetPointSetNode();

  if (node.IsNull())
  {
    QMessageBox::information( NULL, tr("Adaptive Region Growing functionality"),
      tr("Please insert a seed point inside the image.\n\nFirst press the \"Define Seed Point\" button,\nthen click "
      "left mouse button inside the image."));
    return;
  }

  //safety if no pointSet or pointSet empty
  mitk::PointSet::Pointer seedPointSet = dynamic_cast<mitk::PointSet*> (node->GetData());
  if (seedPointSet.IsNull())
  {
    m_Controls.m_pbRunSegmentation->setEnabled(true);
    QMessageBox::information( NULL, tr("Adaptive Region Growing functionality"),
      tr("The seed point is empty! Please choose a new seed point."));
    return;
  }

  int timeStep = mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1") )->GetTimeStep();

  if (!(seedPointSet->GetSize(timeStep)))
  {
    m_Controls.m_pbRunSegmentation->setEnabled(true);
    QMessageBox::information( NULL, tr("Adaptive Region Growing functionality"),
      tr("The seed point is empty! Please choose a new seed point."));
    return;
  }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

  mitk::PointSet::PointType seedPoint = seedPointSet->GetPointSet(timeStep)->GetPoints()->Begin().Value();

  mitk::Image::Pointer orgImage = dynamic_cast<mitk::Image*> (m_InputImageNode->GetData());

  int displayedComponent = 0;
  m_InputImageNode->GetIntProperty("Image.Displayed Component", displayedComponent);

  if (orgImage.IsNotNull()) {
      if (orgImage->GetDimension() == 4) {
          mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
          timeSelector->SetInput(orgImage);
          timeSelector->SetTimeNr( timeStep );
          timeSelector->UpdateLargestPossibleRegion();
          mitk::Image* timedImage = timeSelector->GetOutput();
          if (timedImage->GetPixelType().GetNumberOfComponents() > 1) {
            mitk::Image::Pointer temp = mitk::Image::New();
            AccessVectorPixelTypeByItk_n(timedImage, mitk::extractComponentFromVectorByItk, (temp, displayedComponent));
            timedImage = temp;
          }
          AccessByItk_2(timedImage, StartRegionGrowing, timedImage->GetGeometry(), seedPoint);
      } else if (orgImage->GetDimension() == 3) {
          if (orgImage->GetPixelType().GetNumberOfComponents() > 1) {
            mitk::Image::Pointer temp = mitk::Image::New();
            AccessVectorPixelTypeByItk_n(orgImage, mitk::extractComponentFromVectorByItk, (temp, displayedComponent));
            orgImage = temp;
          }
          //QApplication::setOverrideCursor(QCursor(Qt::WaitCursor)); //set the cursor to waiting
          AccessByItk_2(orgImage, StartRegionGrowing, orgImage->GetGeometry(), seedPoint);
          //QApplication::restoreOverrideCursor();//reset cursor
      } else {
          QApplication::restoreOverrideCursor();//reset cursor
          QMessageBox::information( NULL, tr("Adaptive Region Growing functionality"),
            tr("Only images of dimension 3 or 4 can be processed!"));
          return;
      }
  }
  EnableControls(true); // Segmentation ran successfully, so enable all controls.
  node->SetVisibility(true);
  QApplication::restoreOverrideCursor();//reset cursor

}

template<typename TPixel, unsigned int VImageDimension>
void QmitkAdaptiveRegionGrowingToolGUI::StartRegionGrowing(itk::Image<TPixel, VImageDimension>* itkImage, mitk::BaseGeometry* imageGeometry, mitk::PointSet::PointType seedPoint)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef typename InputImageType::IndexType IndexType;
  typedef itk::ConnectedAdaptiveThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();
  typedef itk::BinaryThresholdImageFilter<InputImageType, InputImageType> ThresholdFilterType;
  typedef itk::MaskImageFilter<InputImageType, InputImageType, InputImageType> MaskImageFilterType;

  if ( !imageGeometry->IsInside(seedPoint) )
  {
    QApplication::restoreOverrideCursor();//reset cursor to be able to click ok with the regular mouse cursor
    QMessageBox::information( NULL, tr("Segmentation functionality"),
      tr("The seed point is outside of the image! Please choose a position inside the image!"));
    return;
  }

  IndexType seedIndex;
  imageGeometry->WorldToIndex( seedPoint, seedIndex);// convert world coordinates to image indices

  if (m_SeedpointValue>m_UPPERTHRESHOLD || m_SeedpointValue<m_LOWERTHRESHOLD)
  {
    QApplication::restoreOverrideCursor();//reset cursor to be able to click ok with the regular mouse cursor
    QMessageBox::information( NULL, tr("Segmentation functionality"),
      tr("The seed point is outside the defined thresholds! Please set a new seed point or adjust the thresholds."));
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
    errorInfo.setWindowTitle(tr("Adaptive RG Segmentation Functionality"));
    errorInfo.setIcon(QMessageBox::Critical);
    errorInfo.setText(tr("An error occurred during region growing!"));
    errorInfo.setDetailedText(exc.what());
    errorInfo.exec();
    return; // can't work
  }
  catch( ... )
  {
    QMessageBox::critical( NULL, tr("Adaptive RG Segmentation Functionality"), tr("An error occurred during region growing!"));
    return;
  }

  mitk::Image::Pointer resultImage = mitk::ImportItkImage(regionGrower->GetOutput())->Clone();

  m_RGMINIMUM = resultImage->GetStatistics()->GetScalarValueMin() != 0 ? resultImage->GetStatistics()->GetScalarValueMin() : 1;
  m_RGMAXIMUM = resultImage->GetStatistics()->GetScalarValueMax();

  //initialize slider
  m_Controls.m_PreviewSlider->setMinimum(m_RGMINIMUM);
  m_Controls.m_PreviewSlider->setMaximum(m_RGMAXIMUM);
  m_Controls.m_PreviewSlider->setValue((m_RGMAXIMUM - m_RGMINIMUM) / 2.0);

  this->m_DetectedLeakagePoint = regionGrower->GetLeakagePoint();

  this->m_SliderInitialized = true;

  //create new node and then delete the old one if there is one
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData( resultImage );

  // set some properties
  newNode->SetProperty("name", mitk::StringProperty::New(m_NAMEFORLABLEDSEGMENTATIONIMAGE));
  newNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  newNode->SetProperty("color", mitk::ColorProperty::New(0.0,1.0,0.0));
  newNode->SetProperty("layer", mitk::IntProperty::New(1));
  newNode->SetProperty("opacity", mitk::FloatProperty::New(0.7));

  //delete the old image, if there was one:
  mitk::DataNode::Pointer binaryNode = m_DataStorage->GetNamedNode(m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  m_DataStorage->Remove(binaryNode);

  // now add result to data tree
  m_DataStorage->Add( newNode, m_InputImageNode );

  typename InputImageType::Pointer inputImageItk;
  mitk::CastToItkImage<InputImageType>( resultImage, inputImageItk );
  //volume rendering preview masking
  typename ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  thresholdFilter->SetInput( inputImageItk );
  thresholdFilter->SetInsideValue( 1 );
  thresholdFilter->SetOutsideValue( 0 );

  double sliderVal = this->m_Controls.m_PreviewSlider->value();
  if (m_CurrentRGDirectionIsUpwards)
  {
    thresholdFilter->SetLowerThreshold( sliderVal );
    thresholdFilter->SetUpperThreshold( itk::NumericTraits< TPixel >::max() );
  }
  else
  {
    thresholdFilter->SetLowerThreshold( itk::NumericTraits< TPixel >::min() );
    thresholdFilter->SetUpperThreshold( sliderVal );
  }
  thresholdFilter->SetInPlace( false );

  typename MaskImageFilterType::Pointer maskFilter = MaskImageFilterType::New();
  maskFilter->SetInput( inputImageItk );
  maskFilter->SetInPlace( false );
  maskFilter->SetMaskImage( thresholdFilter->GetOutput() );
  maskFilter->SetOutsideValue( 0 );
  maskFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer mitkMask;
  mitk::CastToMitkImage<InputImageType>( maskFilter->GetOutput(), mitkMask );
  mitk::DataNode::Pointer maskedNode = mitk::DataNode::New();
  maskedNode->SetData( mitkMask );

  // set some properties
  maskedNode->SetProperty("name", mitk::StringProperty::New(m_NAMEFORMASKEDSEGMENTATION));
  maskedNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  maskedNode->SetProperty("color", mitk::ColorProperty::New(0.0,1.0,0.0));
  maskedNode->SetProperty("layer", mitk::IntProperty::New(1));
  maskedNode->SetProperty("opacity", mitk::FloatProperty::New(0.0));

  //delete the old image, if there was one:
  mitk::DataNode::Pointer deprecatedMask = m_DataStorage->GetNamedNode(m_NAMEFORMASKEDSEGMENTATION);
  m_DataStorage->Remove(deprecatedMask);

  // now add result to data tree
  m_DataStorage->Add( maskedNode, m_InputImageNode );

  this->InitializeLevelWindow();

  if(m_UseVolumeRendering)
    this->EnableVolumeRendering(true);

  m_UpdateSuggestedThreshold = true;// reset first stored threshold value
  //Setting progress to finished
  mitk::ProgressBar::GetInstance()->Progress(357);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkAdaptiveRegionGrowingToolGUI::InitializeLevelWindow()
{
  //get the preview from the datatree
  mitk::DataNode::Pointer newNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);

  mitk::LevelWindow tempLevelWindow;
  newNode->GetLevelWindow(tempLevelWindow, NULL, "levelwindow");

  mitk::ScalarType* level = new mitk::ScalarType(0.0);
  mitk::ScalarType* window = new mitk::ScalarType(1.0);

  tempLevelWindow.SetRangeMinMax(mitk::ScalarType(m_RGMINIMUM), mitk::ScalarType(m_RGMAXIMUM));
  *level = (m_RGMAXIMUM - m_RGMINIMUM) / 2.0;

  tempLevelWindow.SetLevelWindow(*level, *window);
  newNode->SetLevelWindow(tempLevelWindow, NULL, "levelwindow");
  //update the widgets
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  m_SliderInitialized = true;

  //inquiry need to fix bug#1828
  static int lastSliderPosition = 0;
  if ((this->m_SeedpointValue + this->m_DetectedLeakagePoint - 1) == lastSliderPosition)
  {
    //this->ChangeLevelWindow(lastSliderPosition);
  }
  lastSliderPosition = this->m_SeedpointValue + this->m_DetectedLeakagePoint-1;

  if(m_MultiWidget)
  {
    this->m_MultiWidget->levelWindowWidget->GetManager()->SetAutoTopMostImage(false);
    this->m_MultiWidget->levelWindowWidget->GetManager()->SetLevelWindowProperty(static_cast<mitk::LevelWindowProperty*>(newNode->GetProperty("levelwindow")));
  }

  if (m_UseVolumeRendering)
  this->UpdateVolumeRenderingThreshold((int) (*level + 0.5));//lower threshold for labeled image
}

void QmitkAdaptiveRegionGrowingToolGUI::ChangeLevelWindow(double newValue)
{
  if (m_SliderInitialized)
  {
    //do nothing, if no preview exists
    mitk::DataNode::Pointer newNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
    if (newNode.IsNull())
      return;

    mitk::LevelWindow tempLevelWindow;

    newNode->GetLevelWindow(tempLevelWindow, NULL, "levelwindow"); //get the levelWindow associated with the preview

    mitk::ScalarType level;
    mitk::ScalarType* window = new mitk::ScalarType(1);

    //adjust the levelwindow according to the position of the slider (newvalue)
    if (m_CurrentRGDirectionIsUpwards)
    {
      level = m_RGMAXIMUM - newValue + 0.5;
      tempLevelWindow.SetLevelWindow(level, *window);
    }
    else
    {
      level = newValue - m_RGMINIMUM + 0.5;
      tempLevelWindow.SetLevelWindow(level, *window);
    }

    newNode->SetLevelWindow(tempLevelWindow, NULL, "levelwindow");

    if (m_UseVolumeRendering)
    this->UpdateVolumeRenderingThreshold((int) (level - 0.5));//lower threshold for labeled image

    newNode->SetVisibility(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkAdaptiveRegionGrowingToolGUI::DecreaseSlider()
{
  //moves the slider one step to the left, when the "-"-button is pressed
  if (this->m_Controls.m_PreviewSlider->value() != this->m_Controls.m_PreviewSlider->minimum())
  {
    int newValue = this->m_Controls.m_PreviewSlider->value() - 1;
    this->ChangeLevelWindow(newValue);
    this->m_Controls.m_PreviewSlider->setValue(newValue);
  }
}

void QmitkAdaptiveRegionGrowingToolGUI::IncreaseSlider()
{
  //moves the slider one step to the right, when the "+"-button is pressed
  if (this->m_Controls.m_PreviewSlider->value() != this->m_Controls.m_PreviewSlider->maximum())
  {
    int newValue = this->m_Controls.m_PreviewSlider->value() + 1;
    this->ChangeLevelWindow(newValue);
    this->m_Controls.m_PreviewSlider->setValue(newValue);
  }
}

void QmitkAdaptiveRegionGrowingToolGUI::ConfirmSegmentation()
{
  //get image node
  if(m_InputImageNode.IsNull())
  {
    QMessageBox::critical( NULL, tr("Adaptive region growing functionality"), tr("Please specify the image in Datamanager!"));
    return;
  }
  //get image data
  mitk::Image::Pointer orgImage = dynamic_cast<mitk::Image*> (m_InputImageNode->GetData());
  if(orgImage.IsNull())
  {
    QMessageBox::critical( NULL, tr("Adaptive region growing functionality"), tr("No Image found!"));
    return;
  }
  //get labeled segmentation
  mitk::Image::Pointer labeledSeg = (mitk::Image*)m_DataStorage->GetNamedObject<mitk::Image>(m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if(labeledSeg.IsNull())
  {
    QMessageBox::critical( NULL, tr("Adaptive region growing functionality"), tr("No Segmentation Preview found!"));
    return;
  }

  mitk::DataNode::Pointer newNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if (newNode.IsNull())
    return;

  QmitkConfirmSegmentationDialog dialog;
  QString segName = QString::fromStdString(m_RegionGrow3DTool->GetCurrentSegmentationName());

  dialog.SetSegmentationName(segName);
  int result = dialog.exec();

  switch(result)
  {
  case QmitkConfirmSegmentationDialog::CREATE_NEW_SEGMENTATION:
    m_RegionGrow3DTool->SetOverwriteExistingSegmentation(false);
    break;
  case QmitkConfirmSegmentationDialog::OVERWRITE_SEGMENTATION:
    m_RegionGrow3DTool->SetOverwriteExistingSegmentation(true);
    break;
  case QmitkConfirmSegmentationDialog::CANCEL_SEGMENTATION:
    return;
  }

  mitk::DataNode::Pointer targetSegmentation = m_RegionGrow3DTool->GetTargetSegmentationNode();
  mitk::Image::Pointer originalSegmentation = dynamic_cast<mitk::Image*>(targetSegmentation->GetData());
  mitk::Image::Pointer previewImage = dynamic_cast<mitk::Image*>(newNode->GetData());

  // Select single 3D volume if we have more than one time step
  if(originalSegmentation->GetTimeGeometry()->CountTimeSteps() > 1) {
    int timeStep = mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))->GetTimeStep();
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput( originalSegmentation );
    timeSelector->SetTimeNr( timeStep );
    timeSelector->UpdateLargestPossibleRegion();
    AccessByItk_1(timeSelector->GetOutput(), ITKThresholding, previewImage);
  } else { // Use original
    AccessByItk_1(originalSegmentation, ITKThresholding, previewImage);
  }

  originalSegmentation->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  // disable volume rendering preview after the segmentation node was created
  this->EnableVolumeRendering(false);
  newNode->SetVisibility(false);
  m_Controls.m_cbVolumeRendering->setChecked(false);
  //TODO disable slider etc...

  if (m_RegionGrow3DTool.IsNotNull())
  {
    m_RegionGrow3DTool->ConfirmSegmentation();
  }
}

template<typename TPixel, unsigned int VImageDimension>
void QmitkAdaptiveRegionGrowingToolGUI::ITKThresholding(itk::Image<TPixel, VImageDimension>* inputSegmentation, mitk::Image* computedSegmentation)
{
  if (computedSegmentation == nullptr) {
    return;
  }

  int displayedComponent = 0;
  m_InputImageNode->GetIntProperty("Image.Displayed Component", displayedComponent);

  typedef itk::Image<TPixel, VImageDimension> InputImageType;

  typename InputImageType::Pointer computedSegmentationInITK = InputImageType::New();
  mitk::CastToItkImage<InputImageType>(computedSegmentation, computedSegmentationInITK);

  //Fill current preiview image in segmentation image
  inputSegmentation->FillBuffer(0);
  itk::ImageRegionIterator<InputImageType> itOriginal(inputSegmentation, inputSegmentation->GetLargestPossibleRegion());
  itk::ImageRegionIterator<InputImageType> itComputed(computedSegmentationInITK, computedSegmentationInITK->GetLargestPossibleRegion());
  itOriginal.GoToBegin();
  itComputed.GoToBegin();

  //calculate threhold from slider value
  int currentTreshold = 0;
  if (m_CurrentRGDirectionIsUpwards)
  {
    currentTreshold = m_RGMAXIMUM - m_Controls.m_PreviewSlider->value() + 1;
  }
  else
  {
    currentTreshold = m_Controls.m_PreviewSlider->value() - m_RGMINIMUM;
  }

  //iterate over image and set pixel in segmentation according to thresholded labeled image
  while(!itOriginal.IsAtEnd() && !itComputed.IsAtEnd()) {
    //Use threshold slider to determine if pixel is set to 1
    if(itComputed.Value() != 0 && itComputed.Value() > currentTreshold) {
      itOriginal.Set(1);
    }

    ++itOriginal;
    ++itComputed;
  }
}

void QmitkAdaptiveRegionGrowingToolGUI::EnableControls(bool enable)
{
  if (m_RegionGrow3DTool.IsNull())
    return;

  // Check if seed point is already set, if not leave RunSegmentation disabled
  //if even m_DataStorage is NULL leave node NULL
  mitk::DataNode::Pointer node = m_RegionGrow3DTool->GetPointSetNode();
  if (node.IsNull()) {
    this->m_Controls.m_pbRunSegmentation->setEnabled(false);
  }
  else
  {
    this->m_Controls.m_pbRunSegmentation->setEnabled(enable);
  }

  // Check if a segmentation exists, if not leave segmentation dependent disabled.
  //if even m_DataStorage is NULL leave node NULL
  node = m_DataStorage?m_DataStorage->GetNamedNode(m_NAMEFORLABLEDSEGMENTATIONIMAGE):NULL;
  if (node.IsNull())
  {
      this->m_Controls.m_PreviewSlider->setEnabled(false);
      this->m_Controls.m_pbConfirmSegementation->setEnabled(false);
  }
  else
  {
      this->m_Controls.m_PreviewSlider->setEnabled(enable);
      this->m_Controls.m_pbConfirmSegementation->setEnabled(enable);
  }

  this->m_Controls.m_cbVolumeRendering->setEnabled(enable);
}

void QmitkAdaptiveRegionGrowingToolGUI::EnableVolumeRendering(bool enable)
{
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode( m_NAMEFORMASKEDSEGMENTATION );

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

  double val = this->m_Controls.m_PreviewSlider->value();
  this->ChangeLevelWindow( val );

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkAdaptiveRegionGrowingToolGUI::UpdateVolumeRenderingThreshold(int thValue)
{
  typedef short PixelType;
  typedef itk::Image< PixelType, 3 > InputImageType;
  typedef itk::BinaryThresholdImageFilter< InputImageType, InputImageType > ThresholdFilterType;
  typedef itk::MaskImageFilter< InputImageType, InputImageType, InputImageType > MaskImageFilterType;

  mitk::DataNode::Pointer grownImageNode = m_DataStorage->GetNamedNode( m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  mitk::Image::Pointer grownImage = dynamic_cast< mitk::Image* >( grownImageNode->GetData() );

  if ( !grownImage )
  {
    MITK_ERROR << "Missing data node for labeled segmentation image.";
    return;
  }

  InputImageType::Pointer itkGrownImage;
  mitk::CastToItkImage( grownImage, itkGrownImage );

  ThresholdFilterType::Pointer thresholdFilter = ThresholdFilterType::New();
  thresholdFilter->SetInput( itkGrownImage );
  thresholdFilter->SetInPlace( false );

  double sliderVal = this->m_Controls.m_PreviewSlider->value();
  PixelType threshold = itk::NumericTraits< PixelType >::min();
  if (m_CurrentRGDirectionIsUpwards)
  {
    threshold = static_cast< PixelType >( m_UPPERTHRESHOLD - sliderVal + 0.5 );

    thresholdFilter->SetLowerThreshold( threshold );
    thresholdFilter->SetUpperThreshold( itk::NumericTraits< PixelType >::max() );
  }
  else
  {
    threshold = sliderVal - m_LOWERTHRESHOLD + 0.5;

    thresholdFilter->SetLowerThreshold( itk::NumericTraits< PixelType >::min() );
    thresholdFilter->SetUpperThreshold( threshold );
  }
  thresholdFilter->UpdateLargestPossibleRegion();

  MaskImageFilterType::Pointer maskFilter = MaskImageFilterType::New();
  maskFilter->SetInput( itkGrownImage );
  maskFilter->SetInPlace( false );
  maskFilter->SetMaskImage( thresholdFilter->GetOutput() );
  maskFilter->SetOutsideValue( 0 );
  maskFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer mitkMaskedImage;
  mitk::CastToMitkImage< InputImageType >( maskFilter->GetOutput(), mitkMaskedImage );
  mitk::DataNode::Pointer maskNode = m_DataStorage->GetNamedNode(m_NAMEFORMASKEDSEGMENTATION);
  maskNode->SetData( mitkMaskedImage );
}

void QmitkAdaptiveRegionGrowingToolGUI::UseVolumeRendering(bool on)
{
  m_UseVolumeRendering = on;

  this->EnableVolumeRendering(on);
}

void QmitkAdaptiveRegionGrowingToolGUI::SetLowerThresholdValue( double lowerThreshold )
{
  m_LOWERTHRESHOLD = lowerThreshold;
}

void QmitkAdaptiveRegionGrowingToolGUI::SetUpperThresholdValue( double upperThreshold)
{
  m_UPPERTHRESHOLD = upperThreshold;
}

void QmitkAdaptiveRegionGrowingToolGUI::Deactivated()
{
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

void QmitkAdaptiveRegionGrowingToolGUI::Activated()
{
}
