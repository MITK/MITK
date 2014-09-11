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
#include "mitkRegionGrow3DTool.h"
#include "mitkToolManager.h"
#include "mitkRenderingManager.h"
#include "mitkLevelWindowProperty.h"
#include "mitkPointSetInteractor.h"
#include "mitkGlobalInteraction.h"
#include "mitkITKImageImport.h"

#include "itkImage.h"
#include "itkBinaryThresholdImageFilter.h"
#include "itkConnectedAdaptiveThresholdImageFilter.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkMaskAndCutRoiImageFilter.h"
#include "mitkPadImageFilter.h"

#include "mitkRegionGrow3DTool.xpm"

namespace mitk {
  MITK_TOOL_MACRO(MitkSegmentation_EXPORT, RegionGrow3DTool, "RegionGrower 3D");
}

mitk::RegionGrow3DTool::RegionGrow3DTool(): Tool("PressMoveRelease"),
m_LowerThreshold(-5000),
m_UpperThreshold(5000),
m_CurrentRGDirectionIsUpwards(false)
{

  CONNECT_FUNCTION( "Release", OnMouseReleased);

  m_FeedbackNode = DataNode::New();
  m_FeedbackNode->SetProperty( "color", ColorProperty::New(1.0, 0.0, 0.0) );
  m_FeedbackNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  m_FeedbackNode->SetProperty( "layer", IntProperty::New( 100 ) );
  m_FeedbackNode->SetProperty( "levelwindow", LevelWindowProperty::New( LevelWindow(100, 1) ) );
  m_FeedbackNode->SetProperty( "name", StringProperty::New("failsafe region grow feedback") );
  m_FeedbackNode->SetProperty( "opacity", FloatProperty::New(0.3) );
  m_FeedbackNode->SetProperty( "helper object", BoolProperty::New(true) );
  m_FeedbackNode->SetVisibility(false);

  m_PointSetNode = mitk::DataNode::New();
  m_PointSetNode->SetName("regiongrow3d pointset");
  m_PointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
  m_PointSetNode->SetProperty("layer", mitk::IntProperty::New(2));

  mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
  m_PointSetNode->SetData(pointSet);
}

mitk::RegionGrow3DTool::~RegionGrow3DTool()
{

}

bool mitk::RegionGrow3DTool::OnMouseReleased( StateMachineAction*, InteractionEvent* /*interactionEvent*/ )
{
    mitk::PointSetInteractor::Pointer interactor = dynamic_cast<mitk::PointSetInteractor*> (m_PointSetNode->GetInteractor());
    if (interactor.IsNotNull())
    {
      mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*> (m_PointSetNode->GetData());
      if (pointSet.IsNotNull())
      {
        if (pointSet->GetSize() == 1)
        {
          //check if we have a valid picture
          this->RunSegmentation();
          SeedButtonToggled.Send(false);
        }
      }
    }
  return true;
}

void mitk::RegionGrow3DTool::Activated()
{
  if (m_ToolManager)
  {
    m_ToolManager->RoiDataChanged += mitk::MessageDelegate<mitk::RegionGrow3DTool>(this, &mitk::RegionGrow3DTool::UpdatePreview);
    m_OriginalImageNode = m_ToolManager->GetReferenceData(0);
    m_NodeToProceed = m_OriginalImageNode;

    if (m_NodeToProceed.IsNotNull())
    {
      SetupPreviewNodeFor(m_NodeToProceed);
      mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (m_OriginalImageNode->GetData());
      if (image.IsNotNull())
      {
       m_RoiMin = image->GetScalarValueMin();
       m_RoiMax = image->GetScalarValueMax();
      }
    }
    else
      m_ToolManager->ActivateTool(-1);
  }
}

