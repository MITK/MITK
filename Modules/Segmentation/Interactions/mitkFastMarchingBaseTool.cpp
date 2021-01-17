/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkFastMarchingBaseTool.h"
#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkInteractionConst.h"
#include "mitkRenderingManager.h"
#include "mitkInteractionPositionEvent.h"

#include "mitkImageAccessByItk.h"

#include "mitkSegTool2D.h"

#include <mitkImageCast.h>

// itk filter
#include "itkBinaryThresholdImageFilter.h"
#include "itkCurvatureAnisotropicDiffusionImageFilter.h"
#include "itkGradientMagnitudeRecursiveGaussianImageFilter.h"
#include "itkSigmoidImageFilter.h"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>


mitk::FastMarchingBaseTool::FastMarchingBaseTool(unsigned int toolDim)
  : AutoSegmentationWithPreviewTool(false, "FastMarchingTool"),
    m_LowerThreshold(0),
    m_UpperThreshold(200),
    m_StoppingValue(100),
    m_Sigma(1.0),
    m_Alpha(-0.5),
    m_Beta(3.0),
    m_ToolDimension(toolDim)
{
}

mitk::FastMarchingBaseTool::~FastMarchingBaseTool()
{
}

bool mitk::FastMarchingBaseTool::CanHandle(const BaseData* referenceData, const BaseData* workingData) const
{
  if(!Superclass::CanHandle(referenceData, workingData))
    return false;

  if (referenceData == nullptr)
    return false;

  auto *image = dynamic_cast<const Image *>(referenceData);

  if (image == nullptr)
    return false;

  if (image->GetDimension() < 3)
    return false;

  return true;
}

const char **mitk::FastMarchingBaseTool::GetXPM() const
{
  return nullptr; // mitkFastMarchingBaseTool_xpm;
}

us::ModuleResource mitk::FastMarchingBaseTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("FastMarching_48x48.png");
  return resource;
}

us::ModuleResource mitk::FastMarchingBaseTool::GetCursorIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("FastMarching_Cursor_32x32.png");
  return resource;
}

void mitk::FastMarchingBaseTool::SetUpperThreshold(double value)
{
  m_UpperThreshold = value / 10.0;
}

void mitk::FastMarchingBaseTool::SetLowerThreshold(double value)
{
  m_LowerThreshold = value / 10.0;
}

void mitk::FastMarchingBaseTool::SetBeta(double value)
{
  if (m_Beta != value)
  {
    m_Beta = value;
  }
}

void mitk::FastMarchingBaseTool::SetSigma(double value)
{
  if (m_Sigma != value)
  {
    if (value > 0.0)
    {
      m_Sigma = value;
    }
  }
}

void mitk::FastMarchingBaseTool::SetAlpha(double value)
{
  if (m_Alpha != value)
  {
    m_Alpha = value;
  }
}

void mitk::FastMarchingBaseTool::SetStoppingValue(double value)
{
  if (m_StoppingValue != value)
  {
    m_StoppingValue = value;
  }
}

void mitk::FastMarchingBaseTool::Activated()
{
  Superclass::Activated();

  m_SeedsAsPointSet = mitk::PointSet::New();
  //ensure that the seed points are visible for all timepoints.
  dynamic_cast<ProportionalTimeGeometry*>(m_SeedsAsPointSet->GetTimeGeometry())->SetStepDuration(std::numeric_limits<TimePointType>::max());

  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName(std::string(this->GetName()) + "_PointSet");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);

  m_ToolManager->GetDataStorage()->Add(m_SeedsAsPointSetNode, m_ToolManager->GetWorkingData(0));
}

void mitk::FastMarchingBaseTool::Deactivated()
{
  this->ClearSeeds();

  m_ToolManager->GetDataStorage()->Remove(m_SeedsAsPointSetNode);
  m_SeedsAsPointSetNode = nullptr;
  m_SeedsAsPointSet = nullptr;

  Superclass::Deactivated();
}

void mitk::FastMarchingBaseTool::ConnectActionsAndFunctions()
{
  CONNECT_FUNCTION("ShiftSecondaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION("ShiftPrimaryButtonPressed", OnAddPoint);
  CONNECT_FUNCTION("DeletePoint", OnDelete);
}

void mitk::FastMarchingBaseTool::OnAddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if (!this->IsUpdating() && m_SeedsAsPointSet.IsNotNull())
  {
    const auto positionEvent = dynamic_cast<mitk::InteractionPositionEvent*>(interactionEvent);

    if (positionEvent != nullptr)
    {
      auto workingPlaneGeometry = positionEvent->GetSender()->GetCurrentWorldPlaneGeometry();

      // if click was on another plane and we are in 2D mode we should reset the seeds
      if (m_ToolDimension == 2 && ( nullptr == this->GetWorkingPlaneGeometry() || !this->GetWorkingPlaneGeometry()->IsOnPlane(workingPlaneGeometry)))
      {
        this->ClearSeeds();
        this->SetWorkingPlaneGeometry(workingPlaneGeometry->Clone());
      }

      m_SeedsAsPointSet->InsertPoint(m_SeedsAsPointSet->GetSize(), positionEvent->GetPositionInWorld());

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      this->UpdatePreview();
    }
  }
}

void mitk::FastMarchingBaseTool::OnDelete(StateMachineAction*, InteractionEvent* /*interactionEvent*/)
{
  if (!this->IsUpdating() && m_SeedsAsPointSet.IsNotNull())
  {
    // delete last seed point
    if (this->m_SeedsAsPointSet->GetSize() > 0)
    {
      m_SeedsAsPointSet->RemovePointAtEnd(0);

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      this->UpdatePreview();
    }
  }
}

