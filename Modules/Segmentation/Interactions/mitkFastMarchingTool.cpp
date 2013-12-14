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
#include "mitkImageAccessByItk.h"
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
m_LowerThreshold(-100),
m_UpperThreshold(2000),
m_StoppingValue(2000),
m_Sigma(1.2),
m_Alpha(-1.9),
m_Beta(2.8),
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
  if (m_UpperThreshold != value)
  {
    m_UpperThreshold = value;
    if (m_Initialized)
    {
      m_ThresholdFilter->SetUpperThreshold( m_UpperThreshold );
      m_NeedUpdate = true;
    }
  }
}

double mitk::FastMarchingTool::GetUpperThreshold()
{
  return m_UpperThreshold;
}

void mitk::FastMarchingTool::SetLowerThreshold(double value)
{
  if (m_LowerThreshold != value)
  {
    m_LowerThreshold = value;
    if (m_Initialized)
    {
      m_ThresholdFilter->SetLowerThreshold( m_LowerThreshold );
      m_NeedUpdate = true;
    }
  }
}

double mitk::FastMarchingTool::GetLowerThreshold()
{
  return m_LowerThreshold;
}

void mitk::FastMarchingTool::SetBeta(double value)
{
  if (m_Beta != value)
  {
    m_Beta = value;
    if (m_Initialized)
    {
      m_SigmoidFilter->SetBeta( m_Beta );
      m_NeedUpdate = true;
    }
  }
}

double mitk::FastMarchingTool::GetBeta()
{
  return m_Beta;
}

void mitk::FastMarchingTool::SetSigma(double value)
{
  if (m_Sigma != value)
  {
    m_Sigma = value;
    if (m_Initialized)
    {
      m_GradientMagnitudeFilter->SetSigma( m_Sigma );
      m_NeedUpdate = true;
    }
  }
}

double mitk::FastMarchingTool::GetSigma()
{
  return m_Sigma;
}

void mitk::FastMarchingTool::SetAlpha(double value)
{
  if (m_Alpha != value)
  {
    m_Alpha = value;
    if (m_Initialized)
    {
      m_SigmoidFilter->SetAlpha( m_Alpha );
      m_NeedUpdate = true;
    }
  }
}

double mitk::FastMarchingTool::GetAlpha()
{
  return m_Alpha;
}

void mitk::FastMarchingTool::SetStoppingValue(double value)
{
  if (m_StoppingValue != value)
  {
    m_StoppingValue = value;
    if (m_Initialized)
    {
      m_FastMarchingFilter->SetStoppingValue( m_StoppingValue );
      m_NeedUpdate = true;
    }
  }
}

double mitk::FastMarchingTool::GetStoppingValue()
{
  return m_StoppingValue;
}

