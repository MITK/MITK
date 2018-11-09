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

#include "mitkBaseRenderer.h"
#include "mitkInteractionConst.h"
#include "mitkRenderingManager.h"

#include "itkOrImageFilter.h"
#include "mitkImageTimeSelector.h"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, FastMarchingTool, "FastMarching2D tool");
}

mitk::FastMarchingTool::FastMarchingTool()
  : FeedbackContourTool("FastMarchingTool"),
    m_NeedUpdate(true),
    m_CurrentTimeStep(0),
    m_PositionEvent(nullptr),
    m_LowerThreshold(0),
    m_UpperThreshold(200),
    m_StoppingValue(100),
    m_Sigma(1.0),
    m_Alpha(-0.5),
    m_Beta(3.0)
{
}

mitk::FastMarchingTool::~FastMarchingTool()
{
  if (this->m_SmoothFilter.IsNotNull())
    this->m_SmoothFilter->RemoveAllObservers();

  if (this->m_SigmoidFilter.IsNotNull())
    this->m_SigmoidFilter->RemoveAllObservers();

  if (this->m_GradientMagnitudeFilter.IsNotNull())
    this->m_GradientMagnitudeFilter->RemoveAllObservers();

  if (this->m_FastMarchingFilter.IsNotNull())
    this->m_FastMarchingFilter->RemoveAllObservers();
}

void mitk::FastMarchingTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("ShiftSecondaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION("ShiftPrimaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION("DeletePoint", OnDelete);
}

const char **mitk::FastMarchingTool::GetXPM() const
{
  return nullptr; // mitkFastMarchingTool_xpm;
}

us::ModuleResource mitk::FastMarchingTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("FastMarching_48x48.png");
  return resource;
}

us::ModuleResource mitk::FastMarchingTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("FastMarching_Cursor_32x32.png");
  return resource;
}

const char *mitk::FastMarchingTool::GetName() const
{
  return "2D Fast Marching";
}

void mitk::FastMarchingTool::BuildITKPipeline()
{
  m_ReferenceImageSliceAsITK = InternalImageType::New();

  m_ReferenceImageSlice = GetAffectedReferenceSlice(m_PositionEvent);
  CastToItkImage(m_ReferenceImageSlice, m_ReferenceImageSliceAsITK);

  m_ProgressCommand = mitk::ToolCommand::New();

  m_SmoothFilter = SmoothingFilterType::New();
  m_SmoothFilter->SetInput(m_ReferenceImageSliceAsITK);
  m_SmoothFilter->SetTimeStep(0.05);
  m_SmoothFilter->SetNumberOfIterations(2);
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
  m_ThresholdFilter->SetInsideValue(1.0);

  m_SeedContainer = NodeContainer::New();
  m_SeedContainer->Initialize();
  m_FastMarchingFilter->SetTrialPoints(m_SeedContainer);

  if (this->m_SmoothFilter.IsNotNull())
    this->m_SmoothFilter->RemoveAllObservers();

  if (this->m_SigmoidFilter.IsNotNull())
    this->m_SigmoidFilter->RemoveAllObservers();

  if (this->m_GradientMagnitudeFilter.IsNotNull())
    this->m_GradientMagnitudeFilter->RemoveAllObservers();

  if (this->m_FastMarchingFilter.IsNotNull())
    this->m_FastMarchingFilter->RemoveAllObservers();

  m_SmoothFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  m_GradientMagnitudeFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  m_SigmoidFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  m_FastMarchingFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);

  m_SmoothFilter->SetInput(m_ReferenceImageSliceAsITK);
  m_GradientMagnitudeFilter->SetInput(m_SmoothFilter->GetOutput());
  m_SigmoidFilter->SetInput(m_GradientMagnitudeFilter->GetOutput());
  m_FastMarchingFilter->SetInput(m_SigmoidFilter->GetOutput());
  m_ThresholdFilter->SetInput(m_FastMarchingFilter->GetOutput());
  m_ReferenceImageSliceAsITK = InternalImageType::New();
}

