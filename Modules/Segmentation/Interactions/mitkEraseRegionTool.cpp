/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkEraseRegionTool.h"

#include "mitkEraseRegionTool.xpm"
#include <mitkImagePixelReadAccessor.h>
#include <mitkImageGenerator.h>
#include <mitkImageAccessByItk.h>

// us
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleContext.h>
#include <usModuleResource.h>

namespace mitk
{
  MITK_TOOL_MACRO(MITKSEGMENTATION_EXPORT, EraseRegionTool, "Erase tool");
}

const char **mitk::EraseRegionTool::GetXPM() const
{
  return mitkEraseRegionTool_xpm;
}

us::ModuleResource mitk::EraseRegionTool::GetIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Erase.svg");
  return resource;
}

us::ModuleResource mitk::EraseRegionTool::GetCursorIconResource() const
{
  us::Module *module = us::GetModuleContext()->GetModule();
  us::ModuleResource resource = module->GetResource("Erase_Cursor.svg");
  return resource;
}

const char *mitk::EraseRegionTool::GetName() const
{
  return "Erase";
}

template <typename TPixel, unsigned int VImageDimension>
void DoFillImage(itk::Image<TPixel, VImageDimension>* image)
{
  image->FillBuffer(1);
};

mitk::Image::Pointer mitk::EraseRegionTool::GenerateFillImage(const Image* workingSlice, Point3D seedPoint, mitk::Label::PixelType& seedLabelValue) const
{
  itk::Index<2> seedIndex;
  workingSlice->GetGeometry()->WorldToIndex(seedPoint, seedIndex);

  using AccessorType = ImagePixelReadAccessor<Label::PixelType, 2>;
  AccessorType accessor(workingSlice);
  seedLabelValue = accessor.GetPixelByIndex(seedIndex);
  Image::Pointer fillImage;

  if ( seedLabelValue == LabelSetImage::UnlabeledValue)
  { //clicked on background remove everything which is not locked.
    fillImage = workingSlice->Clone();
    AccessByItk(fillImage, DoFillImage);
  }
  else
  {
    fillImage = Superclass::GenerateFillImage(workingSlice, seedPoint, seedLabelValue);
  }

  return fillImage;
}

void mitk::EraseRegionTool::PrepareFilling(const Image* /*workingSlice*/, Point3D /*seedPoint*/)
{
  m_FillLabelValue = LabelSetImage::UnlabeledValue;
};
