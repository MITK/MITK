/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkRegionGrowingTool_h_Included
#define mitkRegionGrowingTool_h_Included

#include "mitkFeedbackContourTool.h"
#include "mitkLegacyAdaptors.h"
#include <MitkSegmentationExports.h>
#include <array>

struct mitkIpPicDescriptor;

namespace us {
class ModuleResource;
}

namespace mitk
{

/**
  \brief A slice based region growing tool.

  \sa FeedbackContourTool

  \ingroup Interaction
  \ingroup ToolManagerEtAl

  When the user presses the mouse button, RegionGrowingTool will use the gray values at that position
  to initialize a region growing algorithm (in the affected 2D slice).

  By moving the mouse up and down while the button is still pressed, the user can widen or narrow the threshold window, i.e. select more or less within the desired region.
  The current result of region growing will always be shown as a contour to the user.

  After releasing the button, the current result of the region growing algorithm will be written to the
  working image of this tool's ToolManager.

  If the first click is <i>inside</i> a segmentation, nothing will happen (other behaviour, for example removal of a region, can be implemented via OnMousePressedInside()).

  \warning Only to be instantiated by mitk::ToolManager.

  $Author$
*/
class MITKSEGMENTATION_EXPORT RegionGrowingTool : public FeedbackContourTool
{
  public:

    mitkClassMacro(RegionGrowingTool, FeedbackContourTool);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual const char** GetXPM() const override;
    virtual us::ModuleResource GetCursorIconResource() const override;
    us::ModuleResource GetIconResource() const override;

    virtual const char* GetName() const override;

  protected:

    RegionGrowingTool(); // purposely hidden
    virtual ~RegionGrowingTool();

    void ConnectActionsAndFunctions() override;

    virtual void Activated() override;
    virtual void Deactivated() override;

    /**
     * @brief OnMousePressed is called when the user clicks.
     * Calls either OnMousePressedInside() or OnMousePressedOutside().
     */
    virtual void OnMousePressed (StateMachineAction*, InteractionEvent* interactionEvent);

    /**
     * @brief OnMousePressedInside can be used to implement behaviour when the user clicks inside a segmentation.
     */
    virtual void OnMousePressedInside();

    /**
     * @brief OnMousePressedOutside is called when the user clicks outside of the segmented area.
     * Grows a region.
     */
    virtual void OnMousePressedOutside (StateMachineAction*, InteractionEvent* interactionEvent);

    /**
     * @brief OnMouseMoved is called when the user moves the mouse with the left mouse button pressed.
     * Adjusts the thresholds.
     * Up: Increase upper threshold, decrease lower threshold.
     * Down: Decrease upper threshold, increase lower threshold.
     * Right: Increase both thresholds.
     * Left: Decrease both thresholds.
     */
    virtual void OnMouseMoved (StateMachineAction*, InteractionEvent* interactionEvent);

    /**
     * @brief OnMouseReleased converts the feedback contour to a segmentation.
     */
    virtual void OnMouseReleased(StateMachineAction*, InteractionEvent* interactionEvent);

    /**
     * @brief Template to calculate average pixel value around index using a square/cube with radius neighborhood.
     * Example: 1 = 3x3 pixels, 2 = 5x5 pixels, etc.
     */
    template <typename TPixel, unsigned int imageDimension>
    void GetNeighborhoodAverage(itk::Image<TPixel, imageDimension>* itkImage, itk::Index<imageDimension> index, ScalarType* result, unsigned int neighborhood=1);

    /**
     * @brief Template to check whether index is inside already segmented area.
     */
    template <typename TPixel, unsigned int imageDimension>
    void IsInsideSegmentation(itk::Image<TPixel, imageDimension>* itkImage, itk::Index<imageDimension> index, bool* result);

    /**
     * @brief Template that calls an ITK filter to do the region growing.
     */
    template<typename TPixel, unsigned int imageDimension>
    void StartRegionGrowing(itk::Image<TPixel, imageDimension>* itkImage, itk::Index<imageDimension> seedPoint, std::array<ScalarType, 2> thresholds, mitk::Image::Pointer& outputImage);

    Image::Pointer m_ReferenceSlice;
    Image::Pointer m_WorkingSlice;

    ScalarType m_SeedValue;
    itk::Index<3> m_SeedPoint;
    std::array<ScalarType, 2> m_Thresholds;
    std::array<ScalarType, 2> m_InitialThresholds;
    Point2I m_LastScreenPosition;
    int m_ScreenYDifference;
    int m_ScreenXDifference;

  private:

    ScalarType m_VisibleWindow;
    ScalarType m_MouseDistanceScaleFactor;
    int m_PaintingPixelValue;
    bool m_FillFeedbackContour;
    int m_ConnectedComponentValue;
};

} // namespace

#endif


