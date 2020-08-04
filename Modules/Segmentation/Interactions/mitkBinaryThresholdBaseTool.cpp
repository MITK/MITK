/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBinaryThresholdBaseTool.h"

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
#include <itkBinaryThresholdImageFilter.h>
#include <itkImageRegionIterator.h>

mitk::BinaryThresholdBaseTool::BinaryThresholdBaseTool()
  : m_SensibleMinimumThresholdValue(-100),
    m_SensibleMaximumThresholdValue(+100),
    m_CurrentLowerThresholdValue(1),
    m_CurrentUpperThresholdValue(1)
{
  m_ThresholdFeedbackNode = DataNode::New();
  m_ThresholdFeedbackNode->SetProperty("color", ColorProperty::New(0.0, 1.0, 0.0));
  m_ThresholdFeedbackNode->SetProperty("name", StringProperty::New("Thresholding feedback"));
  m_ThresholdFeedbackNode->SetProperty("opacity", FloatProperty::New(0.3));
  m_ThresholdFeedbackNode->SetProperty("binary", BoolProperty::New(true));
  m_ThresholdFeedbackNode->SetProperty("helper object", BoolProperty::New(true));
}

mitk::BinaryThresholdBaseTool::~BinaryThresholdBaseTool()
{
}

void mitk::BinaryThresholdBaseTool::Activated()
{
  Superclass::Activated();

  m_ToolManager->RoiDataChanged +=
    mitk::MessageDelegate<mitk::BinaryThresholdBaseTool>(this, &mitk::BinaryThresholdBaseTool::OnRoiDataChanged);

  m_ToolManager->SelectedTimePointChanged +=
    mitk::MessageDelegate<mitk::BinaryThresholdBaseTool>(this, &mitk::BinaryThresholdBaseTool::OnTimePointChanged);

  m_OriginalImageNode = m_ToolManager->GetReferenceData(0);
  m_NodeForThresholding = m_OriginalImageNode;

  if (m_NodeForThresholding.IsNotNull())
  {
    SetupPreviewNode();
  }
  else
  {
    m_ToolManager->ActivateTool(-1);
  }
}

