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

#ifndef mitkSurfaceInterpolationController_h_Included
#define mitkSurfaceInterpolationController_h_Included

#include "mitkCommon.h"
#include <MitkSurfaceInterpolationExports.h>
#include "mitkRestorePlanePositionOperation.h"
#include "mitkSurface.h"
#include "mitkInteractionConst.h"
#include "mitkColorProperty.h"
#include "mitkProperties.h"

#include "mitkCreateDistanceImageFromSurfaceFilter.h"
#include "mitkReduceContourSetFilter.h"
#include "mitkComputeContourSetNormalsFilter.h"

#include "mitkDataNode.h"
#include "mitkDataStorage.h"
#include "mitkWeakPointer.h"

#include "vtkPolygon.h"
#include "vtkPoints.h"
#include "vtkCellArray.h"
#include "vtkPolyData.h"
#include "vtkSmartPointer.h"
#include "vtkAppendPolyData.h"

#include "vtkMarchingCubes.h"
#include "vtkImageData.h"
#include "mitkVtkRepresentationProperty.h"
#include "vtkProperty.h"

#include "mitkProgressBar.h"

namespace mitk
{

 class MitkSurfaceInterpolation_EXPORT SurfaceInterpolationController : public itk::Object
 {

  public:

    mitkClassMacro(SurfaceInterpolationController, itk::Object)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    struct ContourPositionInformation {
      Surface::Pointer contour;
      Vector3D contourNormal;
      Point3D contourPoint;
    };

    typedef std::vector<ContourPositionInformation> ContourPositionInformationList;
    typedef std::map<mitk::Image*, ContourPositionInformationList> ContourListMap;

    static SurfaceInterpolationController* GetInstance();

    /**
     * @brief Adds a new extracted contour to the list
     * @param newContour the contour to be added. If a contour at that position
     *        already exists the related contour will be updated
     */
    void AddNewContour (Surface::Pointer newContour);

    /**
     * @brief Removes the contour for a given plane for the current selected segmenation
     * @param contourInfo the contour which should be removed
     * @return true if a contour was found and removed, false if no contour was found
     */
    bool RemoveContour (ContourPositionInformation contourInfo);

    /**
     * @brief Adds new extracted contours to the list. If one or more contours at a given position
     *        already exist they will be updated respectively
     * @param newContours the list of the contours
     */
    void AddNewContours (std::vector<Surface::Pointer> newContours);

    /**
    * @brief Returns the contour for a given plane for the current selected segmenation
    * @param ontourInfo the contour which should be returned
    * @return the contour as an mitk::Surface. If no contour is available at the give position NULL is returned
    */
    const mitk::Surface* GetContour (ContourPositionInformation contourInfo);

    /**
    * @brief Returns the number of available contours for the current selected segmentation
    * @return the number of contours
    */
    unsigned int GetNumberOfContours();

    /**
     * Interpolates the 3D surface from the given extracted contours
     */
    void Interpolate ();

    mitk::Surface::Pointer GetInterpolationResult();

    /**
     * Sets the minimum spacing of the current selected segmentation
     * This is needed since the contour points we reduced before they are used to interpolate the surface
     */
    void SetMinSpacing(double minSpacing);

    /**
     * Sets the minimum spacing of the current selected segmentation
     * This is needed since the contour points we reduced before they are used to interpolate the surface
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

    Surface* GetContoursAsSurface();

    void SetDataStorage(DataStorage::Pointer ds);

    /**
     * Sets the current list of contourpoints which is used for the surface interpolation
     * @param segmentation The current selected segmentation
     * \deprecatedSince{2014_03}
     */
    DEPRECATED (void SetCurrentSegmentationInterpolationList(mitk::Image::Pointer segmentation));

    /**
     * Sets the current list of contourpoints which is used for the surface interpolation
     * @param segmentation The current selected segmentation
     */
    void SetCurrentInterpolationSession(mitk::Image::Pointer currentSegmentationImage);

    /**
     * Removes the segmentation and all its contours from the list
     * @param segmentation The segmentation to be removed
     * \deprecatedSince{2014_03}
     */
    DEPRECATED (void RemoveSegmentationFromContourList(mitk::Image* segmentation));

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

    /**
     * @brief Reinitializes the interpolation using the provided contour data
     * @param contours a mitk::Surface which contains the contours as polys in the vtkPolyData
     */
    void ReinitializeInterpolation(mitk::Surface::Pointer contours);

    mitk::Image* GetImage();

    /**
     * Estimates the memory which is needed to build up the equationsystem for the interpolation.
     * \returns The percentage of the real memory which will be used by the interpolation
     */
    double EstimatePortionOfNeededMemory();

    unsigned int GetNumberOfInterpolationSessions();

 protected:

   SurfaceInterpolationController();

   ~SurfaceInterpolationController();

   template<typename TPixel, unsigned int VImageDimension> void GetImageBase(itk::Image<TPixel, VImageDimension>* input, itk::ImageBase<3>::Pointer& result);

 private:

   void OnSegmentationDeleted(const itk::Object *caller, const itk::EventObject &event);

   void ReinitializeInterpolation();

   void AddToInterpolationPipeline(ContourPositionInformation contourInfo);

    ReduceContourSetFilter::Pointer m_ReduceFilter;
    ComputeContourSetNormalsFilter::Pointer m_NormalsFilter;
    CreateDistanceImageFromSurfaceFilter::Pointer m_InterpolateSurfaceFilter;

    Surface::Pointer m_Contours;

    vtkSmartPointer<vtkPolyData> m_PolyData;

    mitk::DataStorage::Pointer m_DataStorage;

    ContourListMap m_ListOfInterpolationSessions;

    mitk::Surface::Pointer m_InterpolationResult;

    unsigned int m_CurrentNumberOfReducedContours;

    mitk::Image* m_SelectedSegmentation;

    std::map<mitk::Image*, unsigned long> m_SegmentationObserverTags;
 };
}
#endif