void mitk::FastMarchingBaseTool::ClearSeeds()
{
  if (this->m_SeedsAsPointSet.IsNotNull())
  {
    // renew pointset
    this->m_SeedsAsPointSet = mitk::PointSet::New();
    //ensure that the seed points are visible for all timepoints.
    dynamic_cast<ProportionalTimeGeometry*>(m_SeedsAsPointSet->GetTimeGeometry())->SetStepDuration(std::numeric_limits<TimePointType>::max());
    this->m_SeedsAsPointSetNode->SetData(this->m_SeedsAsPointSet);
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::FastMarchingBaseTool::DoITKFastMarching(const itk::Image<TPixel, VImageDimension>* inputImage,
  Image* previewImage, unsigned int timeStep, const BaseGeometry* inputGeometry)
{
  // typedefs for itk pipeline
  typedef itk::Image<TPixel, VImageDimension> InputImageType;

  typedef float InternalPixelType;
  typedef itk::Image<InternalPixelType, VImageDimension> InternalImageType;

  typedef mitk::Tool::DefaultSegmentationDataType OutputPixelType;
  typedef itk::Image<OutputPixelType, VImageDimension> OutputImageType;

  typedef itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, InternalImageType> SmoothingFilterType;
  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<InternalImageType, InternalImageType> GradientFilterType;
  typedef itk::SigmoidImageFilter<InternalImageType, InternalImageType> SigmoidFilterType;
  typedef itk::BinaryThresholdImageFilter<InternalImageType, OutputImageType> ThresholdingFilterType;

  typedef itk::FastMarchingImageFilter<InternalImageType, InternalImageType> FastMarchingFilterType;
  typedef typename FastMarchingFilterType::NodeContainer NodeContainer;
  typedef typename FastMarchingFilterType::NodeType NodeType;

  //convert point set seed into trialpoint
  typename NodeContainer::Pointer trialPoints = NodeContainer::New();
  trialPoints->Initialize();

  for (auto pos = m_SeedsAsPointSet->Begin(); pos != m_SeedsAsPointSet->End(); ++pos)
  {
    mitk::Point3D clickInIndex;

    inputGeometry->WorldToIndex(pos->Value(), clickInIndex);
    itk::Index<VImageDimension> seedPosition;
    for (unsigned int dim = 0; dim < VImageDimension; ++dim)
    {
      seedPosition[dim] = clickInIndex[dim];
    }

    NodeType node;
    const double seedValue = 0.0;
    node.SetValue(seedValue);
    node.SetIndex(seedPosition);
    trialPoints->InsertElement(trialPoints->Size(), node);
  }

  // assemble pipeline
  auto smoothFilter = SmoothingFilterType::New();
  smoothFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  smoothFilter->SetTimeStep(0.05);
  smoothFilter->SetNumberOfIterations(2);
  smoothFilter->SetConductanceParameter(9.0);

  auto gradientMagnitudeFilter = GradientFilterType::New();
  gradientMagnitudeFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  gradientMagnitudeFilter->SetSigma(m_Sigma);

  auto sigmoidFilter = SigmoidFilterType::New();
  sigmoidFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  sigmoidFilter->SetAlpha(m_Alpha);
  sigmoidFilter->SetBeta(m_Beta);
  sigmoidFilter->SetOutputMinimum(0.0);
  sigmoidFilter->SetOutputMaximum(1.0);

  auto fastMarchingFilter = FastMarchingFilterType::New();
  fastMarchingFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  fastMarchingFilter->SetStoppingValue(m_StoppingValue);
  fastMarchingFilter->SetTrialPoints(trialPoints);

  auto thresholdFilter = ThresholdingFilterType::New();
  thresholdFilter->SetLowerThreshold(m_LowerThreshold);
  thresholdFilter->SetUpperThreshold(m_UpperThreshold);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->SetInsideValue(1.0);

  // set up pipeline
  smoothFilter->SetInput(inputImage);
  gradientMagnitudeFilter->SetInput(smoothFilter->GetOutput());
  sigmoidFilter->SetInput(gradientMagnitudeFilter->GetOutput());
  fastMarchingFilter->SetInput(sigmoidFilter->GetOutput());
  thresholdFilter->SetInput(fastMarchingFilter->GetOutput());
  thresholdFilter->Update();

  if (nullptr == this->GetWorkingPlaneGeometry())
  {
    previewImage->SetVolume((void*)(thresholdFilter->GetOutput()->GetPixelContainer()->GetBufferPointer()), timeStep);
  }
  else
  {
    mitk::Image::Pointer sliceImage = mitk::Image::New();
    mitk::CastToMitkImage(thresholdFilter->GetOutput(), sliceImage);
    SegTool2D::WriteSliceToVolume(previewImage, this->GetWorkingPlaneGeometry(), sliceImage, timeStep, false);
  }
}

void mitk::FastMarchingBaseTool::DoUpdatePreview(const Image* inputAtTimeStep, Image* previewImage, TimeStepType timeStep)
{
  if (nullptr != inputAtTimeStep && nullptr != previewImage && m_SeedsAsPointSet.IsNotNull() && m_SeedsAsPointSet->GetSize()>0)
  {
    if (nullptr == this->GetWorkingPlaneGeometry())
    {
      AccessFixedDimensionByItk_n(inputAtTimeStep, DoITKFastMarching, 3, (previewImage, timeStep, inputAtTimeStep->GetGeometry()));
    }
    else
    {
      AccessFixedDimensionByItk_n(inputAtTimeStep, DoITKFastMarching, 2, (previewImage, timeStep, inputAtTimeStep->GetGeometry()));
    }
  }
}