void mitk::FastMarchingTool::Activated()
{
  // feedback node and its visualization properties
  m_PreviewNode = mitk::DataNode::New();
  m_PreviewNode->SetName("preview");

  m_PreviewNode->SetProperty("texture interpolation", BoolProperty::New(false) );
  m_PreviewNode->SetProperty("layer", IntProperty::New(100) );
  m_PreviewNode->SetProperty("binary", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("outline binary", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("outline binary shadow", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("helper object", BoolProperty::New(true) );
  m_PreviewNode->SetOpacity(1.0);
  m_PreviewNode->SetColor(0.0, 1.0, 0.0);

  m_ToolManager->GetDataStorage()->Add( m_PreviewNode, m_ToolManager->GetWorkingData(0) );

  m_SeedsAsPointSet = mitk::PointSet::New();
  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName("seeds");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetBoolProperty("updateDataOnRender", true);
  m_SeedsAsPointSetNode->SetColor(1.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);

  m_ToolManager->GetDataStorage()->Add( m_SeedsAsPointSetNode, m_ToolManager->GetWorkingData(0) );
}

void mitk::FastMarchingTool::Deactivated()
{
  m_ToolManager->GetDataStorage()->Remove( m_PreviewNode );
  m_ToolManager->GetDataStorage()->Remove( m_SeedsAsPointSetNode );
  this->ClearSeeds();
  m_PreviewNode = NULL;
  m_PreviewImage = NULL;
  m_SeedsAsPointSetNode = NULL;
  m_LastEventSender = NULL;
  m_LastEventSlice = -1;
  m_Initialized = false;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::FastMarchingTool::AcceptPreview()
{
  mitk::Image::Pointer workingImageSlice = SegTool2D::GetAffectedWorkingSlice( m_PositionEvent );

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  m_OverwritePixelValue = workingImage->GetActiveLabelIndex();

  // paste the binary segmentation to the current working slice
  mitk::SegTool2D::PasteSegmentationOnWorkingImage( workingImageSlice, m_PreviewImage, m_OverwritePixelValue, m_CurrentTimeStep );

  // paste current working slice back to the working image
  mitk::SegTool2D::WriteBackSegmentationResult(m_PositionEvent, workingImageSlice);

  this->ClearSeeds();

  workingImage->Modified();

  m_PreviewNode->SetData(NULL);
  m_PreviewImage = NULL;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

bool mitk::FastMarchingTool::OnAddPoint(Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* p = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!p) return false;

  if (m_PositionEvent != NULL)
      delete m_PositionEvent;
  m_PositionEvent = new PositionEvent(p->GetSender(), p->GetType(), p->GetButton(), p->GetButtonState(), p->GetKey(), p->GetDisplayPosition(), p->GetWorldPosition() );

  //if the pipeline is not initialized or click was on another render window or slice then reset pipeline and preview image
  if( (!m_Initialized) || (m_LastEventSender != m_PositionEvent->GetSender()) || (m_LastEventSlice != m_PositionEvent->GetSender()->GetSlice()) )
  {
    m_SmoothFilter = SmoothingFilterType::New();
    m_SmoothFilter->SetTimeStep(0.025);
    m_SmoothFilter->SetNumberOfIterations(3);
    m_SmoothFilter->SetConductanceParameter(9.0);

    m_GradientMagnitudeFilter = GradientFilterType::New();
    m_GradientMagnitudeFilter->SetSigma(m_Sigma);

    m_SigmoidFilter = SigmoidFilterType::New();
    m_SigmoidFilter->SetAlpha(m_Alpha);
    m_SigmoidFilter->SetBeta(m_Beta);
    m_SigmoidFilter->SetOutputMinimum(0.0);
    m_SigmoidFilter->SetOutputMaximum(1.0);

    m_FastMarchingFilter = FastMarchingFilterType::New();
    m_FastMarchingFilter->SetStoppingValue(m_StoppingValue);

    m_ThresholdFilter = ThresholdingFilterType::New();
    m_ThresholdFilter->SetLowerThreshold(m_LowerThreshold);
    m_ThresholdFilter->SetUpperThreshold(m_UpperThreshold);
    m_ThresholdFilter->SetOutsideValue(0);
    m_ThresholdFilter->SetInsideValue(1);

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

//    this->ClearSeeds();

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
  m_SeedContainer->InsertElement(m_SeedContainer->Size(), node);
  m_FastMarchingFilter->Modified();

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize()-1, m_PositionEvent->GetWorldPosition());

  m_NeedUpdate = true;

  this->Run();

  m_ReadyMessage.Send();

  return true;
}

bool mitk::FastMarchingTool::OnDelete(Action* action, const StateEvent* stateEvent)
{
  // delete last seed point
  if( m_SeedContainer->empty() ) return false;

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
  m_SeedContainer->pop_back();

  m_FastMarchingFilter->Modified();

  if(m_FastMarchingFilter.IsNotNull())
    m_FastMarchingFilter->Modified();

  m_NeedUpdate = true;

  this->Run();

  return true;
}

void mitk::FastMarchingTool::Cancel()
{
  m_ToolManager->ActivateTool(-1);
}

void mitk::FastMarchingTool::Run()
{
  // update FastMarching pipeline and show result
  if (m_Initialized && m_NeedUpdate && m_SeedContainer->Size())
  {
    mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
    assert(workingNode);

    mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
    assert(workingImage);

    // todo: use it later
    //unsigned int timestep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

    CurrentlyBusy.Send(true);

    try
    {
      AccessByItk(workingImage, InternalRun);
    }
    catch( itk::ExceptionObject & e )
    {
      CurrentlyBusy.Send(false);
      m_ProgressCommand->Reset();
      MITK_ERROR << "Exception caught: " << e.GetDescription();
      m_ToolManager->ActivateTool(-1);
      return;
    }
    catch (...)
    {
      CurrentlyBusy.Send(false);
      m_ProgressCommand->Reset();
      MITK_ERROR << "Unkown exception caught!";
      m_ToolManager->ActivateTool(-1);
      return;
    }

    CurrentlyBusy.Send(false);

    m_NeedUpdate = false;

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

template < typename ImageType >
void mitk::FastMarchingTool::InternalRun( ImageType* input )
{
  m_ProgressCommand->AddStepsToDo(20);

  m_ThresholdFilter->Update();

  m_ProgressCommand->Reset();

  OutputImageType::Pointer result = m_ThresholdFilter->GetOutput();
  result->DisconnectPipeline();

  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  assert(workingNode);

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( workingNode->GetData() );
  assert(workingImage);

  // fix intersections with other labels
  typedef itk::ImageRegionConstIterator< ImageType >    InputIteratorType;
  typedef itk::ImageRegionIterator< OutputImageType >   ResultIteratorType;

  typename InputIteratorType  inputIter( input, input->GetLargestPossibleRegion() );
  typename ResultIteratorType resultIter( result, result->GetLargestPossibleRegion() );

  inputIter.GoToBegin();
  resultIter.GoToBegin();

  while ( !resultIter.IsAtEnd() )
  {
    int inputValue = static_cast<int>( inputIter.Get() );

    if ( (inputValue != m_OverwritePixelValue) && workingImage->GetLabelLocked( inputValue ) )
      resultIter.Set(0);

    ++inputIter;
    ++resultIter;
  }

  m_PreviewImage = mitk::Image::New();
  m_PreviewImage->InitializeByItk(result.GetPointer());
  m_PreviewImage->SetChannel(result->GetBufferPointer());
  m_PreviewImage->SetGeometry( m_ReferenceImageSlice->GetGeometry(0)->Clone().GetPointer() );

  m_PreviewNode->SetData(m_PreviewImage);
}

void mitk::FastMarchingTool::ClearSeeds()
{
  if (m_PreviewImage.IsNull()) return;

  // clear seeds for FastMarching as well as the PointSet for visualization
  if (m_SeedContainer.IsNotNull())
    m_SeedContainer->Initialize();

  m_SeedsAsPointSet = mitk::PointSet::New(); // m_SeedsAsPointSet->Clear() does not work
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);

  if(m_FastMarchingFilter.IsNotNull())
    m_FastMarchingFilter->Modified();

  m_PreviewNode->SetData(NULL);
  m_PreviewImage = NULL;

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
