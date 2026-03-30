/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLiveWireTool2D_h
#define mitkLiveWireTool2D_h

#include <mitkEditableContourTool.h>
#include <mitkContourModelLiveWireInteractor.h>

namespace mitk
{
  /**
    \brief A 2D segmentation tool based on a LiveWire approach.

    The contour between the last point and the current mouse position is
    computed by searching the shortest path according to specific features of
    the image. The contour thus tends to snap to the boundary of objects.

    The tool always assumes that unconfirmed contours are always defined for the
    current time point. So the time step in which the contours will be stored as
    segmentations will be determined when the contours got confirmed. Then they
    will be transferred to the slices of the currently selected time step.
    Changing the time point/time step while tool is active will updated the working
    slice the live wire filter. So the behavior of the active live wire contour is
    always WYSIWYG (What you see is what you get).

    \sa SegTool2D
    \sa ImageLiveWireContourModelFilter

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    \warning Only to be instantiated by mitk::ToolManager.
  */
  class MITKSEGMENTATION_EXPORT LiveWireTool2D : public EditableContourTool
  {
  public:
    mitkClassMacro(LiveWireTool2D, EditableContourTool);
    itkFactorylessNewMacro(Self);

    /** \brief Return the cursor icon resource for this tool. */
    us::ModuleResource GetCursorIconResource() const override;

    /** \brief Return the toolbar icon resource for this tool. */
    us::ModuleResource GetIconResource() const override;

    /** \brief Return the human-readable name of this tool ("Live Wire"). */
    const char *GetName() const override;

  protected:
    LiveWireTool2D();
    ~LiveWireTool2D() override;

    /** \brief Wire state machine actions to the corresponding member functions. */
    void ConnectActionsAndFunctions() override;

    /** \brief Recompute the LiveWire contour between start and end points. */
    void UpdateLiveWireContour();

    /** \brief Update the working slice when the time point changes. */
    void OnTimePointChanged() override;

    /** \brief Prepare the initial contour anchor, snapping to the highest gradient magnitude.
      \return The adjusted anchor point.
    */
    mitk::Point3D PrepareInitContour(const mitk::Point3D& clickedPoint) override;

    /** \brief Finalize the preview contour segment for the given click position. */
    virtual void FinalizePreviewContour(const Point3D& clickedPoint) override;

    /** \brief Initialize the preview contour for a new LiveWire segment starting at the clicked point. */
    virtual void InitializePreviewContour(const Point3D& clickedPoint) override;

    /** \brief Update the preview contour as the mouse moves to a new position. */
    virtual void UpdatePreviewContour(const Point3D& clickedPoint) override;

  private:

    /** \brief Handle mouse movement without updating the dynamic cost map. */
    void OnMouseMoveNoDynamicCosts(StateMachineAction *, InteractionEvent *interactionEvent);

    /** \brief Finalize the contour and write it back as a segmentation result. */
    void FinishTool() override;

    template <typename TPixel, unsigned int VImageDimension>
    void FindHighestGradientMagnitudeByITK(itk::Image<TPixel, VImageDimension> *inputImage,
                                           itk::Index<3> &index,
                                           itk::Index<3> &returnIndex);

    mitk::ContourModelLiveWireInteractor::Pointer m_ContourInteractor;

    mitk::ImageLiveWireContourModelFilter::Pointer m_LiveWireFilter;

    bool m_CreateAndUseDynamicCosts;
  };
}

#endif
