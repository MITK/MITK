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
#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkInteractionConst.h"
#include "mitkRenderingManager.h"

#include "itkOrImageFilter.h"
#include "mitkImageCast.h"
#include "mitkImageTimeSelector.h"

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
  : /*FeedbackContourTool*/ AutoSegmentationTool(),
    m_NeedUpdate(true),
    m_CurrentTimeStep(0),
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

bool mitk::FastMarchingTool3D::CanHandle(BaseData *referenceData) const
{
  if (referenceData == nullptr)
    return false;

  auto *image = dynamic_cast<Image *>(referenceData);

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
  m_ThresholdFilter->SetUpperThreshold(m_UpperThreshold);
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::SetLowerThreshold(double value)
{
  m_LowerThreshold = value / 10.0;
  m_ThresholdFilter->SetLowerThreshold(m_LowerThreshold);
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::SetBeta(double value)
{
  if (m_Beta != value)
  {
    m_Beta = value;
    m_SigmoidFilter->SetBeta(m_Beta);
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool3D::SetSigma(double value)
{
  if (m_Sigma != value)
  {
    if (value > 0.0)
    {
      m_Sigma = value;
      m_GradientMagnitudeFilter->SetSigma(m_Sigma);
      m_NeedUpdate = true;
    }
  }
}

void mitk::FastMarchingTool3D::SetAlpha(double value)
{
  if (m_Alpha != value)
  {
    m_Alpha = value;
    m_SigmoidFilter->SetAlpha(m_Alpha);
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool3D::SetStoppingValue(double value)
{
  if (m_StoppingValue != value)
  {
    m_StoppingValue = value;
    m_FastMarchingFilter->SetStoppingValue(m_StoppingValue);
    m_NeedUpdate = true;
  }
}

void mitk::FastMarchingTool3D::Activated()
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

  m_ReferenceImageAsITK = InternalImageType::New();

  m_ProgressCommand = mitk::ToolCommand::New();

  m_ThresholdFilter = ThresholdingFilterType::New();
  m_ThresholdFilter->SetLowerThreshold(m_LowerThreshold);
  m_ThresholdFilter->SetUpperThreshold(m_UpperThreshold);
  m_ThresholdFilter->SetOutsideValue(0);
  m_ThresholdFilter->SetInsideValue(1.0);

  m_SmoothFilter = SmoothingFilterType::New();
  m_SmoothFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  m_SmoothFilter->SetTimeStep(0.05);
  m_SmoothFilter->SetNumberOfIterations(2);
  m_SmoothFilter->SetConductanceParameter(9.0);

  m_GradientMagnitudeFilter = GradientFilterType::New();
  m_GradientMagnitudeFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  m_GradientMagnitudeFilter->SetSigma(m_Sigma);

  m_SigmoidFilter = SigmoidFilterType::New();
  m_SigmoidFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  m_SigmoidFilter->SetAlpha(m_Alpha);
  m_SigmoidFilter->SetBeta(m_Beta);
  m_SigmoidFilter->SetOutputMinimum(0.0);
  m_SigmoidFilter->SetOutputMaximum(1.0);

  m_FastMarchingFilter = FastMarchingFilterType::New();
  m_FastMarchingFilter->AddObserver(itk::ProgressEvent(), m_ProgressCommand);
  m_FastMarchingFilter->SetStoppingValue(m_StoppingValue);

  m_SeedContainer = NodeContainer::New();
  m_SeedContainer->Initialize();
  m_FastMarchingFilter->SetTrialPoints(m_SeedContainer);

  // set up pipeline
  m_SmoothFilter->SetInput(m_ReferenceImageAsITK);
  m_GradientMagnitudeFilter->SetInput(m_SmoothFilter->GetOutput());
  m_SigmoidFilter->SetInput(m_GradientMagnitudeFilter->GetOutput());
  m_FastMarchingFilter->SetInput(m_SigmoidFilter->GetOutput());
  m_ThresholdFilter->SetInput(m_FastMarchingFilter->GetOutput());

  m_ToolManager->GetDataStorage()->Add(m_SeedsAsPointSetNode, m_ToolManager->GetWorkingData(0));

  itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::Pointer pointAddedCommand =
    itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::New();
  pointAddedCommand->SetCallbackFunction(this, &mitk::FastMarchingTool3D::OnAddPoint);
  m_PointSetAddObserverTag = m_SeedsAsPointSet->AddObserver(mitk::PointSetAddEvent(), pointAddedCommand);

  itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::Pointer pointRemovedCommand =
    itk::SimpleMemberCommand<mitk::FastMarchingTool3D>::New();
  pointRemovedCommand->SetCallbackFunction(this, &mitk::FastMarchingTool3D::OnDelete);
  m_PointSetRemoveObserverTag = m_SeedsAsPointSet->AddObserver(mitk::PointSetRemoveEvent(), pointRemovedCommand);

  this->Initialize();
}

void mitk::FastMarchingTool3D::Deactivated()
{
  m_ToolManager->GetDataStorage()->Remove(this->m_ResultImageNode);
  m_ToolManager->GetDataStorage()->Remove(this->m_SeedsAsPointSetNode);
  this->ClearSeeds();
  this->m_SmoothFilter->RemoveAllObservers();
  this->m_SigmoidFilter->RemoveAllObservers();
  this->m_GradientMagnitudeFilter->RemoveAllObservers();
  this->m_FastMarchingFilter->RemoveAllObservers();
  m_ResultImageNode = nullptr;
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  unsigned int numberOfPoints = m_SeedsAsPointSet->GetSize();
  for (unsigned int i = 0; i < numberOfPoints; ++i)
  {
    mitk::Point3D point = m_SeedsAsPointSet->GetPoint(i);
    auto *doOp = new mitk::PointOperation(mitk::OpREMOVE, point, 0);
    m_SeedsAsPointSet->ExecuteOperation(doOp);
  }
  // Deactivate Interaction
  m_SeedPointInteractor->SetDataNode(nullptr);
  m_ToolManager->GetDataStorage()->Remove(m_SeedsAsPointSetNode);
  m_SeedsAsPointSetNode = nullptr;
  m_SeedsAsPointSet->RemoveObserver(m_PointSetAddObserverTag);
  m_SeedsAsPointSet->RemoveObserver(m_PointSetRemoveObserverTag);

  Superclass::Deactivated();
}

void mitk::FastMarchingTool3D::Initialize()
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
  CastToItkImage(m_ReferenceImage, m_ReferenceImageAsITK);
  m_SmoothFilter->SetInput(m_ReferenceImageAsITK);
  m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::ConfirmSegmentation()
{
  // combine preview image with current working segmentation
  if (dynamic_cast<mitk::Image *>(m_ResultImageNode->GetData()))
  {
    // logical or combination of preview and segmentation slice
    OutputImageType::Pointer segmentationImageInITK = OutputImageType::New();

    mitk::Image::Pointer workingImage = dynamic_cast<mitk::Image *>(GetTargetSegmentationNode()->GetData());
    if (workingImage->GetTimeGeometry()->CountTimeSteps() > 1)
    {
      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput(workingImage);
      timeSelector->SetTimeNr(m_CurrentTimeStep);
      timeSelector->UpdateLargestPossibleRegion();
      CastToItkImage(timeSelector->GetOutput(), segmentationImageInITK);
    }
    else
    {
      CastToItkImage(workingImage, segmentationImageInITK);
    }

    typedef itk::OrImageFilter<OutputImageType, OutputImageType> OrImageFilterType;
    OrImageFilterType::Pointer orFilter = OrImageFilterType::New();

    orFilter->SetInput(0, m_ThresholdFilter->GetOutput());
    orFilter->SetInput(1, segmentationImageInITK);
    orFilter->Update();

    // set image volume in current time step from itk image
    workingImage->SetVolume((void *)(m_ThresholdFilter->GetOutput()->GetPixelContainer()->GetBufferPointer()),
                            m_CurrentTimeStep);
    this->m_ResultImageNode->SetVisibility(false);
    this->ClearSeeds();
    workingImage->Modified();
  }

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);
}

void mitk::FastMarchingTool3D::OnAddPoint()
{
  // Add a new seed point for FastMarching algorithm
  mitk::Point3D clickInIndex;

  m_ReferenceImage->GetGeometry()->WorldToIndex(m_SeedsAsPointSet->GetPoint(m_SeedsAsPointSet->GetSize() - 1),
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
  m_FastMarchingFilter->Modified();

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  m_NeedUpdate = true;

  this->Update();

  m_ReadyMessage.Send();
}

void mitk::FastMarchingTool3D::OnDelete()
{
  // delete last seed point
  if (!(this->m_SeedContainer->empty()))
  {
    // delete last element of seeds container
    this->m_SeedContainer->pop_back();
    m_FastMarchingFilter->Modified();

    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    m_NeedUpdate = true;

    this->Update();
  }
}

void mitk::FastMarchingTool3D::Update()
{
  const unsigned int progress_steps = 200;

  if (m_NeedUpdate)
  {
    m_ProgressCommand->AddStepsToDo(progress_steps);

    // remove interaction with poinset while updating
    m_SeedPointInteractor->SetDataNode(nullptr);
    CurrentlyBusy.Send(true);
    try
    {
      m_ThresholdFilter->Update();
    }
    catch (itk::ExceptionObject &excep)
    {
      MITK_ERROR << "Exception caught: " << excep.GetDescription();

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
    result->GetGeometry()->SetOrigin(m_ReferenceImage->GetGeometry()->GetOrigin());
    result->GetGeometry()->SetIndexToWorldTransform(m_ReferenceImage->GetGeometry()->GetIndexToWorldTransform());
    m_ResultImageNode->SetData(result);
    m_ResultImageNode->SetVisibility(true);
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();

    // add interaction with poinset again
    m_SeedPointInteractor->SetDataNode(m_SeedsAsPointSetNode);
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

  if (this->m_FastMarchingFilter.IsNotNull())
    m_FastMarchingFilter->Modified();

  this->m_NeedUpdate = true;
}

void mitk::FastMarchingTool3D::Reset()
{
  // clear all seeds and preview empty result
  this->ClearSeeds();

  m_ResultImageNode->SetVisibility(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::FastMarchingTool3D::SetCurrentTimeStep(int t)
{
  if (m_CurrentTimeStep != t)
  {
    m_CurrentTimeStep = t;

    this->Initialize();
  }
}
