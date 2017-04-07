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
#include "QmitkThresholdComponentsToolGUI.h"

#include "QmitkStdMultiWidget.h"

#include <qmessagebox.h>

#include "mitkITKImageImport.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageTimeSelector.h"
#include "mitkNodePredicateDataType.h"
#include "mitkProperties.h"
#include "mitkTransferFunctionProperty.h"

#include "mitkImageStatisticsHolder.h"

#include "itkMaskImageFilter.h"
#include "itkNumericTraits.h"
#include <itkBinaryThresholdImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkImageIterator.h>
#include <itkMinimumMaximumImageCalculator.h>

#include "QmitkConfirmSegmentationDialog.h"
#include "itkOrImageFilter.h"
#include "mitkImageCast.h"

#include "mitkImagePixelReadAccessor.h"
#include "mitkPixelTypeMultiplex.h"

#include "mitkImageCast.h"

MITK_TOOL_GUI_MACRO(, QmitkThresholdComponentsToolGUI, "")

QmitkThresholdComponentsToolGUI::QmitkThresholdComponentsToolGUI(QWidget *parent)
  : QmitkToolGUI(),
    m_MultiWidget(nullptr),
    m_DataStorage(nullptr),
    m_UseVolumeRendering(true),
    m_SeedInitialized(false)
{
  this->setParent(parent);

  m_Controls.setupUi(this);

	//ctkRangeWidget controls for this slider
  m_Controls.m_ThresholdSlider->setDecimals(1);
  m_Controls.m_ThresholdSlider->setSingleStep(10.0); //adjust step size of slider
  m_Controls.m_ThresholdSlider->setSpinBoxAlignment(Qt::AlignVCenter);
  m_Controls.m_ThresholdSlider->setTracking(false);

  this->CreateConnections();
  this->SetDataNodeNames("labeledSegmentation", "Result", "FeedbackSurface", "maskedSegmentation");

  connect(this, SIGNAL(NewToolAssociated(mitk::Tool *)), this, SLOT(OnNewToolAssociated(mitk::Tool *)));
}

QmitkThresholdComponentsToolGUI::~QmitkThresholdComponentsToolGUI()
{
  // Removing the observer of the PointSet node
  if (m_ThresholdComponentsTool->GetPointSetNode().IsNotNull())
  {
    m_ThresholdComponentsTool->GetPointSetNode()->GetData()->RemoveObserver(m_PointSetAddObserverTag);
    m_ThresholdComponentsTool->GetPointSetNode()->GetData()->RemoveObserver(m_PointSetMoveObserverTag);
  }
  this->RemoveHelperNodes();
}

void QmitkThresholdComponentsToolGUI::OnNewToolAssociated(mitk::Tool *tool)
{
  m_ThresholdComponentsTool = dynamic_cast<mitk::ThresholdComponentsTool *>(tool);
  if (m_ThresholdComponentsTool.IsNotNull())
  { this->m_ThresholdComponentsTool->SetOverwriteExistingSegmentation(true);
    SetInputImageNode(this->m_ThresholdComponentsTool->GetReferenceData());
    this->m_DataStorage = this->m_ThresholdComponentsTool->GetDataStorage();
    this->EnableControls(true);

    // Watch for point added or modified
    itk::SimpleMemberCommand<QmitkThresholdComponentsToolGUI>::Pointer pointAddedCommand =
      itk::SimpleMemberCommand<QmitkThresholdComponentsToolGUI>::New();
    pointAddedCommand->SetCallbackFunction(this, &QmitkThresholdComponentsToolGUI::OnPointAdded);
    m_PointSetAddObserverTag =
      m_ThresholdComponentsTool->GetPointSetNode()->GetData()->AddObserver(mitk::PointSetAddEvent(), pointAddedCommand);
    m_PointSetMoveObserverTag =
      m_ThresholdComponentsTool->GetPointSetNode()->GetData()->AddObserver(mitk::PointSetMoveEvent(), pointAddedCommand);
  }
  else
  {
    this->EnableControls(false);
  }
}

