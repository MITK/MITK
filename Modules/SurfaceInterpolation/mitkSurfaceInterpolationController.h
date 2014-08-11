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

    static SurfaceInterpolationController* GetInstance();

    /**
     * Adds a new extracted contour to the list
     */
    void AddNewContour (Surface::Pointer newContour, PlaneGeometry::Pointer plane);

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
     * Sets the current segmentation which is used by the interpolation
     * This is needed because the calculation of the normals needs to now wheather a normal points inside a segmentation or not
     */
    void SetSegmentationImage(Image* workingImage);

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
     * @brief Removes all sessions
     */
    void RemoveAllInterpolationSessions();

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

   struct ContourPositionPair {
     Surface::Pointer contour;
     mitk::PlaneGeometry::Pointer plane;
   };

    typedef std::vector<ContourPositionPair> ContourPositionPairList;
    typedef std::map<mitk::Image*, ContourPositionPairList> ContourListMap;

    ContourPositionPairList::iterator m_Iterator;

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
