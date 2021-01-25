/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkAutoSegmentationWithPreviewTool.h"

#include "mitkToolManager.h"

#include "mitkColorProperty.h"
#include "mitkLevelWindowProperty.h"
#include "mitkProperties.h"

#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"
#include <mitkSliceNavigationController.h>

#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include "mitkImageStatisticsHolder.h"
#include "mitkImageTimeSelector.h"
#include "mitkLabelSetImage.h"
#include "mitkMaskAndCutRoiImageFilter.h"
#include "mitkPadImageFilter.h"
#include "mitkNodePredicateGeometry.h"
#include "mitkSegTool2D.h"

mitk::AutoSegmentationWithPreviewTool::AutoSegmentationWithPreviewTool(bool lazyDynamicPreviews): m_LazyDynamicPreviews(lazyDynamicPreviews)
{
  m_ProgressCommand = mitk::ToolCommand::New();
}

mitk::AutoSegmentationWithPreviewTool::AutoSegmentationWithPreviewTool(bool lazyDynamicPreviews, const char* interactorType, const us::Module* interactorModule) : AutoSegmentationTool(interactorType, interactorModule), m_LazyDynamicPreviews(lazyDynamicPreviews)
{
  m_ProgressCommand = mitk::ToolCommand::New();
}


mitk::AutoSegmentationWithPreviewTool::~AutoSegmentationWithPreviewTool()
{
}

bool mitk::AutoSegmentationWithPreviewTool::CanHandle(const BaseData* referenceData, const BaseData* workingData) const
{
  if (!Superclass::CanHandle(referenceData, workingData))
    return false;

  if (workingData == nullptr)
    return true;

  auto* labelSet = dynamic_cast<const LabelSetImage*>(workingData);

  if (labelSet != nullptr)
    return true;

  auto* image = dynamic_cast<const Image*>(workingData);

  if (image == nullptr)
    return false;

  //if it is a normal image and not a label set image is used as working data
  //it must have the same pixel type as a label set.
  return MakeScalarPixelType< DefaultSegmentationDataType >() == image->GetPixelType();
}

void mitk::AutoSegmentationWithPreviewTool::Activated()
{
  Superclass::Activated();

  m_ToolManager->RoiDataChanged +=
    mitk::MessageDelegate<mitk::AutoSegmentationWithPreviewTool>(this, &mitk::AutoSegmentationWithPreviewTool::OnRoiDataChanged);

  m_ToolManager->SelectedTimePointChanged +=
    mitk::MessageDelegate<mitk::AutoSegmentationWithPreviewTool>(this, &mitk::AutoSegmentationWithPreviewTool::OnTimePointChanged);

  m_ReferenceDataNode = m_ToolManager->GetReferenceData(0);
  m_SegmentationInputNode = m_ReferenceDataNode;

  m_LastTimePointOfUpdate = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();

  if (m_PreviewSegmentationNode.IsNull())
  {
    m_PreviewSegmentationNode = DataNode::New();
    m_PreviewSegmentationNode->SetProperty("color", ColorProperty::New(0.0, 1.0, 0.0));
    m_PreviewSegmentationNode->SetProperty("name", StringProperty::New(std::string(this->GetName())+" preview"));
    m_PreviewSegmentationNode->SetProperty("opacity", FloatProperty::New(0.3));
    m_PreviewSegmentationNode->SetProperty("binary", BoolProperty::New(true));
    m_PreviewSegmentationNode->SetProperty("helper object", BoolProperty::New(true));
  }

  if (m_SegmentationInputNode.IsNotNull())
  {
    this->ResetPreviewNode();
    this->InitiateToolByInput();
  }
  else
  {
    m_ToolManager->ActivateTool(-1);
  }
}

