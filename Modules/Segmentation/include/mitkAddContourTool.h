/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAddContourTool_h
#define mitkAddContourTool_h

#include <mitkContourTool.h>
#include <MitkSegmentationExports.h>

namespace us
{
  class ModuleResource;
}

namespace mitk
{
  /**
    \brief Fill the inside of a contour with the foreground pixel value.

    \sa ContourTool

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Fills a visible contour (from FeedbackContourTool) during mouse dragging. When the mouse button
    is released, AddContourTool tries to extract a slice from the working image and fill in
    the (filled) contour as a binary image. All inside pixels are set to 1.

    While holding the CTRL key, the contour changes color and the pixels on the inside would be
    filled with 0.

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT AddContourTool : public ContourTool
  {
  public:
    mitkClassMacro(AddContourTool, ContourTool);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Return the cursor icon resource for this tool. */
    us::ModuleResource GetCursorIconResource() const override;

    /** \brief Return the toolbar icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Return the human-readable name of this tool ("Add"). */
    const char *GetName() const override;

  protected:
    AddContourTool(); // purposely hidden
    ~AddContourTool() override;

    /** \brief Toggle painting pixel value and swap cursor icon when CTRL is pressed. */
    void OnInvertLogic(StateMachineAction* action, InteractionEvent* event) override;
  };

} // namespace

#endif
