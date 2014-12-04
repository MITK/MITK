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

#include "mitkImageStatisticsHolder.h"

#include <itkConnectedAdaptiveThresholdImageFilter.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkImageIterator.h>

#include "itkOrImageFilter.h"
#include "mitkImageCast.h"
#include "QmitkConfirmSegmentationDialog.h"

#include "mitkPixelTypeMultiplex.h"
#include "mitkImagePixelReadAccessor.h"


MITK_TOOL_GUI_MACRO( , QmitkAdaptiveRegionGrowingToolGUI, "")

QmitkAdaptiveRegionGrowingToolGUI::QmitkAdaptiveRegionGrowingToolGUI(QWidget* parent) :
QmitkToolGUI(), m_MultiWidget(NULL), m_UseVolumeRendering(false), m_UpdateSuggestedThreshold(true), m_SuggestedThValue(0.0), m_DataStorage(NULL)
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
  this->SetDataNodeNames("labeledRGSegmentation","RGResult","RGFeedbackSurface");

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

void QmitkAdaptiveRegionGrowingToolGUI::SetDataNodeNames(std::string labledSegmentation, std::string binaryImage, std::string surface)
{
  m_NAMEFORLABLEDSEGMENTATIONIMAGE = labledSegmentation;
  m_NAMEFORBINARYIMAGE = binaryImage;
  m_NAMEFORSURFACE = surface;
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
static void AccessPixel(mitk::PixelType ptype, const mitk::Image::Pointer im, mitk::Point3D p, int & val)
{
  mitk::ImagePixelReadAccessor<TPixel,3> access(im);
  val = access.GetPixelByWorldCoordinates(p);
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
          QMessageBox::critical(NULL, "QmitkAdaptiveRegionGrowingToolGUI", "PointSetNode does not contain a pointset");
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
                int component = 0;
                m_InputImageNode->GetIntProperty("Image.Displayed Component", component);
              pixelValues[pos] = image->GetPixelValueByIndex(currentIndex, 0, component);
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
    QMessageBox::information( NULL, "Adaptive Region Growing functionality", "Please specify the image in Datamanager!");
    return;
  }

  mitk::DataNode::Pointer node = m_RegionGrow3DTool->GetPointSetNode();

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

  int timeStep = mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1") )->GetTimeStep();

  if (!(seedPointSet->GetSize(timeStep)))
  {
    m_Controls.m_pbRunSegmentation->setEnabled(true);
    QMessageBox::information( NULL, "Adaptive Region Growing functionality", "The seed point is empty! Please choose a new seed point.");
    return;
  }

  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );

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
          //QApplication::setOverrideCursor(QCursor(Qt::WaitCursor)); //set the cursor to waiting
          AccessByItk_2(orgImage, StartRegionGrowing, orgImage->GetGeometry(), seedPoint);
          //QApplication::restoreOverrideCursor();//reset cursor
      }
      else
      {
          QApplication::restoreOverrideCursor();//reset cursor
          QMessageBox::information( NULL, "Adaptive Region Growing functionality", "Only images of dimension 3 or 4 can be processed!");
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

  mitk::Image::Pointer resultImage = mitk::ImportItkImage(regionGrower->GetOutput())->Clone();
  //initialize slider
  m_Controls.m_PreviewSlider->setMinimum(m_LOWERTHRESHOLD);
  mitk::ScalarType max = m_LOWERTHRESHOLD+resultImage->GetStatistics()->GetScalarValueMax();
  if (max < m_UPPERTHRESHOLD)
    m_Controls.m_PreviewSlider->setMaximum(max);
  else
    m_Controls.m_PreviewSlider->setMaximum(m_UPPERTHRESHOLD);

  this->m_DetectedLeakagePoint = regionGrower->GetLeakagePoint();

  if(m_CurrentRGDirectionIsUpwards)
  {
    m_Controls.m_PreviewSlider->setValue(m_SeedPointValueMean-1);
  }
  else
  {
    m_Controls.m_PreviewSlider->setValue(m_SeedPointValueMean+1);
  }
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
    this->m_Controls.m_PreviewSlider->setValue(m_SeedpointValue);
    *level = m_UPPERTHRESHOLD - (m_SeedpointValue) + 0.5;
  }
  else
  {
    this->m_Controls.m_PreviewSlider->setValue(m_SeedpointValue);
    *level = (m_SeedpointValue) - m_LOWERTHRESHOLD + 0.5;
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


  mitk::Image* img = dynamic_cast<mitk::Image*>(newNode->GetData());
  AccessByItk(img, ITKThresholding);

  // disable volume rendering preview after the segmentation node was created
  this->EnableVolumeRendering(false);
  newNode->SetVisibility(false);
  m_Controls.m_cbVolumeRendering->setChecked(false);
  //TODO disable slider etc...
}

template<typename TPixel, unsigned int VImageDimension>
void QmitkAdaptiveRegionGrowingToolGUI::ITKThresholding(itk::Image<TPixel, VImageDimension>* itkImage)
{
  mitk::Image::Pointer originalSegmentation = dynamic_cast<mitk::Image*>(this->m_RegionGrow3DTool->
                                                                         GetTargetSegmentationNode()->GetData());

  int timeStep = mitk::BaseRenderer::GetInstance( mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1") )->GetTimeStep();

  if (originalSegmentation)
  {
    typedef itk::Image<TPixel, VImageDimension> InputImageType;
    typedef itk::Image<unsigned char, VImageDimension> SegmentationType;


    //select single 3D volume if we have more than one time step
    typename SegmentationType::Pointer originalSegmentationInITK = SegmentationType::New();
    if(originalSegmentation->GetTimeGeometry()->CountTimeSteps() > 1)
    {
      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput( originalSegmentation );
      timeSelector->SetTimeNr( timeStep );
      timeSelector->UpdateLargestPossibleRegion();
      CastToItkImage( timeSelector->GetOutput(), originalSegmentationInITK );
    }
    else //use original
    {
      CastToItkImage( originalSegmentation, originalSegmentationInITK );
    }

    //Fill current preiview image in segmentation image
    originalSegmentationInITK->FillBuffer(0);
    itk::ImageRegionIterator<SegmentationType> itOutput( originalSegmentationInITK, originalSegmentationInITK->GetLargestPossibleRegion() );
    itk::ImageRegionIterator<InputImageType> itInput( itkImage, itkImage->GetLargestPossibleRegion() );
    itOutput.GoToBegin();
    itInput.GoToBegin();

    //calculate threhold from slider value
    int currentTreshold = 0;
    if (m_CurrentRGDirectionIsUpwards)
    {
      currentTreshold = m_UPPERTHRESHOLD - m_Controls.m_PreviewSlider->value() + 1;
    }
    else
    {
      currentTreshold = m_Controls.m_PreviewSlider->value() - m_LOWERTHRESHOLD;
    }

    //iterate over image and set pixel in segmentation according to thresholded labeled image
    while( !itOutput.IsAtEnd() && !itInput.IsAtEnd() )
    {
      //Use threshold slider to determine if pixel is set to 1
      if( itInput.Value() != 0 && itInput.Value() > currentTreshold )
      {
        itOutput.Set( 1 );
      }

      ++itOutput;
      ++itInput;
    }


    //combine current working segmentation image with our region growing result
    originalSegmentation->SetVolume( (void*)(originalSegmentationInITK->GetPixelContainer()->GetBufferPointer()), timeStep);

    originalSegmentation->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
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

void QmitkAdaptiveRegionGrowingToolGUI::UpdateVolumeRenderingThreshold(int thValue)
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