void QmitkThresholdComponentsToolGUI::RemoveHelperNodes()
{
  mitk::DataNode::Pointer imageNode = m_DataStorage->GetNamedNode(m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if (imageNode.IsNotNull())
  {
    m_DataStorage->Remove(imageNode);
  }

  mitk::DataNode::Pointer maskedSegmentationNode = m_DataStorage->GetNamedNode(m_NAMEFORMASKEDSEGMENTATION);
  if (maskedSegmentationNode.IsNotNull())
  {
    m_DataStorage->Remove(maskedSegmentationNode);
  }
}

void QmitkThresholdComponentsToolGUI::CreateConnections()
{
  // Connecting GUI components
  connect((QObject *)(m_Controls.m_pbConfirmSegementation), SIGNAL(clicked()), this, SLOT(ConfirmSegmentation()));
  connect((QObject *)(m_Controls.m_cbVolumeRendering), SIGNAL(toggled(bool)), this, SLOT(UseVolumeRendering(bool)));
  connect(
    m_Controls.m_ThresholdSlider, SIGNAL(maximumValueChanged(double)), this, SLOT(SetUpperThresholdValue(double)));
  connect(
    m_Controls.m_ThresholdSlider, SIGNAL(minimumValueChanged(double)), this, SLOT(SetLowerThresholdValue(double)));
}

void QmitkThresholdComponentsToolGUI::SetDataNodeNames(std::string labledSegmentation,
                                                         std::string binaryImage,
                                                         std::string surface,
                                                         std::string maskedSegmentation)
{
  m_NAMEFORLABLEDSEGMENTATIONIMAGE = labledSegmentation;
  m_NAMEFORBINARYIMAGE = binaryImage;
  m_NAMEFORSURFACE = surface;
  m_NAMEFORMASKEDSEGMENTATION = maskedSegmentation;
}

void QmitkThresholdComponentsToolGUI::SetDataStorage(mitk::DataStorage *dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkThresholdComponentsToolGUI::SetMultiWidget(QmitkStdMultiWidget *multiWidget)
{
  m_MultiWidget = multiWidget;
}

void QmitkThresholdComponentsToolGUI::SetInputImageNode(mitk::DataNode *node)
{
  m_InputImageNode = node;
  mitk::Image *inputImage = dynamic_cast<mitk::Image *>(m_InputImageNode->GetData());
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
static void AccessPixel(mitk::PixelType /*ptype*/, const mitk::Image::Pointer im, mitk::Point3D p, int &val)
{
  mitk::ImagePixelReadAccessor<TPixel, 3> access(im);
  val = access.GetPixelByWorldCoordinates(p);
}

void QmitkThresholdComponentsToolGUI::OnPointAdded()
{
  if (m_ThresholdComponentsTool.IsNull())
    return;

  mitk::DataNode *node = m_ThresholdComponentsTool->GetPointSetNode();

  if (node != NULL)
  {
    mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet *>(node->GetData());

    if (pointSet.IsNull())
    {
      QMessageBox::critical(NULL, "QmitkThresholdComponentsToolGUI", "PointSetNode does not contain a pointset");
      return;
    }

    mitk::Image *image = dynamic_cast<mitk::Image *>(m_InputImageNode->GetData());
	
	  int numberSeedPoints=pointSet->GetSize(); //Counts number of seeds
    mitk::Point3D seedPoint =
      pointSet
        ->GetPointSet(
          mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))->GetTimeStep())
        ->GetPoints()
        ->ElementAt(numberSeedPoints-1);

    if (image->GetGeometry()->IsInside(seedPoint))
      mitkPixelTypeMultiplex3(
        AccessPixel, image->GetChannelDescriptor().GetPixelType(), image, seedPoint, m_SeedpointValue) else return;

	//This section is for predictive thresholding once you set a point.
  // Initializing the region by the area around the seedpoint
  if (m_SeedInitialized) {
  	QmitkThresholdComponentsToolGUI::RunSegmentation();
  }
  
  else {
    m_SeedInitialized = true;
    m_SeedPointValueMean = 0;

    itk::Index<3> currentIndex, runningIndex;
    mitk::ScalarType pixelValues[125];
    unsigned int pos(0);

    image->GetGeometry(0)->WorldToIndex(seedPoint, currentIndex);
    runningIndex = currentIndex;

    for (int i = runningIndex[0] - 2; i <= runningIndex[0] + 2; i++)
    {
      for (int j = runningIndex[1] - 2; j <= runningIndex[1] + 2; j++)
      {
        for (int k = runningIndex[2] - 2; k <= runningIndex[2] + 2; k++)
        {
          currentIndex[0] = i;
          currentIndex[1] = j;
          currentIndex[2] = k;

          if (image->GetGeometry()->IsIndexInside(currentIndex))
          {
            int component = 0;
            m_InputImageNode->GetIntProperty("Image.Displayed Component", component);
            mitkPixelTypeMultiplex4(mitk::FastSinglePixelAccess,
                                    image->GetChannelDescriptor().GetPixelType(),
                                    image,
                                    NULL,
                                    currentIndex,
                                    pixelValues[pos]);

            pos++;
          }
          else
          {
            pixelValues[pos] = std::numeric_limits<long>::min();
            pos++;
          }
        }
      }
    }


    // Now calculation mean and stdev of the pixelValues
    unsigned int numberOfValues(0);
    for (auto &pixelValue : pixelValues)
    {
      if (pixelValue > std::numeric_limits<long>::min())
      {
        m_SeedPointValueMean += pixelValue;
        numberOfValues++;
      }
    }
    m_SeedPointValueMean = m_SeedPointValueMean / numberOfValues;

    mitk::ScalarType var = 0;
    if (numberOfValues > 1)
    {
      for (auto &pixelValue : pixelValues)
      {
        if (pixelValue > std::numeric_limits<mitk::ScalarType>::min())
        {
          var += (pixelValue - m_SeedPointValueMean) * (pixelValue - m_SeedPointValueMean);
        }
      }
      var /= numberOfValues - 1;
    }
    m_SeedPointValueVar = var;
    mitk::ScalarType stdDev = sqrt(var);


    mitk::ScalarType max = image->GetStatistics()->GetScalarValueMax();

    m_LOWERTHRESHOLD = m_SeedPointValueMean - stdDev;
    m_UPPERTHRESHOLD = max; //Always have default no limit on max intensity
    m_Controls.m_ThresholdSlider->setMaximumValue(m_UPPERTHRESHOLD);
    m_Controls.m_ThresholdSlider->setMinimumValue(m_LOWERTHRESHOLD);
      

	}
  }
}

void QmitkThresholdComponentsToolGUI::RunSegmentation()
{
  if (m_InputImageNode.IsNull())
  {
    QMessageBox::information(NULL, "Threshold Morphology functionality", "Please specify the image in Datamanager!");
    return;
  }

  mitk::DataNode::Pointer node = m_ThresholdComponentsTool->GetPointSetNode();

  if (node.IsNull())
  {
    QMessageBox::information(NULL, "Threshold Morphology functionality", "Please insert a seed point inside the "
                                                                            "image.\n\nFirst press the \"Define Seed "
                                                                            "Point\" button,\nthen click left mouse "
                                                                            "button inside the image.");
    return;
  }

  // safety if no pointSet or pointSet empty
  mitk::PointSet::Pointer seedPointSet = dynamic_cast<mitk::PointSet *>(node->GetData());
  if (seedPointSet.IsNull())
  {
    QMessageBox::information(
      NULL, "Threshold Morphology functionality", "The seed point is empty! Please choose a new seed point.");
    return;
  }

  int timeStep =
    mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))->GetTimeStep();

  if (!(seedPointSet->GetSize(timeStep)))
  {
    QMessageBox::information(
      NULL, "Threshold Morphology functionality", "The seed point is empty at this time! Please choose a new seed point.");
    return;
  }

  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));


  mitk::Image::Pointer orgImage = dynamic_cast<mitk::Image *>(m_InputImageNode->GetData());

  if (orgImage.IsNotNull())
  {
    if (orgImage->GetDimension() == 4)
    {
      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput(orgImage);
      timeSelector->SetTimeNr(timeStep);
      timeSelector->UpdateLargestPossibleRegion();
      mitk::Image *timedImage = timeSelector->GetOutput();
      AccessByItk_2(timedImage, StartSegmentation, timedImage->GetGeometry(), seedPointSet);
    }
    else if (orgImage->GetDimension() == 3)
    {

      AccessByItk_2(orgImage, StartSegmentation, orgImage->GetGeometry(), seedPointSet);

    }
    else
    {
      QApplication::restoreOverrideCursor(); // reset cursor
      QMessageBox::information(
        NULL, "Threshold Morphology functionality", "Only images of dimension 3 or 4 can be processed!");
      return;
    }
  }
  EnableControls(true); // Segmentation ran successfully, so enable all controls.
  node->SetVisibility(true);
  QApplication::restoreOverrideCursor(); // reset cursor
}

