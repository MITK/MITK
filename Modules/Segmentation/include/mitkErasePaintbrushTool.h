/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkErasePaintbrushTool_h
#define mitkErasePaintbrushTool_h

#include <mitkPaintbrushTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief Paintbrush erasing tool for interactive segmentation.

    \sa FeedbackContourTool
    \sa ExtractImageFilter
    \sa PaintbrushTool

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Simple paintbrush erasing tool. Right now there are only circular pens of varying size.

    This class configures the painting pixel value to 0 (background/erase) for
    the superclass PaintbrushTool. Holding CTRL temporarily switches to draw mode.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT ErasePaintbrushTool : public PaintbrushTool
  {
  public:
    mitkClassMacro(ErasePaintbrushTool, PaintbrushTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Return the cursor icon resource for this tool. */
    us::ModuleResource GetCursorIconResource() const override;

    /** \brief Return the toolbar icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Return the human-readable name of this tool ("Wipe"). */
    const char *GetName() const override;

  protected:
    ErasePaintbrushTool(); // purposely hidden
    ~ErasePaintbrushTool() override;

    /** \brief Toggle between erase and draw mode and swap the cursor icon accordingly. */
    void OnInvertLogic(StateMachineAction* action, InteractionEvent* event) override;
  };

} // namespace

#endif
