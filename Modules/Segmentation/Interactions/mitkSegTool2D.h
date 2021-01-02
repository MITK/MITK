/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegTool2D_h_Included
#define mitkSegTool2D_h_Included

#include "mitkCommon.h"
#include "mitkImage.h"
#include "mitkTool.h"
#include <MitkSegmentationExports.h>

#include "mitkInteractionPositionEvent.h"

#include "mitkInteractionConst.h"
#include "mitkPlanePositionManager.h"
#include "mitkRestorePlanePositionOperation.h"

#include <mitkDiffSliceOperation.h>

namespace mitk
{
  class BaseRenderer;

  /**
    \brief Abstract base class for segmentation tools.

    \sa Tool

    \ingroup Interaction
    \ingroup ToolManagerEtAl

    Implements 2D segmentation specific helper methods, that might be of use to
    all kind of 2D segmentation tools. At the moment these are:
     - Determination of the slice where the user paints upon (DetermineAffectedImageSlice)
     - Projection of a 3D contour onto a 2D plane/slice

     SegTool2D tries to structure the interaction a bit. If you pass "PressMoveRelease" as the interaction type
     of your derived tool, you might implement the methods OnMousePressed, OnMouseMoved, and OnMouseReleased.
     Yes, your guess about when they are called is correct.

    \warning Only to be instantiated by mitk::ToolManager.

    $Author$
  */
  class MITKSEGMENTATION_EXPORT SegTool2D : public Tool
  {
  public:
    mitkClassMacro(SegTool2D, Tool);

    /**
      \brief Calculates for a given Image and PlaneGeometry, which slice of the image (in index corrdinates) is meant by
      the plane.

      \return false, if no slice direction seems right (e.g. rotated planes)
      \param image
      \param plane
      \param affectedDimension The image dimension, which is constant for all points in the plane, e.g. Axial --> 2
      \param affectedSlice The index of the image slice
    */
    static bool DetermineAffectedImageSlice(const Image *image,
                                            const PlaneGeometry *plane,
                                            int &affectedDimension,
                                            int &affectedSlice);

    /**
     * @brief Updates the surface interpolation by extracting the contour form the given slice.
     * @param slice the slice from which the contour should be extracted
     * @param workingImage the segmentation image
     * @param plane the plane in which the slice lies
     * @param detectIntersection if true the slice is eroded before contour extraction. If the slice is empty after the
     * erosion it is most
     *        likely an intersecting contour an will not be added to the SurfaceInterpolationController
     */
    static void UpdateSurfaceInterpolation(const Image *slice,
                                           const Image *workingImage,
                                           const PlaneGeometry *plane,
                                           bool detectIntersection);

    /**
    * \brief Extract the slice of an image that the user just scribbles on. The given component denotes the vector component of a dwi-image.
    *
    * \param positionEvent Event that specifies the plane that should be used to slice
    * \param image Image that should be sliced
    * \param timeStep TimeStep of the image that shold be sliced
    * \param component  The component to be extracted of a given multi-component image. -1 is the default parameter to denote an invalid component.
    *
    * \return 'nullptr' if SegTool2D is either unable to determine which slice was affected, or if there was some problem
    *         getting the image data at that position.
    */
    static Image::Pointer GetAffectedImageSliceAs2DImage(const InteractionPositionEvent* positionEvent, const Image* image, unsigned int component = 0);

    /**
    * \brief Extract the slice of an image cut by given plane. The given component denotes the vector component of a dwi-image.
    *
    * \param planeGeometry Geometry defining the slice that should be cut out.
    * \param image Image that should be sliced
    * \param timeStep TimeStep of the image that shold be sliced
    * \param component  The component to be extracted of a given multi-component image. -1 is the default parameter to denote an invalid component.
    *
    * \return 'nullptr' if SegTool2D is either unable to determine which slice was affected, or if there was some problem
    *         getting the image data at that position.
    */
    static Image::Pointer GetAffectedImageSliceAs2DImage(const PlaneGeometry* planeGeometry,
      const Image* image,
      TimeStepType timeStep,
      unsigned int component = 0);
    static Image::Pointer GetAffectedImageSliceAs2DImageByTimePoint(const PlaneGeometry* planeGeometry,
      const Image* image,
      TimePointType timePoint,
      unsigned int component = 0);


    static void WriteBackSegmentationResult(const DataNode* workingNode, const PlaneGeometry* planeGeometry, const Image* segmentationResult, TimeStepType timeStep);

    static void WriteSliceToVolume(Image* workingImage, const PlaneGeometry* planeGeometry, const Image* slice, TimeStepType timeStep, bool allowUndo);


    void SetShowMarkerNodes(bool);

    /**
     * \brief Enables or disables the 3D interpolation after writing back the 2D segmentation result, and defaults to
     * true.
     */
    void SetEnable3DInterpolation(bool);

    void Activated() override;
    void Deactivated() override;