template <typename TPixel, unsigned int VImageDimension>
void QmitkThresholdComponentsToolGUI::StartSegmentation(itk::Image<TPixel, VImageDimension> *itkImage,
                                                           mitk::BaseGeometry *imageGeometry,
                                                           mitk::PointSet *seedPointSet)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef typename InputImageType::IndexType IndexType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, InputImageType> ThresholdFilterType;
  typedef itk::MaskImageFilter<InputImageType, InputImageType, InputImageType> MaskImageFilterType;
  typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();

  int numberSeedPoints=seedPointSet->GetSize(); //Counts number of seeds
  mitk::Point3D seedPoint;
  IndexType seedIndex;
  
  for( int counter=0; counter<numberSeedPoints; counter++) 
  {
   	 seedPoint=seedPointSet
        ->GetPointSet(
          mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))->GetTimeStep())
        ->GetPoints()
        ->ElementAt(counter);
        
    //errors with seed points
    if (!imageGeometry->IsInside(seedPoint))
  	{
   	 	QApplication::restoreOverrideCursor(); // reset cursor to be able to click ok with the regular mouse cursor
    		QMessageBox::information(NULL,
                             "Segmentation functionality",
                             "The seed point is outside of the image! Please choose a position inside the image!");
    		return;
  	}

    imageGeometry->WorldToIndex(seedPoint, seedIndex); // convert world coordinates to image indices
  	filter->AddSeed(seedIndex);   //place seed point into filter
	}


  filter->SetInput(itkImage);
  filter->SetLower(m_LOWERTHRESHOLD);
  filter->SetUpper(m_UPPERTHRESHOLD);

  try
  {
    filter->Update();
  }
  catch (itk::ExceptionObject &exc)
  {
    QMessageBox errorInfo;
    errorInfo.setWindowTitle("Threshold Morphology Segmentation Functionality");
    errorInfo.setIcon(QMessageBox::Critical);
    errorInfo.setText("An error occurred during segmentation!");
    errorInfo.setDetailedText(exc.what());
    errorInfo.exec();
    return; // can't work
  }
  catch (...)
  {
    QMessageBox::critical(NULL, "Threshold Morphology Segmentation Functionality", "An error occurred during segmentation!");
    return;
  }
  
  //Create segmentation
  mitk::Image::Pointer resultImage = mitk::ImportItkImage(filter->GetOutput())->Clone();
 
  // create new node and then delete the old one if there is one
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetData(resultImage);

  // set some properties
  newNode->SetVisibility(false);
  newNode->SetProperty("name", mitk::StringProperty::New(m_NAMEFORLABLEDSEGMENTATIONIMAGE));
  newNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  newNode->SetProperty("color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
  newNode->SetProperty("layer", mitk::IntProperty::New(1));
  newNode->SetProperty("opacity", mitk::FloatProperty::New(0.7));

  // delete the old image, if there was one:
  mitk::DataNode::Pointer binaryNode = m_DataStorage->GetNamedNode(m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  m_DataStorage->Remove(binaryNode);

  // now add result to data tree
  m_DataStorage->Add(newNode, m_InputImageNode);
  

  // Save segmentation  
  mitk::Image::Pointer originalSegmentation = dynamic_cast<mitk::Image *>(this->m_ThresholdComponentsTool->GetTargetSegmentationNode()->GetData());
  
  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, VImageDimension> SegmentationType;
  typename SegmentationType::Pointer originalSegmentationInITK = SegmentationType::New();
  mitk::CastToItkImage(resultImage, originalSegmentationInITK);
    
  int timeStep = mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))->GetTimeStep();
  originalSegmentation->SetVolume((void *)(originalSegmentationInITK->GetPixelContainer()->GetBufferPointer()), timeStep);
  originalSegmentation->Modified();
  
  //Create mask for 3D visualization
  typename InputImageType::Pointer inputImageItk;
  mitk::CastToItkImage<InputImageType>(resultImage, inputImageItk);
  

  typename MaskImageFilterType::Pointer maskFilter = MaskImageFilterType::New();
  maskFilter->SetInput(inputImageItk);
  maskFilter->SetInPlace(false);
  maskFilter->SetMaskImage(filter->GetOutput());
  maskFilter->SetOutsideValue(0);
  maskFilter->UpdateLargestPossibleRegion();

  mitk::Image::Pointer mitkMask;
  mitk::CastToMitkImage<InputImageType>(maskFilter->GetOutput(), mitkMask);
  mitk::DataNode::Pointer maskedNode = mitk::DataNode::New();
  maskedNode->SetData(mitkMask);

  // set some properties
  maskedNode->SetProperty("name", mitk::StringProperty::New(m_NAMEFORMASKEDSEGMENTATION));
  maskedNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  maskedNode->SetProperty("color", mitk::ColorProperty::New(0.0, 1.0, 0.0));
  maskedNode->SetProperty("layer", mitk::IntProperty::New(1));
  maskedNode->SetProperty("opacity", mitk::FloatProperty::New(0.0));

  // delete the old image, if there was one:
  mitk::DataNode::Pointer deprecatedMask = m_DataStorage->GetNamedNode(m_NAMEFORMASKEDSEGMENTATION);
  m_DataStorage->Remove(deprecatedMask);

  // now add result to data tree
  m_DataStorage->Add(maskedNode, m_InputImageNode);

  if (m_UseVolumeRendering)
    this->EnableVolumeRendering(true);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkThresholdComponentsToolGUI::ConfirmSegmentation()
{
  QmitkConfirmSegmentationDialog dialog;
  QString segName = QString::fromStdString(m_ThresholdComponentsTool->GetCurrentSegmentationName());

  dialog.SetSegmentationName(segName);
  int result = dialog.exec();

  switch (result)
  {
    case QmitkConfirmSegmentationDialog::CREATE_NEW_SEGMENTATION:
      m_ThresholdComponentsTool->SetOverwriteExistingSegmentation(false);
      break;
    case QmitkConfirmSegmentationDialog::OVERWRITE_SEGMENTATION:
      m_ThresholdComponentsTool->SetOverwriteExistingSegmentation(true);
      break;
    case QmitkConfirmSegmentationDialog::CANCEL_SEGMENTATION:
      return;
  }

	QmitkThresholdComponentsToolGUI::RunSegmentation();
	
  // disable volume rendering preview after the segmentation node was created
  this->EnableVolumeRendering(false);
  m_Controls.m_cbVolumeRendering->setChecked(false);

  if (m_ThresholdComponentsTool.IsNotNull())
  {
    m_ThresholdComponentsTool->ConfirmSegmentation();
  }
}