void mitk::BinaryThresholdBaseTool::Deactivated()
{
  m_ToolManager->RoiDataChanged -=
    mitk::MessageDelegate<mitk::BinaryThresholdBaseTool>(this, &mitk::BinaryThresholdBaseTool::OnRoiDataChanged);

  m_ToolManager->SelectedTimePointChanged -=
    mitk::MessageDelegate<mitk::BinaryThresholdBaseTool>(this, &mitk::BinaryThresholdBaseTool::OnTimePointChanged);

  m_NodeForThresholding = nullptr;
  m_OriginalImageNode = nullptr;
  try
  {
    if (DataStorage *storage = m_ToolManager->GetDataStorage())
    {
      storage->Remove(m_ThresholdFeedbackNode);
      RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
  catch (...)
  {
    // don't care
  }
  m_ThresholdFeedbackNode->SetData(nullptr);

  Superclass::Deactivated();
}

void mitk::BinaryThresholdBaseTool::SetThresholdValues(double lower, double upper)
{
  /* If value is not in the min/max range, do nothing. In that case, this
     method will be called again with a proper value right after. The only
     known case where this happens is with an [0.0, 1.0[ image, where value
     could be an epsilon greater than the max. */
  if (lower < m_SensibleMinimumThresholdValue
    || lower > m_SensibleMaximumThresholdValue
    || upper < m_SensibleMinimumThresholdValue
    || upper > m_SensibleMaximumThresholdValue)
  {
    return;
  }

  m_CurrentLowerThresholdValue = lower;
  m_CurrentUpperThresholdValue = upper;

  if (m_ThresholdFeedbackNode.IsNotNull())
  {

    UpdatePreview();
  }
}

void mitk::BinaryThresholdBaseTool::AcceptCurrentThresholdValue()
{
  CreateNewSegmentationFromThreshold();

  RenderingManager::GetInstance()->RequestUpdateAll();
  m_ToolManager->ActivateTool(-1);
}

void mitk::BinaryThresholdBaseTool::CancelThresholding()
{
  m_ToolManager->ActivateTool(-1);
}

void mitk::BinaryThresholdBaseTool::SetupPreviewNode()
{
  itk::RGBPixel<float> pixel;
  pixel[0] = 0.0f;
  pixel[1] = 1.0f;
  pixel[2] = 0.0f;

  if (m_NodeForThresholding.IsNotNull())
  {
    Image::Pointer image = dynamic_cast<Image *>(m_NodeForThresholding->GetData());
    Image::Pointer originalImage = dynamic_cast<Image *>(m_OriginalImageNode->GetData());

    if (image.IsNotNull())
    {
      mitk::LabelSetImage::Pointer workingImage =
        dynamic_cast<mitk::LabelSetImage *>(m_ToolManager->GetWorkingData(0)->GetData());

      if (workingImage.IsNotNull())
      {
        m_ThresholdFeedbackNode->SetData(workingImage->Clone());
        m_IsOldBinary = false;

        // Let's paint the feedback node green...
        mitk::LabelSetImage::Pointer previewImage =
          dynamic_cast<mitk::LabelSetImage *>(m_ThresholdFeedbackNode->GetData());

        if (previewImage.IsNull())
        {
          MITK_ERROR << "Cannot create helper objects.";
          return;
        }

        previewImage->GetActiveLabel()->SetColor(pixel);
        previewImage->GetActiveLabelSet()->UpdateLookupTable(previewImage->GetActiveLabel()->GetValue());
      }
      else
      {
        mitk::Image::Pointer workingImageBin = dynamic_cast<mitk::Image *>(m_ToolManager->GetWorkingData(0)->GetData());
        if (workingImageBin)
        {
          m_ThresholdFeedbackNode->SetData(workingImageBin->Clone());
          m_IsOldBinary = true;
        }
        else
          m_ThresholdFeedbackNode->SetData(mitk::Image::New());
      }

      m_ThresholdFeedbackNode->SetColor(pixel);
      m_ThresholdFeedbackNode->SetOpacity(0.5);

      int layer(50);
      m_NodeForThresholding->GetIntProperty("layer", layer);
      m_ThresholdFeedbackNode->SetIntProperty("layer", layer + 1);

      if (DataStorage *ds = m_ToolManager->GetDataStorage())
      {
        if (!ds->Exists(m_ThresholdFeedbackNode))
          ds->Add(m_ThresholdFeedbackNode, m_OriginalImageNode);
      }

      if (image.GetPointer() == originalImage.GetPointer())
      {
        m_SensibleMinimumThresholdValue = std::numeric_limits<ScalarType>::max();
        m_SensibleMaximumThresholdValue = std::numeric_limits<ScalarType>::lowest();
        Image::StatisticsHolderPointer statistics = originalImage->GetStatistics();
        for (unsigned int ts = 0; ts < originalImage->GetTimeSteps(); ++ts)
        {
          m_SensibleMinimumThresholdValue = std::min(m_SensibleMinimumThresholdValue, static_cast<double>(statistics->GetScalarValueMin()));
          m_SensibleMaximumThresholdValue = std::max(m_SensibleMaximumThresholdValue, static_cast<double>(statistics->GetScalarValueMax()));
        }
      }

      if (m_LockedUpperThreshold)
      {
        m_CurrentLowerThresholdValue = (m_SensibleMaximumThresholdValue + m_SensibleMinimumThresholdValue) / 2.0;
        m_CurrentUpperThresholdValue = m_SensibleMaximumThresholdValue;
      }
      else
      {
        double range = m_SensibleMaximumThresholdValue - m_SensibleMinimumThresholdValue;
        m_CurrentLowerThresholdValue = m_SensibleMinimumThresholdValue + range / 3.0;
        m_CurrentUpperThresholdValue = m_SensibleMinimumThresholdValue + 2 * range / 3.0;
      }

      bool isFloatImage = false;
      if ((originalImage->GetPixelType().GetPixelType() == itk::ImageIOBase::SCALAR) &&
          (originalImage->GetPixelType().GetComponentType() == itk::ImageIOBase::FLOAT ||
           originalImage->GetPixelType().GetComponentType() == itk::ImageIOBase::DOUBLE))
      {
        isFloatImage = true;
      }

      IntervalBordersChanged.Send(m_SensibleMinimumThresholdValue, m_SensibleMaximumThresholdValue, isFloatImage);
      ThresholdingValuesChanged.Send(m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue);
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

void mitk::BinaryThresholdBaseTool::TransferImageAtTimeStep(const Image* sourceImage, Image* destinationImage, const TimeStepType timeStep)
{
  try
  {
    Image::ConstPointer image3D = this->Get3DImage(sourceImage, timeStep);

    if (image3D->GetDimension() == 2)
    {
      AccessFixedDimensionByItk_2(
        image3D, ITKSetVolume, 2, destinationImage, timeStep);
    }
    else
    {
      AccessFixedDimensionByItk_2(
        image3D, ITKSetVolume, 3, destinationImage, timeStep);
    }
  }
  catch (...)
  {
    Tool::ErrorMessage("Error accessing single time steps of the original image. Cannot create segmentation.");
  }
}

void mitk::BinaryThresholdBaseTool::CreateNewSegmentationFromThreshold()
{
  if (m_NodeForThresholding.IsNotNull() && m_ThresholdFeedbackNode.IsNotNull())
  {
    Image::Pointer feedBackImage = dynamic_cast<Image *>(m_ThresholdFeedbackNode->GetData());
    if (feedBackImage.IsNotNull())
    {
      // create a new image of the same dimensions and smallest possible pixel type
      DataNode::Pointer emptySegmentationNode = GetTargetSegmentationNode();

      if (emptySegmentationNode)
      {
        const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
        auto emptySegmentation = dynamic_cast<Image*>(emptySegmentationNode->GetData());

        // actually perform a thresholding
        // REMARK: the following code in this scope assumes that feedBackImage and emptySegmentationImage
        // are clones of the working image (segmentation provided to the tool). Therefor the have the same
        // time geometry.
        if (feedBackImage->GetTimeSteps() != emptySegmentation->GetTimeSteps())
        {
          mitkThrow() << "Cannot performe threshold. Internal tool state is invalid."
            << " Preview segmentation and segmentation result image have different time geometries.";
        }

        if (m_CreateAllTimeSteps)
        {
          for (unsigned int timeStep = 0; timeStep < feedBackImage->GetTimeSteps(); ++timeStep)
          {
            TransferImageAtTimeStep(feedBackImage, emptySegmentation, timeStep);
          }
        }
        else
        {
          const auto timeStep = emptySegmentation->GetTimeGeometry()->TimePointToTimeStep(timePoint);
          TransferImageAtTimeStep(feedBackImage, emptySegmentation, timeStep);
        }

        // since we are maybe working on a smaller image, pad it to the size of the original image
        if (m_OriginalImageNode.GetPointer() != m_NodeForThresholding.GetPointer())
        {
          mitk::PadImageFilter::Pointer padFilter = mitk::PadImageFilter::New();

          padFilter->SetInput(0, emptySegmentation);
          padFilter->SetInput(1, dynamic_cast<mitk::Image*>(m_OriginalImageNode->GetData()));
          padFilter->SetBinaryFilter(true);
          padFilter->SetUpperThreshold(1);
          padFilter->SetLowerThreshold(1);
          padFilter->Update();

          emptySegmentationNode->SetData(padFilter->GetOutput());
        }

        m_ToolManager->SetWorkingData(emptySegmentationNode);
        m_ToolManager->GetWorkingData(0)->Modified();
      }
    }
  }
}

void mitk::BinaryThresholdBaseTool::OnRoiDataChanged()
{
  mitk::DataNode::Pointer node = m_ToolManager->GetRoiData(0);

  if (node.IsNotNull())
  {
    mitk::MaskAndCutRoiImageFilter::Pointer roiFilter = mitk::MaskAndCutRoiImageFilter::New();
    mitk::Image::Pointer image = dynamic_cast<mitk::Image *>(m_NodeForThresholding->GetData());

    if (image.IsNull())
      return;

    roiFilter->SetInput(image);
    roiFilter->SetRegionOfInterest(node->GetData());
    roiFilter->Update();

    mitk::DataNode::Pointer tmpNode = mitk::DataNode::New();
    tmpNode->SetData(roiFilter->GetOutput());

    m_SensibleMinimumThresholdValue = static_cast<double>(roiFilter->GetMinValue());
    m_SensibleMaximumThresholdValue = static_cast<double>(roiFilter->GetMaxValue());

    m_NodeForThresholding = tmpNode;
  }
  else
    m_NodeForThresholding = m_OriginalImageNode;

  this->SetupPreviewNode();
  this->UpdatePreview();
}

void mitk::BinaryThresholdBaseTool::OnTimePointChanged()
{
  if (m_ThresholdFeedbackNode.IsNotNull() && m_NodeForThresholding.IsNotNull())
  {
    if (m_ThresholdFeedbackNode->GetData()->GetTimeSteps() == 1)
    {
      this->UpdatePreview();
    }
  }
}

template <typename TPixel, unsigned int VImageDimension>
static void ITKThresholding(const itk::Image<TPixel, VImageDimension> *originalImage,
                            mitk::Image *segmentation,
                            double lower,
                            double upper,
                            unsigned int timeStep)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<mitk::Tool::DefaultSegmentationDataType, VImageDimension> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;

  typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
  filter->SetInput(originalImage);
  filter->SetLowerThreshold(lower);
  filter->SetUpperThreshold(upper);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->Update();

  segmentation->SetVolume((void *)(filter->GetOutput()->GetPixelContainer()->GetBufferPointer()), timeStep);
}

template <typename TPixel, unsigned int VImageDimension>
static void ITKThresholdingOldBinary(const itk::Image<TPixel, VImageDimension> *originalImage,
                                     mitk::Image *segmentation,
                                     double lower,
                                     double upper,
                                     unsigned int timeStep)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned char, VImageDimension> SegmentationType;
  typedef itk::BinaryThresholdImageFilter<ImageType, SegmentationType> ThresholdFilterType;

  typename ThresholdFilterType::Pointer filter = ThresholdFilterType::New();
  filter->SetInput(originalImage);
  filter->SetLowerThreshold(lower);
  filter->SetUpperThreshold(upper);
  filter->SetInsideValue(1);
  filter->SetOutsideValue(0);
  filter->Update();

  segmentation->SetVolume((void *)(filter->GetOutput()->GetPixelContainer()->GetBufferPointer()), timeStep);
}

void mitk::BinaryThresholdBaseTool::UpdatePreview()
{
  mitk::Image::Pointer thresholdImage = dynamic_cast<mitk::Image *>(m_NodeForThresholding->GetData());
  mitk::Image::Pointer previewImage = dynamic_cast<mitk::Image *>(m_ThresholdFeedbackNode->GetData());
  if (thresholdImage && previewImage)
  {
    if (previewImage->GetTimeSteps() > 1)
    {
      for (unsigned int timeStep = 0; timeStep < thresholdImage->GetTimeSteps(); ++timeStep)
      {
        auto feedBackImage3D = this->Get3DImage(thresholdImage, timeStep);

        if (m_IsOldBinary)
        {
          AccessByItk_n(feedBackImage3D,
            ITKThresholdingOldBinary,
            (previewImage, m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue, timeStep));
        }
        else
        {
          AccessByItk_n(feedBackImage3D,
            ITKThresholding,
            (previewImage, m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue, timeStep));
        }
      }
    }
    else
    {
      const auto timePoint = mitk::RenderingManager::GetInstance()->GetTimeNavigationController()->GetSelectedTimePoint();
      auto feedBackImage3D = this->Get3DImageByTimePoint(thresholdImage, timePoint);
      auto timeStep = previewImage->GetTimeGeometry()->TimePointToTimeStep(timePoint);

      if (m_IsOldBinary)
      {
        AccessByItk_n(feedBackImage3D,
          ITKThresholdingOldBinary,
          (previewImage, m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue, timeStep));
      }
      else
      {
        AccessByItk_n(feedBackImage3D,
          ITKThresholding,
          (previewImage, m_CurrentLowerThresholdValue, m_CurrentUpperThresholdValue, timeStep));
      }
    }
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
}
