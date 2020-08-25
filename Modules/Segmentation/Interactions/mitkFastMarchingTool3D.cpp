/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkFastMarchingTool3D.h"
#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkInteractionConst.h"
#include "mitkRenderingManager.h"

#include "mitkImageAccessByItk.h"

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

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, FastMarchingTool3D, "FastMarching3D tool");
}

mitk::FastMarchingTool3D::FastMarchingTool3D()
  : AutoSegmentationWithPreviewTool(),
    m_LowerThreshold(0),
    m_UpperThreshold(200),
    m_StoppingValue(100),
    m_Sigma(1.0),
    m_Alpha(-0.5),
    m_Beta(3.0),
    m_PointSetAddObserverTag(0),
    m_PointSetRemoveObserverTag(0)
{
}

mitk::FastMarchingTool3D::~FastMarchingTool3D()
{
}

bool mitk::FastMarchingTool3D::CanHandle(const BaseData* referenceData, const BaseData* workingData) const
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

const char **mitk::FastMarchingTool3D::GetXPM() const
{
  return nullptr; // mitkFastMarchingTool3D_xpm;
}

us::ModuleResource mitk::FastMarchingTool3D::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("FastMarching_48x48.png");
  return resource;
}

const char *mitk::FastMarchingTool3D::GetName() const
{
  return "Fast Marching 3D";
}

void mitk::FastMarchingTool3D::SetUpperThreshold(double value)
{
  m_UpperThreshold = value / 10.0;
}

void mitk::FastMarchingTool3D::SetLowerThreshold(double value)
{
  m_LowerThreshold = value / 10.0;
}

void mitk::FastMarchingTool3D::SetBeta(double value)
{
  if (m_Beta != value)
  {
    m_Beta = value;
  }
}

void mitk::FastMarchingTool3D::SetSigma(double value)
{
  if (m_Sigma != value)
  {
    if (value > 0.0)
    {
      m_Sigma = value;
    }
  }
}

void mitk::FastMarchingTool3D::SetAlpha(double value)
{
  if (m_Alpha != value)
  {
    m_Alpha = value;
  }
}

void mitk::FastMarchingTool3D::SetStoppingValue(double value)
{
  if (m_StoppingValue != value)
  {
    m_StoppingValue = value;
  }
}

void mitk::FastMarchingTool3D::Activated()
{
  Superclass::Activated();

  m_SeedsAsPointSet = mitk::PointSet::New();
  m_SeedsAsPointSetNode = mitk::DataNode::New();
  m_SeedsAsPointSetNode->SetData(m_SeedsAsPointSet);
  m_SeedsAsPointSetNode->SetName("3D_FastMarching_PointSet");
  m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
  m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
  m_SeedsAsPointSetNode->SetVisibility(true);

  // Create PointSetData Interactor
  m_SeedPointInteractor = mitk::PointSetDataInteractor::New();
  // Load the according state machine for regular point set interaction
  m_SeedPointInteractor->LoadStateMachine("PointSet.xml");
  // Set the configuration file that defines the triggers for the transitions
  m_SeedPointInteractor->SetEventConfig("PointSetConfig.xml");
  // set the DataNode (which already is added to the DataStorage
  m_SeedPointInteractor->SetDataNode(m_SeedsAsPointSetNode);

  m_ToolManager->GetDataStorage()->Add(m_SeedsAsPointSetNode, m_ToolManager->GetWorkingData(0));

  m_SeedContainer = NodeContainer::New();
  m_SeedContainer->Initialize();

  itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::Pointer pointAddedCommand =
    itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::New();
  pointAddedCommand->SetCallbackFunction(this, &mitk::FastMarchingTool3D::OnAddPoint);
  m_PointSetAddObserverTag = m_SeedsAsPointSet->AddObserver(mitk::PointSetAddEvent(), pointAddedCommand);

  itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::Pointer pointRemovedCommand =
    itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::New();
  pointRemovedCommand->SetCallbackFunction(this, &mitk::FastMarchingTool3D::OnDelete);
  m_PointSetRemoveObserverTag = m_SeedsAsPointSet->AddObserver(mitk::PointSetRemoveEvent(), pointRemovedCommand);
}

void mitk::FastMarchingTool3D::Deactivated()
{
  this->ClearSeeds();

  // Deactivate Interaction
  m_SeedPointInteractor->SetDataNode(nullptr);
  m_ToolManager->GetDataStorage()->Remove(m_SeedsAsPointSetNode);
  m_SeedsAsPointSetNode = nullptr;
  m_SeedsAsPointSet->RemoveObserver(m_PointSetAddObserverTag);
  m_SeedsAsPointSet->RemoveObserver(m_PointSetRemoveObserverTag);
  m_SeedsAsPointSet = nullptr;

  Superclass::Deactivated();
}

