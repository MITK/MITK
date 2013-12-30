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

#include "mitkFastMarchingTool3D.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"
#include "mitkInteractionConst.h"
#include "mitkGlobalInteraction.h"
#include "mitkUndoController.h"
#include "mitkPositionEvent.h"
#include "mitkToolManager.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include "mitkStateMachineAction.h"
#include "mitkInteractionEvent.h"
#include "mitkInteractionPositionEvent.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, FastMarchingTool3D, "FastMarching3D tool");
}


mitk::FastMarchingTool3D::FastMarchingTool3D() : SegTool3D("PressMoveReleaseAndPointSetting"),
m_NeedUpdate(true),
m_CurrentTimeStep(0),
m_LowerThreshold(0),
m_UpperThreshold(200),
m_StoppingValue(100),
m_Sigma(1.0),
m_Alpha(-0.5),
m_Beta(3.0)
{
}

mitk::FastMarchingTool3D::~FastMarchingTool3D()
{
}

void mitk::FastMarchingTool3D::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION( "ShiftSecondaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION( "ShiftPrimaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION( "DeletePoint", OnDelete);
}

const char** mitk::FastMarchingTool3D::GetXPM() const
{
  return NULL;//mitkFastMarchingTool3D_xpm;
}

us::ModuleResource mitk::FastMarchingTool3D::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  assert(module);
  us::ModuleResource resource = module->GetResource("FastMarching_48x48.png");
  return resource;
}

const char* mitk::FastMarchingTool3D::GetName() const
{
  return "FastMarching3D";
}

