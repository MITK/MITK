/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceInterpolationController_h
#define mitkSurfaceInterpolationController_h

#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkLabel.h>
#include <mitkSurface.h>

#include <MitkSurfaceInterpolationExports.h>

namespace mitk
{
  class ComputeContourSetNormalsFilter;
  class CreateDistanceImageFromSurfaceFilter;
  class LabelSetImage;
  class ReduceContourSetFilter;

  class MITKSURFACEINTERPOLATION_EXPORT SurfaceInterpolationController : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SurfaceInterpolationController, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);
    itkGetMacro(DistanceImageSpacing, double);

    struct MITKSURFACEINTERPOLATION_EXPORT ContourPositionInformation
    {
      int Pos;
      unsigned int SliceIndex;
      Surface::Pointer Contour;
      Vector3D ContourNormal;
      Point3D ContourPoint;
      mitk::PlaneGeometry* Plane;
      mitk::Label::PixelType LabelValue;
      unsigned int LayerValue;
      size_t TimeStep;

      ContourPositionInformation()
        : Pos(-1),
          SliceIndex(0),
          Plane(nullptr),
          LabelValue(std::numeric_limits<mitk::Label::PixelType>::max()),
          LayerValue(std::numeric_limits<unsigned int>::max()),
          TimeStep(std::numeric_limits<size_t>::max())
      {
      }
    };

    typedef std::vector<ContourPositionInformation> ContourPositionInformationList;
    typedef std::vector<ContourPositionInformationList> ContourPositionInformationVec2D;

    //  first index is the current time step. second index is the layerID. third index is the contour index.
    typedef std::vector<ContourPositionInformationVec2D> ContourPositionInformationVec3D;

    typedef std::map<mitk::Image *, ContourPositionInformationVec3D> ContourListMap;
    typedef std::map<mitk::Image *, ContourPositionInformationVec2D> ContourContainer;

    static SurfaceInterpolationController *GetInstance();

    void SetCurrentTimePoint(TimePointType tp)
    {
      if (m_CurrentTimePoint != tp)
      {
        m_CurrentTimePoint = tp;

        if (m_SelectedSegmentation)
        {
          this->ReinitializeInterpolation();
        }
      }
    };

    TimePointType GetCurrentTimePoint() const { return m_CurrentTimePoint; };

    /**
     * @brief Adds a new extracted contour to the list
     * @param newContour the contour to be added. If a contour at that position
     *        already exists the related contour will be updated
     */
    void AddNewContour(Surface::Pointer newContour);

    /**
     * @brief Adds new extracted contours to the list. If one or more contours at a given position
     *        already exist they will be updated respectively
     */
    void AddNewContours(const std::vector<Surface::Pointer>& newContours, std::vector<const mitk::PlaneGeometry*>& contourPlanes, bool reinitializeAction = false);

    /**
    * @brief Returns the contour for a given plane for the current selected segmenation
    * @param contourInfo the contour which should be returned
    * @return the contour as an mitk::Surface. If no contour is available at the give position nullptr is returned
    */
    const mitk::Surface *GetContour(const ContourPositionInformation& contourInfo);

    /**
     * @brief Computes an interior point of the input contour. It's used to detect merge and erase operations.
     *
     * @param contour Contour for which to compute the contour
     * @param labelSetImage LabelSetImage used input to check contour Label.
     * @return mitk::Point3D 3D Interior point of the contour returned.
     */
    mitk::Point3D ComputeInteriorPointOfContour(const ContourPositionInformation& contour,
                                                 mitk::LabelSetImage * labelSetImage);

    /**
     * @brief Make the surface interpolator responsive to the segmentation image by subscribing to events from the image.
     *
     */
    void AddLabelSetConnection();

    /**
     * @brief Make the surface interpolator responsive to the segmentation image by stopping subscription to events from the image.
     *
     */
    void RemoveLabelSetConnection();

    void RemoveLabelSetConnection(mitk::LabelSetImage* labelSetImage, unsigned int layerID);


    /**
     * @brief Resets the pipeline for interpolation. The various filters used are reset.
     *
     */
    void ReinitializeInterpolation();

    void RemoveObservers();

    void AddLabelSetConnection(unsigned int layerID);

    void UnsetSelectedImage()
    {
      m_SelectedSegmentation = nullptr;
    }

    /**
     * @brief Returns the number of layers in the current segmentation image.
     *
     */
    unsigned int GetNumberOfLayersInCurrentSegmentation() const;

    /**
     * @brief Set the number of layers in the current segmentation image.
     *
     */
    void SetNumberOfLayersInCurrentSegmentation(unsigned int);

    /**
     * @brief Function that does the data management when a layer is removed.
     *
     */
    void OnRemoveLayer();

    /**
     * @brief  Function that does the data management when a layer is added.
     *
     */
    void OnAddLayer();

    /**
    * @brief Returns the number of available contours for the current selected segmentation
    * @return the number of contours
    */
    unsigned int GetNumberOfContours();

    /**
     * @brief Performs the interpolation.
     *
     */
    void Interpolate();

    /**
     * @brief Get the Result of the interpolation operation.
     *
     * @return mitk::Surface::Pointer
     */
    mitk::Surface::Pointer GetInterpolationResult();

    /**
     * @brief Sets the minimum spacing of the current selected segmentation
     * This is needed since the contour points we reduced before they are used to interpolate the surface.
     *
     * @param minSpacing Paramter to set
     */
    void SetMinSpacing(double minSpacing);

    /**
     * @brief Sets the minimum spacing of the current selected segmentation
     * This is needed since the contour points we reduced before they are used to interpolate the surface
     * @param maxSpacing Set the max Spacing for interpolation
     */
    void SetMaxSpacing(double maxSpacing);

    /**
     * Sets the volume i.e. the number of pixels that the distance image should have
     * By evaluation we found out that 50.000 pixel delivers a good result
     */
    void SetDistanceImageVolume(unsigned int distImageVolume);

    /**
     * @brief Get the current selected segmentation for which the interpolation is performed
     * @return the current segmentation image
     */
    mitk::Image::Pointer GetCurrentSegmentation();

    Surface *GetContoursAsSurface();

    void SetDataStorage(DataStorage::Pointer ds);

    /**
     * Sets the current list of contourpoints which is used for the surface interpolation
     * @param segmentation The current selected segmentation
     * \deprecatedSince{2014_03}
     */
    DEPRECATED(void SetCurrentSegmentationInterpolationList(mitk::Image::Pointer segmentation));

    /**
     * Sets the current list of contourpoints which is used for the surface interpolation
     * @param currentSegmentationImage The current selected segmentation
     */
    void SetCurrentInterpolationSession(mitk::Image::Pointer currentSegmentationImage);

    /**
     * Removes the segmentation and all its contours from the list
     * @param segmentation The segmentation to be removed
     * \deprecatedSince{2014_03}
     */
    DEPRECATED(void RemoveSegmentationFromContourList(mitk::Image *segmentation));

    /**
     * @brief Remove interpolation session
     * @param segmentationImage the session to be removed
     */
    void RemoveInterpolationSession(mitk::Image::Pointer segmentationImage);

    /**
     * Replaces the current interpolation session with a new one. All contours form the old
     * session will be applied to the new session. This only works if the two images have the
     * geometry
     * @param oldSession the session which should be replaced
     * @param newSession the new session which replaces the old one
     * @return true it the the replacement was successful, false if not (e.g. the image's geometry differs)
     */
    bool ReplaceInterpolationSession(mitk::Image::Pointer oldSession, mitk::Image::Pointer newSession);

    /**
     * @brief Removes all sessions
     */
    void RemoveAllInterpolationSessions();

    mitk::Image *GetImage();

    /**
     * @brief Get the Contours at a certain timeStep and layerID.
     *
     * @param timeStep Time Step from which to get the contours.
     * @param layerID Layer from which to get the contours.
     * @return std::vector<ContourPositionInformation> Returns contours.
     */
    ContourPositionInformationList* GetContours(unsigned int timeStep, unsigned int layerID);

    /**
     * @brief Trigerred with the "Reinit Interpolation" action. The contours are used to repopulate the
     *        surfaceInterpolator data structures so that interpolation can be performed after reloading data.
     *
     * @param contourList List of contours extracted
     * @param contourPlanes List of planes at which the contours were extracted
     */
    void CompleteReinitialization(const std::vector<mitk::Surface::Pointer>& contourList,
                                           std::vector<const mitk::PlaneGeometry *>& contourPlanes);

    /**
     * @brief Removes contours of a particular label, at a given time step and layerID.
     *
     * @param label Label of contour to remove.
     * @param timeStep Time step in which to remove the contours.
     * @param layerID Layer in which the contour should be removed.
     */
    void RemoveContours(mitk::Label::PixelType label, unsigned int timeStep, unsigned int layerID);

    /**
     * Estimates the memory which is needed to build up the equationsystem for the interpolation.
     * \returns The percentage of the real memory which will be used by the interpolation
     */
    double EstimatePortionOfNeededMemory();

    /**
     * Adds Contours from the active Label to the interpolation pipeline
     */
    void AddActiveLabelContoursForInterpolation(mitk::Label::PixelType activeLabel);

    unsigned int GetNumberOfInterpolationSessions();

    /**
     * @brief Removes the contour for a given plane for the current selected segmenation
     * @param contourInfo the contour which should be removed
     * @return true if a contour was found and removed, false if no contour was found
     */
    bool RemoveContour(ContourPositionInformation contourInfo);

    /**
     * @brief Get the Segmentation Image Node object
     *
     * @return DataNode* returns the DataNode containing the segmentation image.
     */
    mitk::DataNode* GetSegmentationImageNode();



  protected:
    SurfaceInterpolationController();

    ~SurfaceInterpolationController() override;

    template <typename TPixel, unsigned int VImageDimension>
    void GetImageBase(itk::Image<TPixel, VImageDimension> *input, itk::ImageBase<3>::Pointer &result);

  private:

    /**
     * @brief
     *
     * @param caller
     * @param event
     */
    void OnSegmentationDeleted(const itk::Object *caller, const itk::EventObject &event);

    /**
     * @brief Function that removes contours of a particular label when the "Remove Label" event is trigerred in the labelSetImage.
     *
     */
    void OnRemoveLabel(mitk::Label::PixelType removedLabelValue);

    /**
     * @brief When a new contour is added to the pipeline or an existing contour is replaced,
     *        the plane geometry information of that contour is added as a child node to the
     *        current node of the segmentation image. This is useful in the retrieval of contour information
     *        when data is reloaded after saving.
     *
     * @param contourInfo contourInfo struct to add to data storage.
     */
    void AddPlaneGeometryNodeToDataStorage(const ContourPositionInformation& contourInfo);

    /**
     * @brief Function that toggles active label, when the active label is changed.
     *
     */
    void OnActiveLabel(mitk::Label::PixelType);

    /**
     * @brief Clears the interpolation data structures. Called from CompleteReinitialization().
     *
     */
    void ClearInterpolationSession();

    /**
     * @brief Add contour to the interpolation pipeline
     *
     * @param contourInfo Contour information to be added
     * @param reinitializationAction If the contour is coming from a reinitialization process or not
     */
    void AddToInterpolationPipeline(ContourPositionInformation& contourInfo, bool reinitializationAction = false);

    /**
     * @brief Function to respond to layer changed
     *
     */
    void OnLayerChanged();

    itk::SmartPointer<ReduceContourSetFilter> m_ReduceFilter;
    itk::SmartPointer<ComputeContourSetNormalsFilter> m_NormalsFilter;
    itk::SmartPointer<CreateDistanceImageFromSurfaceFilter> m_InterpolateSurfaceFilter;

    mitk::Surface::Pointer m_Contours;

    double m_DistanceImageSpacing;

    vtkSmartPointer<vtkPolyData> m_PolyData;

    mitk::DataStorage::Pointer m_DataStorage;

    ContourContainer m_ListOfInterpolationSessions;
    ContourListMap m_ListOfContours;

    mitk::Surface::Pointer m_InterpolationResult;

    unsigned int m_CurrentNumberOfReducedContours;
    unsigned int m_NumberOfConnectionsAdded;

    mitk::Image *m_SelectedSegmentation;

    std::map<mitk::Image *, unsigned long> m_SegmentationObserverTags;

    mitk::TimePointType m_CurrentTimePoint;

    unsigned int m_ContourIndex;
    unsigned int m_ContourPosIndex;
    unsigned int m_NumberOfLayersInCurrentSegmentation;

    mitk::Label::PixelType m_PreviousActiveLabelValue;
    mitk::Label::PixelType m_CurrentActiveLabelValue;

    unsigned int m_PreviousLayerIndex;
    unsigned int m_CurrentLayerIndex;
  };

  namespace ContourExt
  {
    /**
     * @brief Returns the plane the contour belongs to.
     *
     * @param ContourNormal
     * @return size_t
     */
    size_t GetContourOrientation(const mitk::Vector3D& ContourNormal);

    /**
     * @brief Function used to compute an interior point of the contour.
     *        Used to react to the merge label and erase label actions.
     *
     *
     * @tparam VImageDimension Dimension of the image
     * @param contour Contour for which to compute the interior point
     * @param labelSetImage Label Set Image For which to find the contour
     * @param currentTimePoint Current Time Point of the Image
     * @return mitk::Point3D The returned point in the interior of the contour.s
     */
    template<unsigned int VImageDimension>
    mitk::Point3D ComputeInteriorPointOfContour(const mitk::SurfaceInterpolationController::ContourPositionInformation& contour,
                                                 mitk::LabelSetImage * labelSetImage,
                                                 mitk::TimePointType currentTimePoint);
    /**
     * @brief Get a Grid points within the bounding box of the contour at a certain spacing.
     *
     * @param planeDimension  Plane orientation (Sagittal, Coronal, Axial)
     * @param startDim1 Starting coordinate along dimension 1 to start the grid point sampling from
     * @param numPointsToSampleDim1 Number of points to sample along dimension 1
     * @param deltaDim1 Spacing for dimension 1 at which points should be sampled
     * @param startDim2 Starting coordinate along dimension 2 to start the grid point sampling from
     * @param numPointsToSampleDim2 Number of points to sample along dimension 2
     * @param deltaDim2 Spacing for dimension 1 at which points should be sampled
     * @param valuePlaneDim Slice index of the plane in the volume
     * @return std::vector< mitk::Point3D > The computed grid points are returned by the function.
     */
    std::vector< mitk::Point3D > GetBoundingBoxGridPoints(size_t planeDimension,
                                            double startDim1,
                                            size_t numPointsToSampleDim1,
                                            double deltaDim1,
                                            double startDim2,
                                            size_t numPointsToSampleDim2,
                                            double deltaDim2,
                                            double valuePlaneDim);
  };

}

#endif
