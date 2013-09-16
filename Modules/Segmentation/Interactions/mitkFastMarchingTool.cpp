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

#include "mitkFastMarchingTool.h"

#include "mitkToolManager.h"
#include "mitkRenderingManager.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"
#include "mitkBaseRenderer.h"

// us
#include "mitkModule.h"
#include "mitkModuleResource.h"
#include <mitkGetModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, FastMarchingTool, "FastMarching2D tool");
}


mitk::FastMarchingTool::FastMarchingTool() : SegTool2D("PressMoveReleaseAndPointSetting"),
m_NeedUpdate(true),
m_CurrentTimeStep(0),
m_LowerThreshold(0),
m_UpperThreshold(200),
m_StoppingValue(100),
m_Sigma(1.0),
m_Alpha(-2.0),
m_Beta(3.0),
m_PositionEvent(0),
m_Initialized(false)
{
  CONNECT_ACTION( AcADDPOINTRMB, OnAddPoint );
  CONNECT_ACTION( AcADDPOINT, OnAddPoint );
  CONNECT_ACTION( AcREMOVEPOINT, OnDelete );
}

mitk::FastMarchingTool::~FastMarchingTool()
{
}

float mitk::FastMarchingTool::CanHandleEvent( StateEvent const *stateEvent) const
{
  float returnValue = Superclass::CanHandleEvent(stateEvent);

  //we can handle delete
  if(stateEvent->GetId() == 12 )
  {
    returnValue = 1.0;
  }

  return returnValue;
}

const char** mitk::FastMarchingTool::GetXPM() const
{
  return NULL;//mitkFastMarchingTool_xpm;
}

mitk::ModuleResource mitk::FastMarchingTool::GetIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("FastMarching_48x48.png");
  return resource;
}

mitk::ModuleResource mitk::FastMarchingTool::GetCursorIconResource() const
{
  Module* module = GetModuleContext()->GetModule();
  ModuleResource resource = module->GetResource("FastMarching_Cursor_32x32.png");
  return resource;
}

const char* mitk::FastMarchingTool::GetName() const
{
  return "FastMarching2D";
}

