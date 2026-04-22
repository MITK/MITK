/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkFeedbackContourTool_h
#define mitkFeedbackContourTool_h

#include <mitkCommon.h>
#include <mitkContourModelUtils.h>
#include <mitkImage.h>
#include <mitkSegTool2D.h>
#include <MitkSegmentationExports.h>

#include <mitkDataNode.h>

#include <mitkImageCast.h>

namespace mitk
{
  /**
    \brief Base class for tools that use a contour for feedback

    \sa Tool
    \sa ContourModel

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Implements helper methods, that might be of use to all kind of 2D segmentation tools that use a contour for user
    feedback.
     - Providing a feedback contour that might be added or removed from the visible scene (SetFeedbackContourVisible).
     - Filling of a contour into a 2D slice

     These helper methods are actually implemented in ContourModelUtils now. FeedbackContourTool only forwards such requests.

    \warning Only to be instantiated by mitk::ToolManager.

    $Author: nolden $
  */
  class MITKSEGMENTATION_EXPORT FeedbackContourTool : public SegTool2D
  {
  public:
    mitkClassMacro(FeedbackContourTool, SegTool2D);

  protected:
    FeedbackContourTool(const char *); // purposely hidden
    ~FeedbackContourTool() override;

    const ContourModel *GetFeedbackContour() const;

    /** (Re)initialize the feedback contour by creating a new instance.
     * It is assured that the new instance as the same time geometry than
     * the working image.*/
    void InitializeFeedbackContour(bool isClosed);

    /** Clears the current time step of the feedback contour and resets its closed state.*/
    void ClearsCurrentFeedbackContour(bool isClosed);

    /** Updates the feedback contour of the currently selected time point. The update will be done
     * by clearing all existing vertices at the current time point and copying the vertics of the
     * source model at the specified source time step.*/
    void UpdateCurrentFeedbackContour(const ContourModel* sourceModel, TimeStepType sourceTimeStep = 0);
    /** Updates the feedback contour at the time step specified by feedbackTimeStep. The update will be done
     * by clearing all existing vertices at feedbackTimeStep and copying the vertics of the
     * source model at the specified source time step.*/
    void UpdateFeedbackContour(const ContourModel* sourceModel, TimeStepType feedbackTimeStep, TimeStepType sourceTimeStep = 0);

    /** Adds a vertex to the feedback contour for the current time point. */
    void AddVertexToCurrentFeedbackContour(const Point3D& point);

    /** Adds a vertex to the feedback contour for the passed time step. If time step is invalid, nothing will be added.*/
    void AddVertexToFeedbackContour(const Point3D& point, TimeStepType feedbackTimeStep);

    void SetFeedbackContourVisible(bool);

    /// Provide values from 0.0 (black) to 1.0 (full color)
    void SetFeedbackContourColor(float r, float g, float b);
    void SetFeedbackContourColor(const Color& color);
    void SetFeedbackContourColorDefault();
    void SetFeedbackContourWidth(float width);

    void Deactivated() override;

    void Activated() override;

    /**
      \brief Projects a contour onto an image point by point. Converts from world to index coordinates.

      \param slice
      \param contourIn3D
    */
    ContourModel::Pointer ProjectContourTo2DSlice(const Image *slice,
                                                  const ContourModel *contourIn3D);

    /**
      \brief Projects a slice index coordinates of a contour back into world coordinates.

      \param sliceGeometry
      \param contourIn2D
    */
    ContourModel::Pointer BackProjectContourFrom2DSlice(const BaseGeometry *sliceGeometry,
                                                        const ContourModel *contourIn2D);

    /**
     * \brief Checks preconditions and writes the feedback contour as a segmentation result.
     *
     * Steps: (1) gets the contour at the time point from positionEvent;
     * (2) gets the affected working slice; (3) projects the contour onto the working slice
     * and fills it; (4) writes the slice back via SegTool2D::WriteBackSegmentationResult().
     *
     * \param[in] positionEvent The position event that triggers the segmentation update.
     * \param[in] labelValue The label value to be written.
     * \param[in] addMode If false, the contour area is set to background (erased).
     *            If true, the label is added to the slice.
     * \param[in] setInvisibleAfterSuccess If true, hides the feedback contour after successful write.
     */
    void WriteBackFeedbackContourAsSegmentationResult(const InteractionPositionEvent* positionEvent, MultiLabelSegmentation::LabelValueType labelValue, bool addMode, bool setInvisibleAfterSuccess = true);

    /**
     * \brief Generates an updated slice image with the contour applied.
     *
     * \param[in] seg The segmentation from which to extract the template slice.
     * \param[in] sliceGeometry Defines which slice to extract and update.
     * \param[in] contour The contour to rasterize into the slice.
     * \param[in] labelValue The label value to write.
     * \param[in] timePoint The time point for slice extraction.
     * \param[in] addMode If false, the contour area is set to background. If true, the label is added.
     * \return The updated slice image.
     */
    mitk::Image::Pointer GenerateSliceWithContourUpdate(const MultiLabelSegmentation* seg, const PlaneGeometry* sliceGeometry,
      const ContourModel* contour, MultiLabelSegmentation::LabelValueType labelValue, TimePointType timePoint, bool addMode);

  private:
    ContourModel::Pointer m_FeedbackContour;
    DataNode::Pointer m_FeedbackContourNode;
    bool m_FeedbackContourVisible;
  };

} // namespace

#endif