void QmitkThresholdComponentsToolGUI::EnableControls(bool enable)
{
  if (m_ThresholdComponentsTool.IsNull())
    return;

  mitk::DataNode::Pointer node = m_ThresholdComponentsTool->GetPointSetNode();

  // Check if a segmentation exists, if not leave segmentation dependent disabled.
  // if even m_DataStorage is NULL leave node NULL
  node = m_DataStorage ? m_DataStorage->GetNamedNode(m_NAMEFORLABLEDSEGMENTATIONIMAGE) : NULL;
  if (node.IsNull())
  {
    this->m_Controls.m_pbConfirmSegementation->setEnabled(false);
  }
  else
  {
    this->m_Controls.m_pbConfirmSegementation->setEnabled(enable);
  }

  this->m_Controls.m_cbVolumeRendering->setEnabled(enable);
}

void QmitkThresholdComponentsToolGUI::EnableVolumeRendering(bool enable)
{
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode(m_NAMEFORMASKEDSEGMENTATION);

  if (node.IsNull())
    return;

  if (m_MultiWidget)
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

void QmitkThresholdComponentsToolGUI::UseVolumeRendering(bool on)
{
  m_UseVolumeRendering = on;

  this->EnableVolumeRendering(on);
}

void QmitkThresholdComponentsToolGUI::SetLowerThresholdValue(double lowerThreshold)
{
  m_LOWERTHRESHOLD = lowerThreshold;
  
  //Re-segmentation each time the threshold is changed
  if (m_SeedInitialized)
  {
    QmitkThresholdComponentsToolGUI::RunSegmentation();
  }
}

void QmitkThresholdComponentsToolGUI::SetUpperThresholdValue(double upperThreshold)
{
  m_UPPERTHRESHOLD = upperThreshold;
    
  //Re-segmentation each time the threshold is changed
  if (m_SeedInitialized)
  {
    QmitkThresholdComponentsToolGUI::RunSegmentation();
  }
}

void QmitkThresholdComponentsToolGUI::Deactivated()
{
  // make the segmentation preview node invisible
  mitk::DataNode::Pointer node = m_DataStorage->GetNamedNode(m_NAMEFORLABLEDSEGMENTATIONIMAGE);
  if (node.IsNotNull())
  {
    node->SetVisibility(false);
  }

  // disable volume rendering preview after the segmentation node was created
  this->EnableVolumeRendering(false);
  m_Controls.m_cbVolumeRendering->setChecked(false);
}

void QmitkThresholdComponentsToolGUI::Activated()
{
}
