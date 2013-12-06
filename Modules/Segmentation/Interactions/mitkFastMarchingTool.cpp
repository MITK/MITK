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
#include "mitkBaseRenderer.h"

// us
#include <usModule.h>
#include <usModuleResource.h>
#include <usGetModuleContext.h>
#include <usModuleContext.h>

namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, FastMarchingTool, "FastMarching2D tool");
}

mitk::FastMarchingTool::FastMarchingTool() : SegTool2D("PressMoveReleaseAndPointSetting"),
m_NeedUpdate(true),
m_CurrentTimeStep(0),
m_LowerThreshold(0),
m_UpperThreshold(200),
m_StoppingValue(2000),
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

us::ModuleResource mitk::FastMarchingTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("FastMarching_48x48.png");
  return resource;
}

us::ModuleResource mitk::FastMarchingTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("FastMarching_Cursor_32x32.png");
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
  std::string name("feedback");
  mitk::Color color;
  color.SetRed(1.0);
  color.SetGreen(0.0);
  color.SetBlue(0.0);
  m_FeedbackImage->AddLabel(name, color);

  // feedback node and its visualization properties
  m_FeedbackNode = mitk::DataNode::New();
  m_FeedbackNode->SetName("feedback");

  m_FeedbackNode->SetProperty( "texture interpolation", BoolProperty::New(false) );
  m_FeedbackNode->SetProperty( "layer", IntProperty::New( 100 ) );
  m_FeedbackNode->SetProperty( "helper object", BoolProperty::New(true) );

  DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  m_ToolManager->GetDataStorage()->Add( m_FeedbackNode, workingNode );

  m_SeedsAsPointSet = mitk::PointSet::New();
  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName("seeds");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetBoolProperty("updateDataOnRender", true);
  m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add( m_SeedsAsPointSetNode, m_ToolManager->GetWorkingData(0) );
}

void mitk::FastMarchingTool::Deactivated()
{
  Superclass::Deactivated();

  m_ToolManager->GetDataStorage()->Remove( this->m_FeedbackNode );
  m_ToolManager->GetDataStorage()->Remove( this->m_SeedsAsPointSetNode );
  this->ClearSeeds();
  m_FeedbackNode = NULL;
  m_SeedsAsPointSetNode = NULL;
  m_LastEventSender = NULL;
  m_LastEventSlice = -1;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::FastMarchingTool::AcceptPreview()
{
  mitk::Image::Pointer workingImageSlice = SegTool2D::GetAffectedWorkingSlice( m_PositionEvent );

  mitk::LabelSetImage* image = dynamic_cast<mitk::LabelSetImage*>(m_ToolManager->GetWorkingData(0)->GetData());
  int activeLayer = image->GetActiveLayer();
  int activePixelValue = image->GetActiveLabel(activeLayer)->GetIndex();

  // paste the binary segmentation to the current working slice
  mitk::SegTool2D::PasteSegmentationOnWorkingImage( workingImageSlice, m_FeedbackImage, activePixelValue, m_CurrentTimeStep );

  // paste current working slice back to the working image
  mitk::SegTool2D::WriteBackSegmentationResult(m_PositionEvent, workingImageSlice);

  this->ClearSeeds();

  m_FeedbackImage->EraseLabel(1,false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);
}

bool mitk::FastMarchingTool::OnAddPoint(Action* action, const StateEvent* stateEvent)
{
  // Add a new seed point for FastMarching algorithm
  const PositionEvent* p = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!p) return false;

  if (m_PositionEvent != NULL)
      delete m_PositionEvent;
  m_PositionEvent = new PositionEvent(p->GetSender(), p->GetType(), p->GetButton(), p->GetButtonState(), p->GetKey(), p->GetDisplayPosition(), p->GetWorldPosition() );

  //if the pipeline is not initialized or click was on another renderwindow or slice then reset pipeline and preview
  if( (!m_Initialized) || (m_LastEventSender != m_PositionEvent->GetSender()) || (m_LastEventSlice != m_PositionEvent->GetSender()->GetSlice()) )
  {
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

    InternalImageType::Pointer referenceImageSliceAsITK = InternalImageType::New();
    m_ReferenceImageSlice = SegTool2D::GetAffectedReferenceSlice( m_PositionEvent );
    CastToItkImage(m_ReferenceImageSlice, referenceImageSliceAsITK);
    m_SmoothFilter->SetInput( referenceImageSliceAsITK );

    m_GradientMagnitudeFilter->SetInput( m_SmoothFilter->GetOutput() );
    m_SigmoidFilter->SetInput( m_GradientMagnitudeFilter->GetOutput() );
    m_FastMarchingFilter->SetInput( m_SigmoidFilter->GetOutput() );
    m_ThresholdFilter->SetInput( m_FastMarchingFilter->GetOutput() );

    this->ClearSeeds();

    m_Initialized = true;
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

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->End().Index(), m_PositionEvent->GetWorldPosition());

  m_NeedUpdate = true;

  this->Update();

  m_ReadyMessage.Send();

  return true;
}

bool mitk::FastMarchingTool::OnDelete(Action* action, const StateEvent* stateEvent)
{
  // delete last seed point
  if( this->m_SeedContainer->empty() ) return false;

  // get the timestep to support 3D+t
  const mitk::Event *theEvent = stateEvent->GetEvent();
  mitk::ScalarType timeInMS = 0.0;

  //check if the current timestep has to be changed
  if ( theEvent && theEvent->GetSender())
  {
    //additionaly to m_TimeStep we need timeInMS to satisfy the execution of the operations
    timeInMS = theEvent->GetSender()->GetTime();
  }

  //search the point in the list
  int position = m_SeedsAsPointSet->GetSize() - 1;

  PointSet::PointType pt = m_SeedsAsPointSet->GetPoint(position, m_TimeStep);

  PointOperation* doOp = new mitk::PointOperation(OpREMOVE, timeInMS, pt, position);
  //execute the Operation
  m_SeedsAsPointSet->ExecuteOperation(doOp);
  delete doOp;

  //delete last element of seeds container
  this->m_SeedContainer->pop_back();

  m_FastMarchingFilter->Modified();

  if(this->m_FastMarchingFilter.IsNotNull())
    m_FastMarchingFilter->Modified();

  m_NeedUpdate = true;

  this->Update();

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
    catch( itk::ExceptionObject& e )
    {
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

    m_FeedbackImage->SetGeometry( m_ReferenceImageSlice->GetGeometry(0)->Clone().GetPointer() );

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
