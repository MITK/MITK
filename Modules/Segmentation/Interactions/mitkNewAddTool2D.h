/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkNewAddTool2D_h
#define mitkNewAddTool2D_h

#include "mitkEditableContourTool.h"
#include "mitkContourTool.h"
#include "mitkContourModelInteractor.h"

namespace mitk
{
  /**
    \brief A 2D segmentation tool to draw polygon structures.

    The contour between the last point and the current mouse position is
    computed by searching the shortest path according to specific features of
    the image. The contour thus tends to snap to the boundary of objects.

    The tool always assumes that unconfirmed contours are always defined for the
    current time point. So the time step in which the contours will be stored as
    segmentations will be determined when the contours got confirmed. Then they
    will be transfered to the slices of the currently selected time step.

    \sa SegTool2D

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT NewAddTool2D : public EditableContourTool
  {
  public:
    mitkClassMacro(NewAddTool2D, SegTool2D);
    itkFactorylessNewMacro(Self);

    us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;
    const char *GetName() const override;
    const char **GetXPM() const override;

  protected:
    NewAddTool2D();
    ~NewAddTool2D() override;

    void ConnectActionsAndFunctions() override;

  private:
    /// \brief Initialize tool.
    void OnInitContour(StateMachineAction *, InteractionEvent *interactionEvent) override;

    /// \brief Add a control point and finish current segment.
    void OnAddPoint(StateMachineAction *, InteractionEvent *interactionEvent) override;

    /// \brief Draw a contour according to the mouse movement when mouse button is pressed and mouse is moved.
    void OnDrawing(StateMachineAction *, InteractionEvent *interactionEvent) override;

    void OnEndDrawing(StateMachineAction *, InteractionEvent *interactionEvent) override;

    /// \brief Actual contour computation.
    void OnMouseMoved(StateMachineAction *, InteractionEvent *interactionEvent) override;

    /// \brief Finish contour tool.
    void OnFinish(StateMachineAction *, InteractionEvent *interactionEvent) override;

    /// \brief Finish contour interaction.
    void FinishTool() override;

    void EnableContourInteraction(bool on) override;

    void ReleaseInteractors() override;
  
    mitk::ContourModelInteractor::Pointer m_ContourInteractor;
    std::vector<mitk::ContourModelInteractor::Pointer> m_ContourInteractors;
  };
}

#endif
