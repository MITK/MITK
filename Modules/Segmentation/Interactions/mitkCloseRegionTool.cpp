/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCloseRegionTool.h"

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

#include <mitkImageAccessByItk.h>

#include <itkBinaryFillholeImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, CloseRegionTool, "Close tool");
}

const char **mitk::CloseRegionTool::GetXPM() const
{
  return nullptr;
}

us::ModuleResource mitk::CloseRegionTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Close.svg");
  return resource;
}

us::ModuleResource mitk::CloseRegionTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Close_Cursor.svg");
  return resource;
}

const char *mitk::CloseRegionTool::GetName() const
{
  return "Close";
}


template <typename TPixel, unsigned int VImageDimension>
void DoITKRegionClosing(const itk::Image<TPixel, VImageDimension>* oldSegImage,
  mitk::Image::Pointer& filledRegionImage, itk::Index<VImageDimension> seedIndex, mitk::Label::PixelType& seedLabel)
{
  typedef itk::Image<TPixel, VImageDimension> InputImageType;
  typedef itk::Image<mitk::Label::PixelType, VImageDimension> OutputImageType;
  typedef itk::ConnectedThresholdImageFilter<InputImageType, OutputImageType> RegionGrowingFilterType;
  using FillHoleFilter = itk::BinaryFillholeImageFilter<OutputImageType>;

  seedLabel = oldSegImage->GetPixel(seedIndex);

  typename OutputImageType::Pointer itkResultImage;
  filledRegionImage = nullptr;

  try
  {
    auto regionGrower = RegionGrowingFilterType::New();
    regionGrower->SetInput(oldSegImage);
    regionGrower->SetReplaceValue(1);
    regionGrower->AddSeed(seedIndex);

    regionGrower->SetLower(seedLabel);
    regionGrower->SetUpper(seedLabel);

    auto filler = FillHoleFilter::New();
    filler->SetInput(regionGrower->GetOutput());
    filler->SetForegroundValue(1);
    filler->Update();

    itkResultImage = filler->GetOutput();
  }
  catch (const itk::ExceptionObject&)
  {
    return; // can't work
  }
  catch (...)
  {
    return;
  }
  mitk::CastToMitkImage(itkResultImage, filledRegionImage);
}

mitk::Image::Pointer mitk::CloseRegionTool::GenerateFillImage(const Image* workingSlice, Point3D seedPoint, mitk::Label::PixelType& seedLabelValue) const
{
  itk::Index<2> seedIndex;
  workingSlice->GetGeometry()->WorldToIndex(seedPoint, seedIndex);

  Image::Pointer fillImage;

  AccessFixedDimensionByItk_n(workingSlice, DoITKRegionClosing, 2, (fillImage, seedIndex, seedLabelValue));

  if (seedLabelValue == LabelSetImage::UnlabeledValue)
  {
    return nullptr;
  }

  return fillImage;
}

void mitk::CloseRegionTool::PrepareFilling(const Image* /*workingSlice*/, Point3D /*seedPoint*/)
{
  m_FillLabelValue = m_SeedLabelValue;
  m_MergeStyle = MultiLabelSegmentation::MergeStyle::Merge;
};
