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

#include "mitkBinaryThresholdTool.h"

#include "mitkToolManager.h"
#include "mitkBoundingObjectToSegmentationFilter.h"

#include <mitkCoreObjectFactory.h>
#include "mitkLevelWindowProperty.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"
#include "mitkOrganTypeProperty.h"
#include "mitkVtkResliceInterpolationProperty.h"
#include "mitkDataStorage.h"
#include "mitkRenderingManager.h"

#include "mitkImageCast.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageTimeSelector.h"
#include "mitkImageStatisticsHolder.h"
#include <itkImageRegionIterator.h>
#include <itkBinaryThresholdImageFilter.h>
#include "mitkPadImageFilter.h"
#include "mitkMaskAndCutRoiImageFilter.h"
#include "mitkLabelSetImage.h"

// us
#include "usModule.h"
#include "usModuleResource.h"
#include "usGetModuleContext.h"

namespace mitk {
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, BinaryThresholdTool, "Thresholding tool");
}

mitk::BinaryThresholdTool::BinaryThresholdTool()
{
  m_ThresholdFeedbackNode = DataNode::New();
  m_ThresholdFeedbackNode->SetProperty("name", StringProperty::New("Thresholding feedback"));
  m_ThresholdFeedbackNode->SetProperty("helper object", BoolProperty::New(true));
  m_ThresholdFeedbackNode->SetFloatProperty("labelset.contour.width", 0.0);
}

mitk::BinaryThresholdTool::~BinaryThresholdTool()
{
}

us::ModuleResource mitk::BinaryThresholdTool::GetIconResource() const
{
  us::Module* module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Threshold_48x48.png");
  return resource;
}

const char* mitk::BinaryThresholdTool::GetName() const
{
  return "Threshold";
}

void mitk::BinaryThresholdTool::SetThresholdValue(double value)
{
  if (m_ThresholdFeedbackNode.IsNotNull())
  {
    m_CurrentLowerThresholdValue = value;
    // Bug 19250: The range of 0.01 is rather random. It was 0.001 before and probably due to rounding error propagation in VTK code
    // it leads to strange banding effects on floating point images with a huge range (like -40000 - 40000). 0.01 lowers this effect
    // enough to work with our images. Might not work on images with really huge ranges, though. Anyways, still seems to be low enough
    // to work for floating point images with a range between 0 and 1. A better solution might be to dynamically calculate the value
    // based on the value range of the current image (as big as possible, as small as necessary).
    m_ThresholdFeedbackNode->SetProperty("levelwindow", LevelWindowProperty::New(LevelWindow(m_CurrentLowerThresholdValue, 0.01)));
    UpdatePreview();
  }
}

void mitk::BinaryThresholdTool::updateThresholdValue()
{
  m_CurrentLowerThresholdValue = (m_SensibleMaximumThresholdValue + m_SensibleMinimumThresholdValue) / 2.0;
  ThresholdingValueChanged.Send(m_CurrentLowerThresholdValue);
}

void mitk::BinaryThresholdTool::runItkThreshold(Image::Pointer feedbackImage3D, Image::Pointer previewImage, unsigned int timeStep)
{
  AccessByItk_n(feedbackImage3D, ITKThresholding, (previewImage, m_CurrentLowerThresholdValue, m_SensibleMaximumThresholdValue, timeStep));
}