void mitk::RegionGrow3DTool::Deactivated()
{
  m_ToolManager->RoiDataChanged -= mitk::MessageDelegate<mitk::RegionGrow3DTool>(this, &mitk::RegionGrow3DTool::UpdatePreview);
  if (mitk::DataStorage* ds = m_ToolManager->GetDataStorage())
  {
    ds->Remove(m_PointSetNode);
    ds->Remove(m_FeedbackNode);
  }
  m_FeedbackNode->SetData(NULL);
  m_FeedbackNode->SetLevelWindow(NULL);
  m_FeedbackNode->SetVisibility(false);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

const char* mitk::RegionGrow3DTool::GetName() const
{
  return "RegionGrower 3D";
}

const char** mitk::RegionGrow3DTool::GetXPM() const
{
  return mitkRegionGrow3DTool_xpm;
}

void mitk::RegionGrow3DTool::SetSeedPoint(bool toggled)
{
  if (!m_ToolManager->GetDataStorage()->Exists(m_PointSetNode)) //no pointSet present
    m_ToolManager->GetDataStorage()->Add(m_PointSetNode, m_OriginalImageNode);

  //add Interactor if there is none and add it to GlobalInteraction
  if (toggled == true) // button is down
  {
    mitk::PointSetInteractor::Pointer interactor = dynamic_cast<mitk::PointSetInteractor*> (m_PointSetNode->GetInteractor());
    if (interactor.IsNull())
    {
      //create a new interactor and add it to node
      interactor = mitk::PointSetInteractor::New("singlepointinteractorwithoutshiftclick", m_PointSetNode, 1);
    }
    mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);
  }
  else
  {
    mitk::PointSetInteractor::Pointer interactor = dynamic_cast<mitk::PointSetInteractor*> (m_PointSetNode->GetInteractor());
    if (interactor.IsNotNull())
    {
      m_PointSetNode->SetInteractor(NULL);
      mitk::GlobalInteraction::GetInstance()->RemoveInteractor(interactor);
    }
  }
}

void mitk::RegionGrow3DTool::RunSegmentation()
{
  //safety if no pointSet or pointSet empty
  mitk::PointSet::Pointer seedPointSet = dynamic_cast<mitk::PointSet*> (m_PointSetNode->GetData());
  if (seedPointSet.IsNull())
  {
    return;
  }
  if (!(seedPointSet->GetSize() > 0))
  {
    return;
  }

  mitk::PointSet::PointType seedPoint = seedPointSet->GetPointSet()->GetPoints()->Begin().Value();

  mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (m_NodeToProceed->GetData());

  if (image.IsNotNull())
  {
    m_LowerThreshold = static_cast<int> (m_RoiMin);
    m_UpperThreshold = static_cast<int> (m_RoiMax);
    AccessByItk_2(image, StartRegionGrowing, image->GetGeometry(), seedPoint);
  }
}

template<typename TPixel, unsigned int VImageDimension>
void mitk::RegionGrow3DTool::StartRegionGrowing(itk::Image<TPixel, VImageDimension>* itkImage, mitk::BaseGeometry* imageGeometry, mitk::PointSet::PointType seedPoint)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef typename InputImageType::IndexType IndexType;
  typedef itk::ConnectedAdaptiveThresholdImageFilter<InputImageType, InputImageType> RegionGrowingFilterType;
  typename RegionGrowingFilterType::Pointer regionGrower = RegionGrowingFilterType::New();

  if ( !imageGeometry->IsInside(seedPoint) )
  {
    return;
  }

  IndexType seedIndex;
  imageGeometry->WorldToIndex( seedPoint, seedIndex);// convert world coordinates to image indices

  //int seedValue = itkImage->GetPixel(seedIndex);

  regionGrower->SetInput( itkImage );
  regionGrower->AddSeed( seedIndex );
  regionGrower->SetLower( m_LowerThreshold );
  regionGrower->SetUpper( m_UpperThreshold );
  regionGrower->SetGrowingDirectionIsUpwards( m_CurrentRGDirectionIsUpwards );

  try
  {
    regionGrower->Update();
  }
  catch( ... )
  {
    MITK_ERROR << "Something went wrong!"  << endl;
    return;
  }

  m_SeedpointValue = regionGrower->GetSeedpointValue();

  //initialize slider
  if(m_CurrentRGDirectionIsUpwards)
  {
    UpperThresholdValueChanged.Send(m_UpperThreshold);
    LowerThresholdValueChanged.Send(m_SeedpointValue);
  }
  else
  {
    UpperThresholdValueChanged.Send(m_SeedpointValue);
    LowerThresholdValueChanged.Send(m_LowerThreshold);
  }

  m_DetectedLeakagePoint = regionGrower->GetLeakagePoint();

  mitk::Image::Pointer resultImage = mitk::ImportItkImage(regionGrower->GetOutput())->Clone();

  m_FeedbackNode->SetData( resultImage );
  m_FeedbackNode->SetVisibility(true);

  InitializeLevelWindow();
}

