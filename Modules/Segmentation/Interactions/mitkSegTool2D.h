/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSegTool2D_h
#define mitkSegTool2D_h

#include <mitkCommon.h>
#include <mitkImage.h>
#include <mitkTool.h>
#include <MitkSegmentationExports.h>

#include <mitkInteractionPositionEvent.h>

#include <mitkInteractionConst.h>
#include <mitkPlanePositionManager.h>
#include <mitkRestorePlanePositionOperation.h>

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
    * \brief Extract the slice of an image that the user just scribbles on. The given component denotes the vector component of an vector image.
    *
    * \param positionEvent Event that specifies the plane that should be used to slice
    * \param image Image that should be sliced
    * \param component  The component to be extracted of a given multi-component image. -1 is the default parameter to denote an invalid component.
    *
    * \return 'nullptr' if SegTool2D is either unable to determine which slice was affected, or if there was some problem
    *         getting the image data at that position.
    */
    static Image::Pointer GetAffectedImageSliceAs2DImage(const InteractionPositionEvent* positionEvent, const Image* image, unsigned int component = 0);

    /**
    * \brief Extract the slice of an image cut by given plane. The given component denotes the vector component of a vector image.
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

    /** Convenience overloaded version that can be called for a given planeGeometry, slice image and time step.
     * Calls static WriteBackSegmentationResults*/
    static void WriteBackSegmentationResult(const DataNode* workingNode, const PlaneGeometry* planeGeometry, const Image* segmentationResult, TimeStepType timeStep);

    /** Convenience overloaded version that can be called for a given planeGeometry, slice image and time step.
     * For more details see protected WriteSliceToVolume version.*/
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
    DataNode* GetWorkingDataNode() const;
    Image* GetWorkingData() const;

    DataNode* GetReferenceDataNode() const;
    Image* GetReferenceData() const;

    /**
     * This function can be reimplemented by derived classes to react on changes of the current
     * time point. Default implementation does nothing.*/
    virtual void OnTimePointChanged();

    struct SliceInformation
    {
      mitk::Image::ConstPointer slice;
      const mitk::PlaneGeometry *plane = nullptr;
      mitk::TimeStepType timestep = 0;
      unsigned int slicePosition;

      SliceInformation() = default;
      SliceInformation(const mitk::Image* aSlice, const mitk::PlaneGeometry* aPlane, mitk::TimeStepType aTimestep);
    };

    /**
     * @brief Updates the surface interpolation by extracting the contour form the given slice.
     * @param sliceInfos vector of slice information instances from which the contours should be extracted
     * @param workingImage the segmentation image
     * @param detectIntersection if true the slice is eroded before contour extraction. If the slice is empty after the
     * @param activeLayerID The layer ID of the active label.
     * @param activeLabelValue The label value of the active label.
     * erosion it is most
     *        likely an intersecting contour an will not be added to the SurfaceInterpolationController
     */
    static void UpdateSurfaceInterpolation(const std::vector<SliceInformation>& sliceInfos,
      const Image* workingImage,
      bool detectIntersection,
      unsigned int activeLayerID,
      mitk::Label::PixelType activeLabelValue);


    /**
    * \brief Filters events that cannot be handled by 2D segmentation tools
    *
    * Currently an event is discarded if it was not sent by a 2D renderwindow and if it is
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

    /** Convenience version that can be called for a given event (which is used to deduce timepoint and plane) and a slice image.
     * Calls non static WriteBackSegmentationResults*/
    void WriteBackSegmentationResult(const InteractionPositionEvent *, const Image* segmentationResult);

    /** Convenience version that can be called for a given planeGeometry, slice image and time step.
     * Calls non static WriteBackSegmentationResults*/
    void WriteBackSegmentationResult(const PlaneGeometry *planeGeometry, const Image* segmentationResult, TimeStepType timeStep);

    /** Overloaded version that calls the static version and also adds the contour markers.
     * @remark If the sliceList is empty, this function does nothing.*/
    void WriteBackSegmentationResults(const std::vector<SliceInformation> &sliceList, bool writeSliceToVolume = true);

    /** \brief Writes all provided source slices into the data of the passed workingNode.
     * The function does the following: 1) for every passed slice write it to workingNode (and generate and undo/redo step);
     * 2) update the surface interpolation and 3) marke the node as modified.
     * @param workingNode Pointer to the node that contains the working image.
     * @param sliceList Vector of all slices that should be written into the workingNode. If the list is
     * empty, the function call does nothing.
     * @param writeSliceToVolume If set to false the write operation (WriteSliceToVolume will be skipped)
     * and only the surface interpolation will be updated.
     * @pre workingNode must point to a valid instance and contain an image instance as data.*/
    static void WriteBackSegmentationResults(const DataNode* workingNode, const std::vector<SliceInformation>& sliceList, bool writeSliceToVolume = true);

    /** Writes the provided source slice into the target slice with the given pixel value.
     * If passed workingImage is a LabelSetImage the label set rules will be applied when
     * writing all non zero source pixels into the target slice (e.g. locked lables will not be touched)
     * with the given paintingPixelValue.
     * @param targetSlice Pointer to the slice that should be filled with the content of the sourceSlice.
     * @param sourceSlice Pointer to the slice that is the source/preview every pixel will be (tried to be) transfered .
     * @param workingImage Will be used to check if LabeSetImageRules have to be applied and the label set state.
     * @param paintingPixelValue Value that will be used to paint onto target slice.
     * @pre targetSlice must point to a valid instance.
     * @pre sourceSlice must point to a valid instance.
     * @pre workingImage must point to a valid instance.*/
    static void WritePreviewOnWorkingImage(
      Image *targetSlice, const Image *sourceSlice, const Image *workingImage, int paintingPixelValue);

    /** Writes a provided slice into the passed working image. The content of working image that is covered
    * by the slice will be completly overwritten. If asked for it also generates the needed
    * undo/redo steps.
    * @param workingImage Pointer to the image that is the target of the write operation.
    * @param sliceInfo SliceInfo instance that containes the slice image, the defining plane geometry and time step.
    * @param allowUndo Indicates if undo/redo operations should be registered for the write operation
    * performed by this call. true: undo/redo will be generated; false: no undo/redo will be generated, so
    * this operation cannot be revoked by the user.
    * @pre workingImage must point to a valid instance.*/
    static void WriteSliceToVolume(Image* workingImage, const SliceInformation &sliceInfo, bool allowUndo);

    /**
      \brief Adds a new node called Contourmarker to the datastorage which holds a mitk::PlanarFigure.
      By selecting this node the slicestack will be reoriented according to the passed
      PlanarFigure's Geometry
    */
    int AddContourmarker(const PlaneGeometry* planeGeometry, unsigned int sliceIndex);

    void InteractiveSegmentationBugMessage(const std::string &message) const;

    /** Helper function to check if a position events points to a point inside the boundingbox of a passed
     data instance.*/
    static bool IsPositionEventInsideImageRegion(InteractionPositionEvent* positionEvent, const BaseData* data);

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