void mitk::AutoSegmentationWithPreviewTool::Deactivated()
{
  m_ToolManager->RoiDataChanged -=
    mitk::MessageDelegate<mitk::AutoSegmentationWithPreviewTool>(this, &mitk::AutoSegmentationWithPreviewTool::OnRoiDataChanged);

  m_ToolManager->SelectedTimePointChanged -=
    mitk::MessageDelegate<mitk::AutoSegmentationWithPreviewTool>(this, &mitk::AutoSegmentationWithPreviewTool::OnTimePointChanged);

  m_SegmentationInputNode = nullptr;
  m_ReferenceDataNode = nullptr;
  m_WorkingPlaneGeometry = nullptr;

  try
  {
    if (DataStorage *storage = m_ToolManager->GetDataStorage())
    {
      storage->Remove(m_PreviewSegmentationNode);
      RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  catch (...)
  {
    // don't care
  }

  if (m_PreviewSegmentationNode.IsNotNull())
  {
    m_PreviewSegmentationNode->SetData(nullptr);
  }

  Superclass::Deactivated();
}

void mitk::AutoSegmentationWithPreviewTool::ConfirmSegmentation()
{
  if (m_LazyDynamicPreviews && m_CreateAllTimeSteps)
  { // The tool should create all time steps but is currently in lazy mode,
    // thus ensure that a preview for all time steps is available.
    this->UpdatePreview(true);
  }

  CreateResultSegmentationFromPreview();

  RenderingManager::GetInstance()->RequestUpdateAll();

  if (!m_KeepActiveAfterAccept)
  {
    m_ToolManager->ActivateTool(-1);
  }
}

void  mitk::AutoSegmentationWithPreviewTool::InitiateToolByInput()
{
  //default implementation does nothing.
  //implement in derived classes to change behavior
}

mitk::Image* mitk::AutoSegmentationWithPreviewTool::GetPreviewSegmentation()
{
  if (m_PreviewSegmentationNode.IsNull())
  {
    return nullptr;
  }

  return dynamic_cast<Image*>(m_PreviewSegmentationNode->GetData());
}

mitk::DataNode* mitk::AutoSegmentationWithPreviewTool::GetPreviewSegmentationNode()
{
  return m_PreviewSegmentationNode;
}

const mitk::Image* mitk::AutoSegmentationWithPreviewTool::GetSegmentationInput() const
{
  if (m_SegmentationInputNode.IsNull())
  {
    return nullptr;
  }

  return dynamic_cast<const Image*>(m_SegmentationInputNode->GetData());
}

const mitk::Image* mitk::AutoSegmentationWithPreviewTool::GetReferenceData() const
{
  if (m_ReferenceDataNode.IsNull())
  {
    return nullptr;
  }

  return dynamic_cast<const Image*>(m_ReferenceDataNode->GetData());
}

void mitk::AutoSegmentationWithPreviewTool::ResetPreviewNode()
{
  itk::RGBPixel<float> previewColor;
  previewColor[0] = 0.0f;
  previewColor[1] = 1.0f;
  previewColor[2] = 0.0f;

  const auto image = this->GetSegmentationInput();
  if (nullptr != image)
  {
    mitk::LabelSetImage::ConstPointer workingImage =
      dynamic_cast<const mitk::LabelSetImage *>(m_ToolManager->GetWorkingData(0)->GetData());

    if (workingImage.IsNotNull())
    {
      auto newPreviewImage = workingImage->Clone();
      if (newPreviewImage.IsNull())
      {
        MITK_ERROR << "Cannot create preview helper objects. Unable to clone working image";
        return;
      }

      m_PreviewSegmentationNode->SetData(newPreviewImage);

      // Let's paint the feedback node green...
      newPreviewImage->GetActiveLabel()->SetColor(previewColor);
      newPreviewImage->GetActiveLabelSet()->UpdateLookupTable(newPreviewImage->GetActiveLabel()->GetValue());
    }
    else
    {
      mitk::Image::ConstPointer workingImageBin = dynamic_cast<const mitk::Image*>(m_ToolManager->GetWorkingData(0)->GetData());
      if (workingImageBin.IsNotNull())
      {
        auto newPreviewImage = workingImageBin->Clone();
        if (newPreviewImage.IsNull())
        {
          MITK_ERROR << "Cannot create preview helper objects. Unable to clone working image";
          return;
        }

        m_PreviewSegmentationNode->SetData(newPreviewImage->Clone());
      }
      else
      {
        mitkThrow() << "Tool is an invalid state. Cannot setup preview node. Working data is an unsupported class and should have not been accepted by CanHandle().";
      }
    }

    m_PreviewSegmentationNode->SetColor(previewColor);
    m_PreviewSegmentationNode->SetOpacity(0.5);

    int layer(50);
    m_ReferenceDataNode->GetIntProperty("layer", layer);
    m_PreviewSegmentationNode->SetIntProperty("layer", layer + 1);

    if (DataStorage *ds = m_ToolManager->GetDataStorage())
    {
      if (!ds->Exists(m_PreviewSegmentationNode))
        ds->Add(m_PreviewSegmentationNode, m_ReferenceDataNode);
    }
  }
}

template <typename TPixel, unsigned int VImageDimension>
static void ITKSetVolume(const itk::Image<TPixel, VImageDimension> *originalImage,
                         mitk::Image *segmentation,
                         unsigned int timeStep)
{
  auto constPixelContainer = originalImage->GetPixelContainer();
  //have to make a const cast because itk::PixelContainer does not provide a const correct access :(
  auto pixelContainer = const_cast<typename itk::Image<TPixel, VImageDimension>::PixelContainer*>(constPixelContainer);

  segmentation->SetVolume((void *)pixelContainer->GetBufferPointer(), timeStep);
}

void mitk::AutoSegmentationWithPreviewTool::TransferImageAtTimeStep(const Image* sourceImage, Image* destinationImage, const TimeStepType timeStep)
{
  try
  {
    Image::ConstPointer sourceImageAtTimeStep = this->GetImageByTimeStep(sourceImage, timeStep);

    if (sourceImageAtTimeStep->GetPixelType() != destinationImage->GetPixelType())
    {
      mitkThrow() << "Cannot transfer images. Tool is in an invalid state, source image and destination image do not have the same pixel type. "
        << "Source pixel type: " << sourceImage->GetPixelType().GetTypeAsString()
        << "; destination pixel type: " << destinationImage->GetPixelType().GetTypeAsString();
    }

    if (!Equal(*(sourceImage->GetGeometry(timeStep)), *(destinationImage->GetGeometry(timeStep)), NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_PRECISION, false))
    {
      mitkThrow() << "Cannot transfer images. Tool is in an invalid state, source image and destination image do not have the same geometry.";
    }

    if (nullptr != this->GetWorkingPlaneGeometry())
    {
      auto sourceSlice = SegTool2D::GetAffectedImageSliceAs2DImage(this->GetWorkingPlaneGeometry(), sourceImage, timeStep);
      SegTool2D::WriteBackSegmentationResult(this->GetTargetSegmentationNode(), m_WorkingPlaneGeometry, sourceSlice, timeStep);
    }
    else
    { //take care of the full segmentation volume
      if (sourceImageAtTimeStep->GetDimension() == 2)
      {
        AccessFixedDimensionByItk_2(
          sourceImageAtTimeStep, ITKSetVolume, 2, destinationImage, timeStep);
      }
      else
      {
        AccessFixedDimensionByItk_2(
          sourceImageAtTimeStep, ITKSetVolume, 3, destinationImage, timeStep);
      }
    }
  }
  catch (...)
  {
    Tool::ErrorMessage("Error accessing single time steps of the original image. Cannot create segmentation.");
    throw;
  }
}

void mitk::AutoSegmentationWithPreviewTool::CreateResultSegmentationFromPreview()
{
  const auto segInput = this->GetSegmentationInput();
  auto previewImage = this->GetPreviewSegmentation();
  if (nullptr != segInput && nullptr != previewImage)
  {
    DataNode::Pointer resultSegmentationNode = GetTargetSegmentationNode();

    if (resultSegmentationNode.IsNotNull())
    {
      const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
      auto resultSegmentation = dynamic_cast<Image*>(resultSegmentationNode->GetData());

      // REMARK: the following code in this scope assumes that previewImage and resultSegmentation
      // are clones of the working image (segmentation provided to the tool). Therefore they have
      // the same time geometry.
      if (previewImage->GetTimeSteps() != resultSegmentation->GetTimeSteps())
      {
        mitkThrow() << "Cannot perform threshold. Internal tool state is invalid."
          << " Preview segmentation and segmentation result image have different time geometries.";
      }

      if (m_CreateAllTimeSteps)
      {
        for (unsigned int timeStep = 0; timeStep < previewImage->GetTimeSteps(); ++timeStep)
        {
          this->TransferImageAtTimeStep(previewImage, resultSegmentation, timeStep);
        }
      }
      else
      {
        const auto timeStep = resultSegmentation->GetTimeGeometry()->TimePointToTimeStep(timePoint);
        this->TransferImageAtTimeStep(previewImage, resultSegmentation, timeStep);
      }

      // since we are maybe working on a smaller image, pad it to the size of the original image
      if (m_ReferenceDataNode.GetPointer() != m_SegmentationInputNode.GetPointer())
      {
        mitk::PadImageFilter::Pointer padFilter = mitk::PadImageFilter::New();

        padFilter->SetInput(0, resultSegmentation);
        padFilter->SetInput(1, dynamic_cast<mitk::Image*>(m_ReferenceDataNode->GetData()));
        padFilter->SetBinaryFilter(true);
        padFilter->SetUpperThreshold(1);
        padFilter->SetLowerThreshold(1);
        padFilter->Update();

        resultSegmentationNode->SetData(padFilter->GetOutput());
      }

      m_ToolManager->SetWorkingData(resultSegmentationNode);
      m_ToolManager->GetWorkingData(0)->Modified();
      this->EnsureTargetSegmentationNodeInDataStorage();
    }
  }
}

void mitk::AutoSegmentationWithPreviewTool::OnRoiDataChanged()
{
  mitk::DataNode::ConstPointer node = m_ToolManager->GetRoiData(0);

  if (node.IsNotNull())
  {
    mitk::MaskAndCutRoiImageFilter::Pointer roiFilter = mitk::MaskAndCutRoiImageFilter::New();
    mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(m_SegmentationInputNode->GetData());

    if (image.IsNull())
      return;

    roiFilter->SetInput(image);
    roiFilter->SetRegionOfInterest(node->GetData());
    roiFilter->Update();

    mitk::DataNode::Pointer tmpNode = mitk::DataNode::New();
    tmpNode->SetData(roiFilter->GetOutput());

    m_SegmentationInputNode = tmpNode;
  }
  else
    m_SegmentationInputNode = m_ReferenceDataNode;

  this->ResetPreviewNode();
  this->InitiateToolByInput();
  this->UpdatePreview();
}

void mitk::AutoSegmentationWithPreviewTool::OnTimePointChanged()
{
  if (m_IsTimePointChangeAware && m_PreviewSegmentationNode.IsNotNull() && m_SegmentationInputNode.IsNotNull())
  {
    const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();

    const bool isStaticSegOnDynamicImage = m_PreviewSegmentationNode->GetData()->GetTimeSteps() == 1 && m_SegmentationInputNode->GetData()->GetTimeSteps() > 1;
    if (timePoint!=m_LastTimePointOfUpdate && (isStaticSegOnDynamicImage || m_LazyDynamicPreviews))
    { //we only need to update either because we are lazzy
      //or because we have a static segmentation with a dynamic image 
      this->UpdatePreview();
    }
  }
}

void mitk::AutoSegmentationWithPreviewTool::UpdatePreview(bool ignoreLazyPreviewSetting)
{
  const auto inputImage = this->GetSegmentationInput();
  auto previewImage = this->GetPreviewSegmentation();
  int progress_steps = 200;

  this->CurrentlyBusy.Send(true);
  m_IsUpdating = true;

  this->UpdatePrepare();

  const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();

  try
  {
    if (nullptr != inputImage && nullptr != previewImage)
    {
      m_ProgressCommand->AddStepsToDo(progress_steps);

      if (previewImage->GetTimeSteps() > 1 && (ignoreLazyPreviewSetting || !m_LazyDynamicPreviews))
      {
        for (unsigned int timeStep = 0; timeStep < previewImage->GetTimeSteps(); ++timeStep)
        {
          Image::ConstPointer feedBackImage;
          auto previewTimePoint = previewImage->GetTimeGeometry()->TimeStepToTimePoint(timeStep);
          auto inputTimeStep = inputImage->GetTimeGeometry()->TimePointToTimeStep(previewTimePoint);

          if (nullptr != this->GetWorkingPlaneGeometry())
          { //only extract a specific slice defined by the working plane as feedback image.
            feedBackImage = SegTool2D::GetAffectedImageSliceAs2DImage(this->GetWorkingPlaneGeometry(), inputImage, inputTimeStep);
          }
          else
          { //work on the whole feedback image
            feedBackImage = this->GetImageByTimeStep(inputImage, inputTimeStep);
          }

          this->DoUpdatePreview(feedBackImage, previewImage, timeStep);
        }
      }
      else
      {
        Image::ConstPointer feedBackImage;
        if (nullptr != this->GetWorkingPlaneGeometry())
        {
          feedBackImage = SegTool2D::GetAffectedImageSliceAs2DImageByTimePoint(this->GetWorkingPlaneGeometry(), inputImage, timePoint);
        }
        else
        {
          feedBackImage = this->GetImageByTimePoint(inputImage, timePoint);
        }

        auto timeStep = previewImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

        this->DoUpdatePreview(feedBackImage, previewImage, timeStep);
      }
      RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  catch (itk::ExceptionObject & excep)
  {
    MITK_ERROR << "Exception caught: " << excep.GetDescription();

    m_ProgressCommand->SetProgress(progress_steps);

    std::string msg = excep.GetDescription();
    ErrorMessage.Send(msg);
  }
  catch (...)
  {
    m_ProgressCommand->SetProgress(progress_steps);
    m_IsUpdating = false;
    CurrentlyBusy.Send(false);
    throw;
  }

  this->UpdateCleanUp();
  m_LastTimePointOfUpdate = timePoint;
  m_ProgressCommand->SetProgress(progress_steps);
  m_IsUpdating = false;
  CurrentlyBusy.Send(false);
}

bool mitk::AutoSegmentationWithPreviewTool::IsUpdating() const
{
  return m_IsUpdating;
}

void mitk::AutoSegmentationWithPreviewTool::UpdatePrepare()
{
  // default implementation does nothing
  //reimplement in derived classes for special behavior
}

void mitk::AutoSegmentationWithPreviewTool::UpdateCleanUp()
{
  // default implementation does nothing
  //reimplement in derived classes for special behavior
}

mitk::TimePointType mitk::AutoSegmentationWithPreviewTool::GetLastTimePointOfUpdate() const
{
  return m_LastTimePointOfUpdate;
}
