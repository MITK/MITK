/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEraseRegionTool_h
#define mitkEraseRegionTool_h

#include "mitkFillRegionBaseTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief Erase the inside of a contour by
           filling the inside of a contour with the background pixel value.

    \sa SetRegionTool

    \ingroup Interactions

    Finds the outer contour of a shape in 2D (possibly including single patches) and sets all
    the pixels inside to the background pixel value (erasing a segmentation).
    If clicked on the background, the outer contour might contain the whole image and thus
    fill the whole image with the background pixel value.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT EraseRegionTool : public FillRegionBaseTool
  {
  public:
    mitkClassMacro(EraseRegionTool, FillRegionBaseTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char **GetXPM() const override;
    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    const char *GetName() const override;

  protected:
    EraseRegionTool() = default; // purposely hidden
    ~EraseRegionTool() = default;

    Image::Pointer GenerateFillImage(const Image* workingSlice, Point3D seedPoint, mitk::Label::PixelType& seedLabelValue) const override;
    void PrepareFilling(const Image* workingSlice, Point3D seedPoint) override;
  };

} // namespace

#endif