void mitk::RegionGrow3DTool::InitializeLevelWindow()
{

  mitk::LevelWindow tempLevelWindow;
  m_FeedbackNode->GetLevelWindow(tempLevelWindow, NULL, "levelWindow");

  mitk::ScalarType* level = new mitk::ScalarType(0.5);
  mitk::ScalarType* window = new mitk::ScalarType(1);

  int upper;
  if (m_CurrentRGDirectionIsUpwards)
  {
    upper = m_UpperThreshold - m_SeedpointValue;
  }
  else
  {
    upper = m_SeedpointValue - m_LowerThreshold;
  }

  tempLevelWindow.SetRangeMinMax(mitk::ScalarType(0), mitk::ScalarType(upper));
  tempLevelWindow.SetLevelWindow(*level, *window);

  m_FeedbackNode->SetLevelWindow(tempLevelWindow, NULL, "levelWindow");

  //get the suggested threshold from the detected leakage-point and adjust the slider

  if (m_CurrentRGDirectionIsUpwards)
  {
    SliderValueChanged.Send(m_SeedpointValue + m_DetectedLeakagePoint -1);
  }
  else
  {
    SliderValueChanged.Send(m_SeedpointValue - m_DetectedLeakagePoint +1);
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void mitk::RegionGrow3DTool::ChangeLevelWindow(int value)
{
  if (m_FeedbackNode.IsNull())
    return;

  mitk::LevelWindow tempLevelWindow;

  m_FeedbackNode->GetLevelWindow(tempLevelWindow, NULL, "levelWindow"); //get the levelWindow associated with the preview

  mitk::ScalarType level;// = this->m_UPPERTHRESHOLD - newValue + 0.5;
  mitk::ScalarType* window = new mitk::ScalarType(1);

  //adjust the levelwindow according to the position of the slider (newvalue)
  if (m_CurrentRGDirectionIsUpwards)
  {
    level = m_UpperThreshold - value + 0.5;
    tempLevelWindow.SetLevelWindow(level, *window);
  }
  else
  {
    level = value - m_LowerThreshold +0.5;
    tempLevelWindow.SetLevelWindow(level, *window);
  }

  m_FeedbackNode->SetLevelWindow(tempLevelWindow, NULL, "levelWindow");

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::RegionGrow3DTool::ConfirmSegmentation( std::string name, mitk::Color color)
{
  mitk::DataNode::Pointer new_node = mitk::DataNode::New();
  new_node->SetColor(color);
  new_node->SetName(name);
  new_node->SetProperty("binary", mitk::BoolProperty::New("true"));

  mitk::Image* image = dynamic_cast<mitk::Image*> (m_FeedbackNode->GetData());

  mitk::LevelWindow tempLevelWindow;
  m_FeedbackNode->GetLevelWindow( tempLevelWindow, NULL, "levelWindow");
  int upperThresholdLabeledImage = (short int) tempLevelWindow.GetRangeMax();
  int lowerThresholdLabeledImage = (short int) tempLevelWindow.GetLowerWindowBound() + 1;

  typedef itk::Image<int, 3> InputImageType;
  typedef itk::Image<unsigned char, 3> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<InputImageType, SegmentationType> ThresholdFilterType;

  ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
  InputImageType::Pointer itkImage;
  mitk::CastToItkImage(image, itkImage);
  filter->SetInput(itkImage);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->SetUpperThreshold(upperThresholdLabeledImage);
  filter->SetLowerThreshold(lowerThresholdLabeledImage);
  filter->Update();
  mitk::Image::Pointer new_image = mitk::Image::New();
  mitk::CastToMitkImage(filter->GetOutput(), new_image);


  //pad to original size
  if (m_OriginalImageNode.GetPointer() != m_NodeToProceed.GetPointer())
  {
    mitk::PadImageFilter::Pointer padFilter = mitk::PadImageFilter::New();

    padFilter->SetInput(0, new_image);
    padFilter->SetInput(1, dynamic_cast<mitk::Image*> (m_OriginalImageNode->GetData()));
    padFilter->SetBinaryFilter(true);
    padFilter->SetUpperThreshold(1);
    padFilter->SetLowerThreshold(1);
    padFilter->Update();

    new_image = padFilter->GetOutput();
  }

  new_node->SetData(new_image);
  m_ToolManager->GetDataStorage()->Add(new_node);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  m_ToolManager->ActivateTool(-1);
}

void mitk::RegionGrow3DTool::CancelSegmentation()
{
  m_ToolManager->ActivateTool(-1);
}

void mitk::RegionGrow3DTool::SetupPreviewNodeFor( DataNode* nodeToProceed)
{
  if (nodeToProceed)
  {
    Image::Pointer image = dynamic_cast<Image*>( nodeToProceed->GetData() );

    if (image.IsNotNull())
    {
      m_FeedbackNode->SetData( image );
      int layer(50);
      nodeToProceed->GetIntProperty("layer", layer);
      m_FeedbackNode->SetIntProperty("layer", layer+1);
      m_FeedbackNode->SetLevelWindow(NULL);

      if (DataStorage* storage = m_ToolManager->GetDataStorage())
      {
        if (storage->Exists(m_FeedbackNode))
          storage->Remove(m_FeedbackNode);
        storage->Add( m_FeedbackNode, nodeToProceed );
      }
    }
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::RegionGrow3DTool::UpdatePreview()
{
  typedef itk::Image<int, 3> ItkImageType;
  typedef itk::Image<unsigned char, 3> ItkMaskType;

  mitk::DataNode::Pointer node = m_ToolManager->GetRoiData(0);
  if (node.IsNull())
  {
    this->SetupPreviewNodeFor(m_OriginalImageNode);
    m_NodeToProceed = m_OriginalImageNode;
    mitk::Image::Pointer image = dynamic_cast<mitk::Image*> (m_OriginalImageNode->GetData());
    if (image.IsNotNull())
    {
      m_RoiMin = image->GetScalarValueMin();
      m_RoiMax = image->GetScalarValueMax();
    }
    return;
  }

  mitk::MaskAndCutRoiImageFilter::Pointer roiFilter = mitk::MaskAndCutRoiImageFilter::New();
  roiFilter->SetInput(dynamic_cast<mitk::Image*> (m_NodeToProceed->GetData()));

  roiFilter->SetRegionOfInterest(node->GetData());
  roiFilter->Update();

  mitk::DataNode::Pointer new_node = mitk::DataNode::New();
  mitk::Image::Pointer tmpImage = roiFilter->GetOutput();
  new_node->SetData(tmpImage);

  m_RoiMax = roiFilter->GetMaxValue();
  m_RoiMin = roiFilter->GetMinValue();

  this->SetupPreviewNodeFor(new_node);
  m_NodeToProceed = new_node;

  //this->RunSegmentation();
}

void mitk::RegionGrow3DTool::SetCurrentRGDirectionIsUpwards(bool flag)
{
  m_CurrentRGDirectionIsUpwards = flag;
}
