/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDrawPaintbrushTool_h
#define mitkDrawPaintbrushTool_h

#include <mitkPaintbrushTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief Paintbrush drawing tool for interactive segmentation.

    \sa FeedbackContourTool
    \sa ExtractSliceFilter
    \sa PaintbrushTool

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Simple paintbrush drawing tool. Right now there are only circular pens of varying size.

    This class configures the painting pixel value to 1 (foreground) for
    the superclass PaintbrushTool. Holding CTRL temporarily switches to erase mode.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT DrawPaintbrushTool : public PaintbrushTool
  {
  public:
    mitkClassMacro(DrawPaintbrushTool, PaintbrushTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Return the cursor icon resource for this tool. */
    us::ModuleResource GetCursorIconResource() const override;

    /** \brief Return the toolbar icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Return the human-readable name of this tool ("Paint"). */
    const char *GetName() const override;

  protected:
    DrawPaintbrushTool(); // purposely hidden
    ~DrawPaintbrushTool() override;

    /** \brief Toggle between draw and erase mode and swap the cursor icon accordingly. */
    void OnInvertLogic(StateMachineAction* action, InteractionEvent* event) override;
  };

} // namespace

#endif