void mitk::FastMarchingTool3D::OnAddPoint()
{
  // Add a new seed point for FastMarching algorithm
  mitk::Point3D clickInIndex;

  this->GetReferenceData()->GetGeometry()->WorldToIndex(m_SeedsAsPointSet->GetPoint(m_SeedsAsPointSet->GetSize() - 1),
                                                clickInIndex);
  itk::Index<3> seedPosition;
  seedPosition[0] = clickInIndex[0];
  seedPosition[1] = clickInIndex[1];
  seedPosition[2] = clickInIndex[2];

  NodeType node;
  const double seedValue = 0.0;
  node.SetValue(seedValue);
  node.SetIndex(seedPosition);
  this->m_SeedContainer->InsertElement(this->m_SeedContainer->Size(), node);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  this->UpdatePreview();
}

void mitk::FastMarchingTool3D::OnDelete()
{
  // delete last seed point
  if (!(this->m_SeedContainer->empty()))
  {
    // delete last element of seeds container
    this->m_SeedContainer->pop_back();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    this->UpdatePreview();
  }
}

void mitk::FastMarchingTool3D::ClearSeeds()
{
  // clear seeds for FastMarching as well as the PointSet for visualization
  if (this->m_SeedContainer.IsNotNull())
    this->m_SeedContainer->Initialize();

  if (this->m_SeedsAsPointSet.IsNotNull())
  {
    // remove observers from current pointset
    m_SeedsAsPointSet->RemoveObserver(m_PointSetAddObserverTag);
    m_SeedsAsPointSet->RemoveObserver(m_PointSetRemoveObserverTag);

    // renew pointset
    this->m_SeedsAsPointSet = mitk::PointSet::New();
    this->m_SeedsAsPointSetNode->SetData(this->m_SeedsAsPointSet);
    m_SeedsAsPointSetNode->SetName("Seeds_Preview");
    m_SeedsAsPointSetNode->SetBoolProperty("helper object", true);
    m_SeedsAsPointSetNode->SetColor(0.0, 1.0, 0.0);
    m_SeedsAsPointSetNode->SetVisibility(true);

    // add callback function for adding and removing points
    itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::Pointer pointAddedCommand =
      itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::New();
    pointAddedCommand->SetCallbackFunction(this, &mitk::FastMarchingTool3D::OnAddPoint);
    m_PointSetAddObserverTag = m_SeedsAsPointSet->AddObserver(mitk::PointSetAddEvent(), pointAddedCommand);

    itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::Pointer pointRemovedCommand =
      itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::New();
    pointRemovedCommand->SetCallbackFunction(this, &mitk::FastMarchingTool3D::OnDelete);
    m_PointSetRemoveObserverTag = m_SeedsAsPointSet->AddObserver(mitk::PointSetRemoveEvent(), pointRemovedCommand);
  }
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::FastMarchingTool3D::DoITKFastMarching(const itk::Image<TPixel, VImageDimension>* inputImage,
  mitk::Image* segmentation, unsigned int timeStep)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;

  /* typedefs for itk pipeline */

  typedef mitk::Tool::DefaultSegmentationDataType OutputPixelType;
  typedef itk::Image<OutputPixelType, VImageDimension> OutputImageType;

  typedef itk::CurvatureAnisotropicDiffusionImageFilter<InputImageType, InternalImageType> SmoothingFilterType;
  typedef itk::GradientMagnitudeRecursiveGaussianImageFilter<InternalImageType, InternalImageType> GradientFilterType;
  typedef itk::SigmoidImageFilter<InternalImageType, InternalImageType> SigmoidFilterType;
  typedef itk::BinaryThresholdImageFilter<InternalImageType, OutputImageType> ThresholdingFilterType;

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
  fastMarchingFilter->SetTrialPoints(m_SeedContainer);

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

  segmentation->SetVolume((void*)(thresholdFilter->GetOutput()->GetPixelContainer()->GetBufferPointer()), timeStep);
}

void mitk::FastMarchingTool3D::UpdatePrepare()
{
  // remove interaction with poinset while updating
  if (m_SeedPointInteractor.IsNotNull())
    m_SeedPointInteractor->SetDataNode(nullptr);
}

void mitk::FastMarchingTool3D::UpdateCleanUp()
{
  // add interaction with poinset again
  if (m_SeedPointInteractor.IsNotNull())
    m_SeedPointInteractor->SetDataNode(m_SeedsAsPointSetNode);
}

void mitk::FastMarchingTool3D::DoUpdatePreview(const Image* inputAtTimeStep, Image* previewImage, TimeStepType timeStep)
{
  if (nullptr != inputAtTimeStep && nullptr != previewImage && m_SeedContainer.IsNotNull() && !m_SeedContainer->empty())
  {
    AccessFixedDimensionByItk_n(inputAtTimeStep, DoITKFastMarching, 3, (previewImage, timeStep));
  }
}
