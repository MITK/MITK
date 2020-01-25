/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkEraseRegionTool_h_Included
#define mitkEraseRegionTool_h_Included

#include "mitkSetRegionTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief Fill the inside of a contour with 1

    \sa SetRegionTool

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Finds the outer contour of a shape in 2D (possibly including holes) and sets all
    the inside pixels to 0 (erasing a segmentation).

    \warning Only to be instantiated by mitk::ToolManager.

    $Author$
  */
  class MITKSEGMENTATION_EXPORT EraseRegionTool : public SetRegionTool
  {
  public:
    mitkClassMacro(EraseRegionTool, SetRegionTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      const char **GetXPM() const override;
    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    const char *GetName() const override;

  protected:
    EraseRegionTool(); // purposely hidden
    ~EraseRegionTool() override;
  };

} // namespace

#endif