void mitk::FastMarchingTool::SetUpperThreshold(double value)
{
  if (!m_Initialized) return;

  if (m_UpperThreshold != value)
  {
    m_UpperThreshold = value / 10.0;
    m_ThresholdFilter->SetUpperThreshold( m_UpperThreshold );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetLowerThreshold(double value)
{
  if (!m_Initialized) return;

  if (m_LowerThreshold != value)
  {
    m_LowerThreshold = value / 10.0;
    m_ThresholdFilter->SetLowerThreshold( m_LowerThreshold );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetBeta(double value)
{
  if (!m_Initialized) return;

  if (m_Beta != value)
  {
    m_Beta = value;
    m_SigmoidFilter->SetBeta( m_Beta );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetSigma(double value)
{
  if (!m_Initialized) return;

  if (m_Sigma != value)
  {
    m_Sigma = value;
    m_GradientMagnitudeFilter->SetSigma( m_Sigma );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetAlpha(double value)
{
  if (!m_Initialized) return;

  if (m_Alpha != value)
  {
    m_Alpha = value;
    m_SigmoidFilter->SetAlpha( m_Alpha );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetStoppingValue(double value)
{
  if (!m_Initialized) return;

  if (m_StoppingValue != value)
  {
    m_StoppingValue = value;
    m_FastMarchingFilter->SetStoppingValue( m_StoppingValue );
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::Activated()
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

  m_ToolManager->GetDataStorage()->Add( m_FeedbackNode, m_ToolManager->GetWorkingData(0) );

  m_SeedsAsPointSet = mitk::PointSet::New();
  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName("seeds");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add( m_SeedsAsPointSetNode, m_ToolManager->GetWorkingData(0) );

  m_ProgressCommand = mitk::ToolCommand::New();

  m_SmoothFilter = SmoothingFilterType::New();
  m_SmoothFilter->SetTimeStep( 0.025 );
  m_SmoothFilter->SetNumberOfIterations( 3 );
  m_SmoothFilter->SetConductanceParameter( 9.0 );

  m_GradientMagnitudeFilter = GradientFilterType::New();
  m_GradientMagnitudeFilter->SetSigma( m_Sigma );

  m_SigmoidFilter = SigmoidFilterType::New();
  m_SigmoidFilter->SetAlpha( m_Alpha );
  m_SigmoidFilter->SetBeta( m_Beta );
  m_SigmoidFilter->SetOutputMinimum( 0.0 );
  m_SigmoidFilter->SetOutputMaximum( 1.0 );

  m_FastMarchingFilter = FastMarchingFilterType::New();
  m_FastMarchingFilter->SetStoppingValue( m_StoppingValue );

  m_ThresholdFilter = ThresholdingFilterType::New();
  m_ThresholdFilter->SetLowerThreshold( m_LowerThreshold );
  m_ThresholdFilter->SetUpperThreshold( m_UpperThreshold );
  m_ThresholdFilter->SetOutsideValue( 0 );
  m_ThresholdFilter->SetInsideValue( 1 );

  m_SeedContainer = NodeContainer::New();
  m_SeedContainer->Initialize();
  m_FastMarchingFilter->SetTrialPoints( m_SeedContainer );

  m_SmoothFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_GradientMagnitudeFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_SigmoidFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);
  m_FastMarchingFilter->AddObserver( itk::ProgressEvent(), m_ProgressCommand);

//  m_SmoothFilter->SetInput( referenceImageSliceAsITK );
  m_GradientMagnitudeFilter->SetInput( m_SmoothFilter->GetOutput() );
  m_SigmoidFilter->SetInput( m_GradientMagnitudeFilter->GetOutput() );
  m_FastMarchingFilter->SetInput( m_SigmoidFilter->GetOutput() );
  m_ThresholdFilter->SetInput( m_FastMarchingFilter->GetOutput() );
}

void mitk::FastMarchingTool::Deactivated()
{
  Superclass::Deactivated();

  this->m_SmoothFilter->RemoveAllObservers();
  this->m_SigmoidFilter->RemoveAllObservers();
  this->m_GradientMagnitudeFilter->RemoveAllObservers();
  this->m_FastMarchingFilter->RemoveAllObservers();

  m_ToolManager->GetDataStorage()->Remove( this->m_FeedbackNode );
  m_ToolManager->GetDataStorage()->Remove( this->m_SeedsAsPointSetNode );
  this->ClearSeeds();
  m_FeedbackNode = NULL;
  m_SeedsAsPointSetNode = NULL;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::FastMarchingTool::ConfirmSegmentation()
{
  mitk::Image::Pointer workingImageSlice = GetAffectedWorkingSlice( m_PositionEvent );

  // paste the binary segmentation to the current working slice
  mitk::SegTool2D::PasteSegmentationOnWorkingImage( workingImageSlice, m_FeedbackImage, m_ToolManager->GetActiveLabel()->GetIndex(), m_CurrentTimeStep );

  // paste current working slice back to the working image
  mitk::SegTool2D::WriteBackSegmentationResult(m_PositionEvent, workingImageSlice);

  this->ClearSeeds();

  m_FeedbackImage->EraseLabel(1,false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

bool mitk::FastMarchingTool::OnAddPoint(Action* action, const StateEvent* stateEvent)
{
  // Add a new seed point for FastMarching algorithm
  const PositionEvent* p = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!p) return false;

  if (m_PositionEvent != NULL)
      delete m_PositionEvent;
  m_PositionEvent = new PositionEvent(p->GetSender(), p->GetType(), p->GetButton(), p->GetButtonState(), p->GetKey(), p->GetDisplayPosition(), p->GetWorldPosition() );

  //if click was on another renderwindow or slice then reset pipeline and preview
  if( (m_LastEventSender != m_PositionEvent->GetSender()) || (m_LastEventSlice != m_PositionEvent->GetSender()->GetSlice()) )
  {
    m_Initialized = true;
    InternalImageType::Pointer referenceImageSliceAsITK = InternalImageType::New();
    m_ReferenceImageSlice = SegTool2D::GetAffectedReferenceSlice( m_PositionEvent );
    CastToItkImage(m_ReferenceImageSlice, referenceImageSliceAsITK);
    m_SmoothFilter->SetInput( referenceImageSliceAsITK );

    this->ClearSeeds();
  }

  m_LastEventSender = m_PositionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  mitk::Point3D clickInIndex;

  m_ReferenceImageSlice->GetGeometry()->WorldToIndex(m_PositionEvent->GetWorldPosition(), clickInIndex);
  itk::Index<2> seedPosition;
  seedPosition[0] = clickInIndex[0];
  seedPosition[1] = clickInIndex[1];

  NodeType node;
  const double seedValue = 0.0;
  node.SetValue( seedValue );
  node.SetIndex( seedPosition );
  this->m_SeedContainer->InsertElement(this->m_SeedContainer->Size(), node);
  m_FastMarchingFilter->Modified();

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize(), m_PositionEvent->GetWorldPosition());

  m_NeedUpdate = true;

  this->Update();

  return true;
}

bool mitk::FastMarchingTool::OnDelete(Action* action, const StateEvent* stateEvent)
{
  // delete last seed point
  if(!(this->m_SeedContainer->empty()))
  {
    //delete last element of seeds container
    this->m_SeedContainer->pop_back();
    m_FastMarchingFilter->Modified();

    //delete last point in pointset - somehow ugly
    m_SeedsAsPointSet->GetPointSet()->GetPoints()->DeleteIndex(m_SeedsAsPointSet->GetSize() - 1);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    m_NeedUpdate = true;

    this->Update();
  }
  return true;
}

void mitk::FastMarchingTool::Update()
{
  // update FastMarching pipeline and show result
  if (m_NeedUpdate)
  {
    m_ProgressCommand->AddStepsToDo(20);
    CurrentlyBusy.Send(true);
    try
    {
      m_ThresholdFilter->Update();
    }
    catch( itk::ExceptionObject & e )
    {
     MITK_ERROR << "Exception caught: " << e.GetDescription();

     m_ProgressCommand->SetRemainingProgress(100);
     CurrentlyBusy.Send(false);
     std::string msg = e.GetDescription();
     ErrorMessage.Send(msg);
     return;
    }

    m_ProgressCommand->SetRemainingProgress(100);
    CurrentlyBusy.Send(false);

    OutputImageType::Pointer output = m_ThresholdFilter->GetOutput();
    output->DisconnectPipeline();

    m_FeedbackImage->InitializeByItk(output.GetPointer());
    m_FeedbackImage->SetChannel(output->GetBufferPointer());

    m_FeedbackImage->SetGeometry( m_ReferenceImageSlice->GetGeometry(0)->Clone().GetPointer() );

    const mitk::Color& color = m_ToolManager->GetActiveLabel()->GetColor();

    m_FeedbackImage->SetLabelColor(1, color);
    m_FeedbackNode->SetData(m_FeedbackImage);

    m_NeedUpdate = false;

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::FastMarchingTool::ClearSeeds()
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