void mitk::FastMarchingTool3D::SetUpperThreshold(double value)
{
  if ( m_UpperThreshold != value )
  {
    m_UpperThreshold = value;
    m_ThresholdFilter->SetUpperThreshold( m_UpperThreshold );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool3D::SetLowerThreshold(double value)
{
  if ( m_LowerThreshold != value )
  {
    m_LowerThreshold = value;
    m_ThresholdFilter->SetLowerThreshold( m_LowerThreshold );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool3D::SetBeta(double value)
{
  if ( m_Beta != value )
  {
    m_Beta = value;
    m_SigmoidFilter->SetBeta( m_Beta );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool3D::SetSigma(double value)
{
  if ( m_Sigma != value )
  {
    m_Sigma = value;
    m_GradientMagnitudeFilter->SetSigma( m_Sigma );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool3D::SetAlpha(double value)
{
  if ( m_Alpha != value )
  {
    m_Alpha = value;
    m_SigmoidFilter->SetAlpha( m_Alpha );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool3D::SetStoppingValue(double value)
{
  if ( m_StoppingValue != value )
  {
    m_StoppingValue = value;
    m_FastMarchingFilter->SetStoppingValue( m_StoppingValue );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool3D::Activated()
{
  Superclass::Activated();

  m_FeedbackImage = mitk::LabelSetImage::New();
  std::string name = "feedback";
  mitk::Color color;
  m_FeedbackImage->AddLabel(name, color);

  // feedback node and its visualization properties
  m_FeedbackNode = mitk::DataNode::New();
  m_FeedbackNode->SetName("feedback");

  m_FeedbackNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  m_FeedbackNode->SetProperty( "layer", IntProperty::New( 100 ) );
  m_FeedbackNode->SetProperty( "helper object", BoolProperty::New(true) );

  m_ToolManager->GetDataStorage()->Add( m_FeedbackNode, m_WorkingNode );

  m_SeedsAsPointSet = mitk::PointSet::New();
  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName("seeds");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add( m_SeedsAsPointSetNode, m_WorkingNode );

  m_ReferenceImage = dynamic_cast<mitk::Image*>(m_ToolManager->GetReferenceData(0)->GetData());
  if(m_ReferenceImage->GetTimeGeometry()->CountTimeSteps() > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
    timeSelector->SetInput( m_ReferenceImage );
    timeSelector->SetTimeNr( m_CurrentTimeStep );
    timeSelector->UpdateLargestPossibleRegion();
    m_ReferenceImage = timeSelector->GetOutput();
  }

/*
  m_SmoothFilter = SmoothingFilterType::New();
  m_SmoothFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_SmoothFilter->SetTimeStep( 0.038 );
  m_SmoothFilter->SetNumberOfIterations( 2 );
  m_SmoothFilter->SetConductanceParameter( 1.0 );
*/
  m_GradientMagnitudeFilter = GradientFilterType::New();
  m_GradientMagnitudeFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_GradientMagnitudeFilter->SetSigma( m_Sigma );

  m_SigmoidFilter = SigmoidFilterType::New();
  m_SigmoidFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_SigmoidFilter->SetAlpha( m_Alpha );
  m_SigmoidFilter->SetBeta( m_Beta );
  m_SigmoidFilter->SetOutputMinimum( 0.0 );
  m_SigmoidFilter->SetOutputMaximum( 1.0 );

  m_FastMarchingFilter = FastMarchingFilterType::New();
  m_FastMarchingFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_FastMarchingFilter->SetStoppingValue( m_StoppingValue );

  m_ThresholdFilter = ThresholdingFilterType::New();
  m_ThresholdFilter->SetLowerThreshold( m_LowerThreshold );
  m_ThresholdFilter->SetUpperThreshold( m_UpperThreshold );
  m_ThresholdFilter->SetOutsideValue( 0 );
  m_ThresholdFilter->SetInsideValue( 1.0 );

  m_SeedContainer = NodeContainer::New();
  m_SeedContainer->Initialize();
  m_FastMarchingFilter->SetTrialPoints( m_SeedContainer );

  //set up pipeline
  InternalImageType::Pointer referenceImageAsITK = InternalImageType::New();
  CastToItkImage(m_ReferenceImage, referenceImageAsITK);
  //m_SmoothFilter->SetInput( m_ReferenceImageAsITK );

//  m_SmoothFilter->SetInput( m_ReferenceImageAsITK );
  m_GradientMagnitudeFilter->SetInput( referenceImageAsITK ); //m_SmoothFilter->GetOutput() );
  m_SigmoidFilter->SetInput( m_GradientMagnitudeFilter->GetOutput() );
  m_FastMarchingFilter->SetInput( m_SigmoidFilter->GetOutput() );
  m_ThresholdFilter->SetInput( m_FastMarchingFilter->GetOutput() );
}

void mitk::FastMarchingTool3D::Deactivated()
{
  Superclass::Deactivated();
  m_ToolManager->GetDataStorage()->Remove( this->m_FeedbackNode );
  m_ToolManager->GetDataStorage()->Remove( this->m_SeedsAsPointSetNode );
  this->ClearSeeds();
//  this->m_SmoothFilter->RemoveAllObservers();
  this->m_SigmoidFilter->RemoveAllObservers();
  this->m_GradientMagnitudeFilter->RemoveAllObservers();
  this->m_FastMarchingFilter->RemoveAllObservers();
  m_FeedbackNode = NULL;

  unsigned int numberOfPoints = m_SeedsAsPointSet->GetSize();
  for (unsigned int i = 0; i < numberOfPoints; ++i)
  {
    mitk::Point3D point = m_SeedsAsPointSet->GetPoint(i);
    mitk::PointOperation* doOp = new mitk::PointOperation(mitk::OpREMOVE, point, 0);
    m_SeedsAsPointSet->ExecuteOperation(doOp);
  }

  m_ToolManager->GetDataStorage()->Remove(m_SeedsAsPointSetNode);
  m_SeedsAsPointSetNode = NULL;
  m_SeedsAsPointSet->RemoveObserver(m_PointSetAddObserverTag);
  m_SeedsAsPointSet->RemoveObserver(m_PointSetRemoveObserverTag);
}

void mitk::FastMarchingTool3D::ConfirmSegmentation()
{
  // combine preview image with current working segmentation
  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
  int activeLayer = workingImage->GetActiveLayer();
  int activePixelValue = workingImage->GetActiveLabel(activeLayer)->GetIndex();
  /*
  mitk::Image::Pointer diffImage = mitk::Image::New();

  PixelType pixelType(mitk::MakeScalarPixelType<DefaultSegmentationDataType>() );
  diffImage->Initialize( pixelType, workingImage->GetDimension(), workingImage->GetDimensions() );

  typedef itk::Image< unsigned char, 3 > SegmentationImageType;
  SegmentationImageType::Pointer itkLabelImage;
  mitk::CastToItkImage( workingImage, itkLabelImage );

  diffImage->SetImportVolume(itkLabelImage->GetBufferPointer());

  AffineGeometryFrame3D::Pointer originalGeometryAGF = workingImage->GetTimeSlicedGeometry()->Clone();
  TimeSlicedGeometry::Pointer originalGeometry = dynamic_cast<TimeSlicedGeometry*>( originalGeometryAGF.GetPointer() );
  diffImage->SetGeometry( originalGeometry );
  */
//  this->WritePreviewOnWorkingImage( workingImage, m_FeedbackImage, activePixelValue, m_CurrentTimeStep );

  this->ClearSeeds();

  //the workingImage was modified within the pipeline, but not marked so
  workingImage->Modified();
//  workingImage->GetVtkImageData()->Modified();

  m_FeedbackImage->EraseLabel(1, false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);
}

bool mitk::FastMarchingTool3D::OnAddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return false;

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize(), positionEvent->GetPositionInWorld());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  mitk::Point3D clickInIndex;
  m_ReferenceImage->GetGeometry()->WorldToIndex(positionEvent->GetPositionInWorld(), clickInIndex);

  itk::Index<3> seedPosition;
  seedPosition[0] = clickInIndex[0];
  seedPosition[1] = clickInIndex[1];
  seedPosition[2] = clickInIndex[2];

  NodeType node;
  const double seedValue = 0.0;
  node.SetValue( seedValue );
  node.SetIndex( seedPosition );
  this->m_SeedContainer->InsertElement(this->m_SeedContainer->Size(), node);
  m_FastMarchingFilter->Modified();

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize(), positionEvent->GetPositionInWorld());

  m_NeedUpdate = true;

  this->Update();

  m_ReadyMessage.Send();

  return true;
}

bool mitk::FastMarchingTool3D::OnDelete(StateMachineAction*, InteractionEvent* interactionEvent)
{
  // delete last seed point
  if(!(this->m_SeedContainer->empty()))
  {
    //delete last element of seeds container
    this->m_SeedContainer->pop_back();
    m_FastMarchingFilter->Modified();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    m_NeedUpdate = true;

    this->Update();
  }
  return true;
}

void mitk::FastMarchingTool3D::Update()
{
  if (m_NeedUpdate)
  {
    m_ProgressCommand->AddStepsToDo(100);
    CurrentlyBusy.Send(true);
    try
    {
      m_ThresholdFilter->Update();
    }
    catch( itk::ExceptionObject & e )
    {
     MITK_ERROR << "Exception caught: " << e.GetDescription();

     m_ProgressCommand->Reset();
     CurrentlyBusy.Send(false);

     std::string msg = e.GetDescription();
     ErrorMessage.Send(msg);

     return;
    }
    m_ProgressCommand->Reset();
    CurrentlyBusy.Send(false);

    OutputImageType::Pointer output = m_ThresholdFilter->GetOutput();
    output->DisconnectPipeline();

    m_FeedbackImage->InitializeByItk(output.GetPointer());
    m_FeedbackImage->SetChannel(output->GetBufferPointer());

    m_FeedbackImage->SetGeometry( m_ReferenceImage->GetGeometry(0)->Clone().GetPointer() );

    mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
    assert(workingImage);

    const mitk::Color& color = workingImage->GetActiveLabelColor();

    m_FeedbackImage->SetLabelColor(1, color);
    m_FeedbackNode->SetData(m_FeedbackImage);

    m_NeedUpdate = false;

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::FastMarchingTool3D::ClearSeeds()
{
  // clear seeds for FastMarching as well as the PointSet for visualization
  if(this->m_SeedContainer.IsNotNull())
    this->m_SeedContainer->Initialize();

  this->m_SeedsAsPointSet = mitk::PointSet::New(); // m_SeedsAsPointSet->Clear() does not work
  this->m_SeedsAsPointSetNode->SetData(this->m_SeedsAsPointSet);

  if(this->m_FastMarchingFilter.IsNotNull())
    m_FastMarchingFilter->Modified();

  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::SetCurrentTimeStep(int t)
{
  if( m_CurrentTimeStep != t )
  {
    m_CurrentTimeStep = t;

    if(m_ReferenceImage->GetTimeSlicedGeometry()->CountTimeSteps() > 1)
    {
      mitk::ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
      timeSelector->SetInput( m_ReferenceImage );
      timeSelector->SetTimeNr( m_CurrentTimeStep );
      timeSelector->UpdateLargestPossibleRegion();
      m_ReferenceImage = timeSelector->GetOutput();
    }
  }
}
