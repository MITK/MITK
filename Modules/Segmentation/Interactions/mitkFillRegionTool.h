/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFillRegionTool_h
#define mitkFillRegionTool_h

#include "mitkFillRegionBaseTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief Fill the inside of a contour with the foreground pixel value.

    \sa SetRegionTool

    \ingroup Interactions

    Finds the outer contour of a shape in 2D (possibly including holes) and sets all
    the pixels inside to the foreground pixel value (filling holes in a segmentation).
    If clicked on the background, the outer contour might contain the whole image and thus
    fill the whole image with the foreground pixel value.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT FillRegionTool : public FillRegionBaseTool
  {
  public:
    mitkClassMacro(FillRegionTool, FillRegionBaseTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    const char **GetXPM() const override;
    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    const char *GetName() const override;

  protected:
    void PrepareFilling(const Image* workingSlice, Point3D seedPoint) override;

    FillRegionTool() = default; // purposely hidden
    ~FillRegionTool() = default;
  };

} // namespace

#endif