    itkSetMacro(IsTimePointChangeAware, bool);
    itkGetMacro(IsTimePointChangeAware, bool);
    itkBooleanMacro(IsTimePointChangeAware);

  protected:
    SegTool2D();             // purposely hidden
    SegTool2D(const char *, const us::Module *interactorModule = nullptr); // purposely hidden
    ~SegTool2D() override;

    /**
     * @brief returns the segmentation node that should be modified by the tool.
     */
    mitk::DataNode* GetWorkingDataNode() const;
    mitk::Image* GetWorkingData() const;

    mitk::DataNode* GetReferenceDataNode() const;
    mitk::Image* GetReferenceData() const;

    /**
     * This function can be reimplemented by derived classes to react on changes of the current
     * time point. Default implementation does nothing.*/
    virtual void OnTimePointChanged();

    struct SliceInformation
    {
      mitk::Image::ConstPointer slice;
      const mitk::PlaneGeometry *plane;
      mitk::TimeStepType timestep;

      SliceInformation() {}
      SliceInformation(const mitk::Image *slice, const mitk::PlaneGeometry *plane, mitk::TimeStepType timestep)
      {
        this->slice = slice;
        this->plane = plane;
        this->timestep = timestep;
      }
    };

    /**
     * @brief Updates the surface interpolation by extracting the contour form the given slice.
     * @param sliceInfos vector of slice information instances from which the contours should be extracted
     * @param workingImage the segmentation image
     * @param detectIntersection if true the slice is eroded before contour extraction. If the slice is empty after the
     * erosion it is most
     *        likely an intersecting contour an will not be added to the SurfaceInterpolationController
     */
    static void UpdateSurfaceInterpolation(const std::vector<SliceInformation>& sliceInfos,
      const Image* workingImage,
      bool detectIntersection);


    /**
    * \brief Filters events that cannot be handle by 2D segmentation tools
    *
    * Current an event is discarded if it was not sent by a 2D renderwindow and if it is
    * not of type InteractionPositionEvent
    */
    bool FilterEvents(InteractionEvent *interactionEvent, DataNode *dataNode) override;

    /**
      \brief Extract the slice of the currently selected working image that the user just scribbles on.
      \return nullptr if SegTool2D is either unable to determine which slice was affected, or if there was some problem
      getting the image data at that position,
                   or just no working image is selected.
    */
    Image::Pointer GetAffectedWorkingSlice(const InteractionPositionEvent *) const;

    /**
      \brief Extract the slice of the currently selected reference image that the user just scribbles on.
      \return nullptr if SegTool2D is either unable to determine which slice was affected, or if there was some problem
      getting the image data at that position,
                   or just no reference image is selected.
    */
    Image::Pointer GetAffectedReferenceSlice(const InteractionPositionEvent *) const;
    /** Overload version that gets the reference slice passed on the passed plane geometry and timestep.*/
    Image::Pointer GetAffectedReferenceSlice(const PlaneGeometry* planeGeometry, TimeStepType timeStep) const;

    void WriteBackSegmentationResult(const InteractionPositionEvent *, const Image* segmentationResult);

    void WriteBackSegmentationResult(const PlaneGeometry *planeGeometry, const Image* segmentationResult, TimeStepType timeStep);

    void WriteBackSegmentationResults(const std::vector<SliceInformation> &sliceList, bool writeSliceToVolume = true);
    static void WriteBackSegmentationResults(const DataNode* workingNode, const std::vector<SliceInformation>& sliceList, bool writeSliceToVolume = true);

    void WritePreviewOnWorkingImage(
      Image *targetSlice, Image *sourceSlice, Image *workingImage, int paintingPixelValue, int timestep);

    static void WriteSliceToVolume(Image* workingImage, const SliceInformation &sliceInfo, bool allowUndo);

    /**
      \brief Adds a new node called Contourmarker to the datastorage which holds a mitk::PlanarFigure.
             By selecting this node the slicestack will be reoriented according to passed the PlanarFigure's Geometry
    */
    int AddContourmarker(const PlaneGeometry* planeGeometry, unsigned int sliceIndex);

    void InteractiveSegmentationBugMessage(const std::string &message) const;

    BaseRenderer *m_LastEventSender = nullptr;
    unsigned int m_LastEventSlice = 0;

    itkGetMacro(LastTimePointTriggered, TimePointType);

  private:
    /** Internal method that gets triggered as soon as the tool manager indicates a
     * time point change. If the time point has changed since last time and tool
     * is set to be time point change aware, OnTimePointChanged() will be called.*/
    void OnTimePointChangedInternal();

    static void  RemoveContourFromInterpolator(const SliceInformation& sliceInfo);

    // The prefix of the contourmarkername. Suffix is a consecutive number
    const std::string m_Contourmarkername;

    bool m_ShowMarkerNodes = false;
    static bool m_SurfaceInterpolationEnabled;

    bool m_IsTimePointChangeAware = true;

    TimePointType m_LastTimePointTriggered = 0.;
  };

} // namespace

#endif
