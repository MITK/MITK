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
#include "mitkLabelSetImage.h"
#include "mitkRenderingManager.h"
#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkBaseRenderer.h"
#include "mitkStateMachineAction.h"
#include "mitkInteractionEvent.h"

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
m_LowerThreshold(-100),
m_UpperThreshold(2000),
m_StoppingValue(2000),
m_Sigma(1.2),
m_Alpha(-1.9),
m_Beta(2.8),
m_Initialized(false)
{
}

mitk::FastMarchingTool::~FastMarchingTool()
{
}

void mitk::FastMarchingTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION( "ShiftSecondaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION( "ShiftPrimaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION( "DeletePoint", OnDelete);
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
  Superclass::Activated();

  // feedback node and its visualization properties
  m_PreviewNode = mitk::DataNode::New();
  m_PreviewNode->SetName("tool preview");

  m_PreviewNode->SetProperty("texture interpolation", BoolProperty::New(false) );
  m_PreviewNode->SetProperty("layer", IntProperty::New(100) );
  m_PreviewNode->SetProperty("binary", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("outline binary", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("outline binary shadow", BoolProperty::New(true) );
  m_PreviewNode->SetProperty("helper object", BoolProperty::New(true) );
  m_PreviewNode->SetOpacity(1.0);
  m_PreviewNode->SetColor(0.0, 1.0, 0.0);

  m_ToolManager->GetDataStorage()->Add( m_PreviewNode, m_WorkingNode );

  m_SeedsAsPointSet = mitk::PointSet::New();
  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName("seeds");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetBoolProperty("updateDataOnRender", true);
  m_SeedsAsPointSetNode->SetColor(1.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);

  m_ToolManager->GetDataStorage()->Add( m_SeedsAsPointSetNode, m_WorkingNode );

  SigmaValueChanged.Send(m_Sigma);
  AlphaValueChanged.Send(m_Alpha);
  BetaValueChanged.Send(m_Beta);
  StopValueChanged.Send(m_StoppingValue);
}

void mitk::FastMarchingTool::Deactivated()
{
  Superclass::Deactivated();

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

void mitk::FastMarchingTool::CreateNewLabel(const std::string& name, const mitk::Color& color)
{
  if ( m_PreviewImage.IsNull() ) return;

  mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(m_WorkingNode->GetData());
  assert(workingImage);

  workingImage->AddLabel(name,color);

  this->AcceptPreview();
}

void mitk::FastMarchingTool::AcceptPreview()
{
  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  m_PaintingPixelValue = workingImage->GetActiveLabelIndex();

  CurrentlyBusy.Send(true);

  // paste the preview image to the current working slice
  mitk::SegTool2D::WritePreviewOnWorkingImage( m_WorkingImageSlice, m_PreviewImage, m_PaintingPixelValue, m_CurrentTimeStep );

  // paste current working slice back to the working image
  mitk::SegTool2D::WriteBackSegmentationResult(m_PositionEvent, m_WorkingImageSlice);

  this->ClearSeeds();

  workingImage->Modified();

  m_PreviewNode->SetData(NULL);
  m_PreviewImage = NULL;

  CurrentlyBusy.Send(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

bool mitk::FastMarchingTool::OnAddPoint( StateMachineAction*, InteractionEvent* interactionEvent )
{
  mitk::InteractionPositionEvent* positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>( interactionEvent );
  if (!positionEvent) return false;

  m_WorkingImageSlice = SegTool2D::GetAffectedWorkingSlice( positionEvent );

  m_PositionEvent = InteractionPositionEvent::New( positionEvent->GetSender(),
                                                   positionEvent->GetPointerPositionOnScreen() );

  //if the pipeline is not initialized or click was on another render window or slice then reset pipeline and preview image
  if( (!m_Initialized) || (m_LastEventSender != positionEvent->GetSender()) || (m_LastEventSlice != positionEvent->GetSender()->GetSlice()) )
  {
    m_SmoothFilter = SmoothingFilterType::New();
    m_SmoothFilter->SetTimeStep(0.014);
    m_SmoothFilter->SetNumberOfIterations(3);
    m_SmoothFilter->SetConductanceParameter(6.0);

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

    m_SeedsAsPointSet = mitk::PointSet::New(); // m_SeedsAsPointSet->Clear() does not work
    m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);

    InternalImageType::Pointer referenceImageSliceAsITK = InternalImageType::New();
    m_ReferenceImageSlice = SegTool2D::GetAffectedReferenceSlice( positionEvent );
    CastToItkImage(m_ReferenceImageSlice, referenceImageSliceAsITK);
    m_SmoothFilter->SetInput( referenceImageSliceAsITK );

    m_GradientMagnitudeFilter->SetInput( m_SmoothFilter->GetOutput() );
    m_SigmoidFilter->SetInput( m_GradientMagnitudeFilter->GetOutput() );
    m_FastMarchingFilter->SetInput( m_SigmoidFilter->GetOutput() );
    m_ThresholdFilter->SetInput( m_FastMarchingFilter->GetOutput() );

    m_Initialized = true;
  }

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  itk::Index<3> lastSeedIndex;

  m_ReferenceImageSlice->GetGeometry()->WorldToIndex(positionEvent->GetPositionInWorld(), lastSeedIndex);
  itk::Index<2> seedPosition;
  seedPosition[0] = lastSeedIndex[0];
  seedPosition[1] = lastSeedIndex[1];

  NodeType node;
  const double seedValue = 0.0;
  node.SetValue( seedValue );
  node.SetIndex( seedPosition );
  m_SeedContainer->InsertElement(m_SeedContainer->Size(), node);
  m_FastMarchingFilter->Modified();

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize()-1, positionEvent->GetPositionInWorld());

  m_NeedUpdate = true;

  this->Run();

  return true;
}

bool mitk::FastMarchingTool::OnDelete(StateMachineAction*, InteractionEvent* interactionEvent)
{
  // delete last seed point
  if( m_SeedContainer->empty() ) return false;

  mitk::ScalarType timeInMS = interactionEvent->GetSender()->GetTime();

  //search the point in the list
  int position = m_SeedsAsPointSet->GetSize() - 1;

  PointSet::PointType pt = m_SeedsAsPointSet->GetPoint(position, m_CurrentTimeStep);

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
    mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
    assert(workingImage);

    m_CurrentTimeStep = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetTime()->GetPos();

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

  mitk::LabelSetImage* workingImage = dynamic_cast< mitk::LabelSetImage* >( m_WorkingNode->GetData() );
  assert(workingImage);

  // fix intersections with other labels
  typedef itk::ImageRegionConstIterator< ImageType >    InputIteratorType;
  typedef itk::ImageRegionIterator< OutputImageType >   ResultIteratorType;

  InputIteratorType  inputIter( input, input->GetLargestPossibleRegion() );
  ResultIteratorType resultIter( result, result->GetLargestPossibleRegion() );

  inputIter.GoToBegin();
  resultIter.GoToBegin();

  while ( !resultIter.IsAtEnd() )
  {
    int inputValue = static_cast<int>( inputIter.Get() );

    if ( (inputValue != m_PaintingPixelValue) && workingImage->GetLabelLocked( inputValue ) )
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
