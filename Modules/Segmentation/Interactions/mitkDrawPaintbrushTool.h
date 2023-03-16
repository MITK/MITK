/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDrawPaintbrushTool_h
#define mitkDrawPaintbrushTool_h

#include "mitkPaintbrushTool.h"
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
   \brief Paintbrush tool for InteractiveSegmentation

   \sa FeedbackContourTool
   \sa ExtractImageFilter

   \ingroup Interaction
   \ingroup ToolManagerEtAl

   Simple paintbrush drawing tool. Right now there are only circular pens of varying size.

   This class specified only the drawing "color" for the super class PaintbrushTool.

   \warning Only to be instantiated by mitk::ToolManager.
   $Author: maleike $
 */
  class MITKSEGMENTATION_EXPORT DrawPaintbrushTool : public PaintbrushTool
  {
  public:
    mitkClassMacro(DrawPaintbrushTool, PaintbrushTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      const char **GetXPM() const override;
    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    const char *GetName() const override;

  protected:
    DrawPaintbrushTool(); // purposely hidden
    ~DrawPaintbrushTool() override;
  };

} // namespace

#endif