void mitk::FastMarchingTool::SetUpperThreshold(double value)
{
  if (m_UpperThreshold != value)
  {
    m_UpperThreshold = value / 10.0;
    m_ThresholdFilter->SetUpperThreshold(m_UpperThreshold);
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetLowerThreshold(double value)
{
  if (m_LowerThreshold != value)
  {
    m_LowerThreshold = value / 10.0;
    m_ThresholdFilter->SetLowerThreshold(m_LowerThreshold);
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetBeta(double value)
{
  if (m_Beta != value)
  {
    m_Beta = value;
    m_SigmoidFilter->SetBeta(m_Beta);
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetSigma(double value)
{
  if (m_Sigma != value)
  {
    m_Sigma = value;
    m_GradientMagnitudeFilter->SetSigma(m_Sigma);
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetAlpha(double value)
{
  if (m_Alpha != value)
  {
    m_Alpha = value;
    m_SigmoidFilter->SetAlpha(m_Alpha);
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::SetStoppingValue(double value)
{
  if (m_StoppingValue != value)
  {
    m_StoppingValue = value;
    m_FastMarchingFilter->SetStoppingValue(m_StoppingValue);
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool::Activated()
{
  Superclass::Activated();

  m_ResultImageNode = mitk::DataNode::New();
  m_ResultImageNode->SetName("FastMarching_Preview");
  m_ResultImageNode->SetBoolProperty("helper object", true);
  m_ResultImageNode->SetColor(0.0, 1.0, 0.0);
  m_ResultImageNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add(this->m_ResultImageNode, m_ToolManager->GetReferenceData(0));

  m_SeedsAsPointSet = mitk::PointSet::New();
  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName("Seeds_Preview");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);
  m_ToolManager->GetDataStorage()->Add(this->m_SeedsAsPointSetNode, m_ToolManager->GetReferenceData(0));

  this->Initialize();
}

void mitk::FastMarchingTool::Deactivated()
{
  m_ToolManager->GetDataStorage()->Remove(this->m_ResultImageNode);
  m_ToolManager->GetDataStorage()->Remove(this->m_SeedsAsPointSetNode);
  this->ClearSeeds();
  m_ResultImageNode = nullptr;
  m_SeedsAsPointSetNode = nullptr;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  Superclass::Deactivated();
}

void mitk::FastMarchingTool::Initialize()
{
  m_ReferenceImage = dynamic_cast<mitk::Image *>(m_ToolManager->GetReferenceData(0)->GetData());
  if (m_ReferenceImage->GetTimeGeometry()->CountTimeSteps() > 1)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = ImageTimeSelector::New();
    timeSelector->SetInput(m_ReferenceImage);
    timeSelector->SetTimeNr(m_CurrentTimeStep);
    timeSelector->UpdateLargestPossibleRegion();
    m_ReferenceImage = timeSelector->GetOutput();
  }
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool::ConfirmSegmentation()
{
  // combine preview image with current working segmentation
  if (dynamic_cast<mitk::Image *>(m_ResultImageNode->GetData()))
  {
    // logical or combination of preview and segmentation slice

    mitk::Image::Pointer workingImageSlice;
    mitk::Image::Pointer workingImage = dynamic_cast<mitk::Image *>(this->m_ToolManager->GetWorkingData(0)->GetData());
    workingImageSlice = GetAffectedImageSliceAs2DImage(m_WorkingPlane, workingImage, m_CurrentTimeStep);

    mitk::Image::Pointer segmentationResult = mitk::Image::New();

    bool isDeprecatedUnsignedCharSegmentation =
      (workingImage->GetPixelType().GetComponentType() == itk::ImageIOBase::UCHAR);

    if (isDeprecatedUnsignedCharSegmentation)
    {
      typedef itk::Image<unsigned char, 2> OutputUCharImageType;
      OutputUCharImageType::Pointer workingImageSliceInITK = OutputUCharImageType::New();

      CastToItkImage(workingImageSlice, workingImageSliceInITK);

      typedef itk::OrImageFilter<OutputImageType, OutputUCharImageType, OutputUCharImageType> OrImageFilterType;
      OrImageFilterType::Pointer orFilter = OrImageFilterType::New();

      orFilter->SetInput1(m_ThresholdFilter->GetOutput());
      orFilter->SetInput2(workingImageSliceInITK);
      orFilter->Update();

      mitk::CastToMitkImage(orFilter->GetOutput(), segmentationResult);
    }
    else
    {
      OutputImageType::Pointer workingImageSliceInITK = OutputImageType::New();

      CastToItkImage(workingImageSlice, workingImageSliceInITK);

      typedef itk::OrImageFilter<OutputImageType, OutputImageType> OrImageFilterType;
      OrImageFilterType::Pointer orFilter = OrImageFilterType::New();

      orFilter->SetInput(0, m_ThresholdFilter->GetOutput());
      orFilter->SetInput(1, workingImageSliceInITK);
      orFilter->Update();

      mitk::CastToMitkImage(orFilter->GetOutput(), segmentationResult);
    }

    segmentationResult->GetGeometry()->SetOrigin(workingImageSlice->GetGeometry()->GetOrigin());
    segmentationResult->GetGeometry()->SetIndexToWorldTransform(
      workingImageSlice->GetGeometry()->GetIndexToWorldTransform());

    // write to segmentation volume and hide preview image
    // again, current time step is not considered
    this->WriteBackSegmentationResult(m_WorkingPlane, segmentationResult, m_CurrentTimeStep);
    this->m_ResultImageNode->SetVisibility(false);

    this->ClearSeeds();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);
}

void mitk::FastMarchingTool::OnAddPoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  // Add a new seed point for FastMarching algorithm
  auto *positionEvent = dynamic_cast<mitk::InteractionPositionEvent *>(interactionEvent);
  // const PositionEvent* p = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (positionEvent == nullptr)
    return;

  if (m_PositionEvent.IsNotNull())
    m_PositionEvent = nullptr;

  m_PositionEvent =
    InteractionPositionEvent::New(positionEvent->GetSender(), positionEvent->GetPointerPositionOnScreen());

  // if click was on another renderwindow or slice then reset pipeline and preview
  if ((m_LastEventSender != m_PositionEvent->GetSender()) ||
      (m_LastEventSlice != m_PositionEvent->GetSender()->GetSlice()))
  {
    this->BuildITKPipeline();
    this->ClearSeeds();
  }

  m_LastEventSender = m_PositionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();
  m_WorkingPlane = positionEvent->GetSender()->GetCurrentWorldPlaneGeometry()->Clone();

  mitk::Point3D clickInIndex;

  m_ReferenceImageSlice->GetGeometry()->WorldToIndex(m_PositionEvent->GetPositionInWorld(), clickInIndex);
  itk::Index<2> seedPosition;
  seedPosition[0] = clickInIndex[0];
  seedPosition[1] = clickInIndex[1];

  NodeType node;
  const double seedValue = 0.0;
  node.SetValue(seedValue);
  node.SetIndex(seedPosition);
  this->m_SeedContainer->InsertElement(this->m_SeedContainer->Size(), node);
  m_FastMarchingFilter->Modified();

  m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize(), m_PositionEvent->GetPositionInWorld());

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  m_NeedUpdate = true;

  this->Update();

  m_ReadyMessage.Send();
}

void mitk::FastMarchingTool::OnDelete(StateMachineAction *, InteractionEvent *)
{
  // delete last seed point
  if (!(this->m_SeedContainer->empty()))
  {
    // delete last element of seeds container
    this->m_SeedContainer->pop_back();
    m_FastMarchingFilter->Modified();

    // delete last point in pointset - somehow ugly
    m_SeedsAsPointSet->GetPointSet()->GetPoints()->DeleteIndex(m_SeedsAsPointSet->GetSize() - 1);

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    m_NeedUpdate = true;

    this->Update();
  }
}

void mitk::FastMarchingTool::Update()
{
  const unsigned int progress_steps = 20;

  // update FastMarching pipeline and show result
  if (m_NeedUpdate)
  {
    m_ProgressCommand->AddStepsToDo(progress_steps);
    CurrentlyBusy.Send(true);
    try
    {
      m_ThresholdFilter->Update();
    }
    catch (itk::ExceptionObject &excep)
    {
      MITK_ERROR << "Exception caught: " << excep.GetDescription();

      // progress by max step count, will force
      m_ProgressCommand->SetProgress(progress_steps);
      CurrentlyBusy.Send(false);

      std::string msg = excep.GetDescription();
      ErrorMessage.Send(msg);

      return;
    }
    m_ProgressCommand->SetProgress(progress_steps);
    CurrentlyBusy.Send(false);

    // make output visible
    mitk::Image::Pointer result = mitk::Image::New();
    CastToMitkImage(m_ThresholdFilter->GetOutput(), result);
    result->GetGeometry()->SetOrigin(m_ReferenceImageSlice->GetGeometry()->GetOrigin());
    result->GetGeometry()->SetIndexToWorldTransform(m_ReferenceImageSlice->GetGeometry()->GetIndexToWorldTransform());
    m_ResultImageNode->SetData(result);
    m_ResultImageNode->SetVisibility(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void mitk::FastMarchingTool::ClearSeeds()
{
  // clear seeds for FastMarching as well as the PointSet for visualization
  if (this->m_SeedContainer.IsNotNull())
    this->m_SeedContainer->Initialize();

  if (this->m_SeedsAsPointSet.IsNotNull())
  {
    this->m_SeedsAsPointSet = mitk::PointSet::New();
    this->m_SeedsAsPointSetNode->SetData(this->m_SeedsAsPointSet);
    m_SeedsAsPointSetNode->SetName("Seeds_Preview");
    m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
    m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
    m_SeedsAsPointSetNode->SetVisibility(true);
  }

  if (this->m_FastMarchingFilter.IsNotNull())
    m_FastMarchingFilter->Modified();

  this->m_NeedUpdate = true;
}

void mitk::FastMarchingTool::Reset()
{
  // clear all seeds and preview empty result
  this->ClearSeeds();

  m_ResultImageNode->SetVisibility(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::FastMarchingTool::SetCurrentTimeStep(int t)
{
  if (m_CurrentTimeStep != t)
  {
    m_CurrentTimeStep = t;

    this->Initialize();
  }
}
