/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceBasedInterpolationController_h_Included
#define mitkSurfaceBasedInterpolationController_h_Included

//#include "mitkCommon.h"
#include "mitkContourModel.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkSurface.h"
#include <MitkSurfaceInterpolationExports.h>

#include "mitkCreateDistanceImageFromSurfaceFilter.h"
#include "mitkReduceContourSetFilter.h"

#include "mitkProgressBar.h"

namespace mitk
{
  class RestorePlanePositionOperation;

  class MITKSURFACEINTERPOLATION_EXPORT SurfaceBasedInterpolationController : public itk::Object
  {
  public:
    mitkClassMacroItkParent(SurfaceBasedInterpolationController, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    static SurfaceBasedInterpolationController *GetInstance();

    /**
     * Adds a new extracted contour to the list
     */
    void AddNewContour(ContourModel::Pointer newContour, RestorePlanePositionOperation *op);

    /**
     * Launches the interpolation method. A surface mesh is generated out of the given extracted contours.
     */
    void Interpolate();

    /**
     * Retrieves a surface mesh resulting from the interpolation of the given extracted contours.
     */
    mitk::Surface::Pointer GetInterpolationResult();

    /**
     * Sets the minimum spacing of the current selected segmentation
     * This is needed since the contour points we reduced before, are used to interpolate the surface
     */
    void SetMinSpacing(double minSpacing);

    /**
     * Sets the minimum spacing of the current selected segmentation
     * This is needed since the contour points we reduced before, are used to interpolate the surface
     */
    void SetMaxSpacing(double maxSpacing);

    /**
     * Sets the volume i.e. the number of pixels that the distance image should have
     * By evaluation we found out that 50.000 pixel delivers a good result
     */
    void SetDistanceImageVolume(unsigned int value);

    /**
     * Sets the working image used by the interpolation method.
     * This is needed because the calculation of the normals needs to now wheather a normal points toward the inside of
     * a segmentation or not
     */
    void SetWorkingImage(Image *workingImage);

    /**
     * Retrieves the input contours as a mitk::Surface
     */
    Surface *GetContoursAsSurface();

    /**
     * Sets the current list of contour points which is used for the surface interpolation
     * @param activeLabel The active label in the current working image
     */
    void SetActiveLabel(int activeLabel);

    mitk::Image *GetImage();

    /**
     * Estimates the memory that is needed to build up the equation system for the interpolation.
     * \returns The percentage of the real memory which will be used by the interpolation calculation
     */
    double EstimatePortionOfNeededMemory();

  protected:
    SurfaceBasedInterpolationController();

    ~SurfaceBasedInterpolationController() override;

    void Initialize();

  private:
    //   void OnSegmentationDeleted(const itk::Object *caller, const itk::EventObject &event);
    /*
    struct ContourPositionPair {
      ContourModel::Pointer contour;
      RestorePlanePositionOperation* position;
     };
     */
    typedef std::pair<ContourModel::Pointer, RestorePlanePositionOperation *> ContourPositionPair;
    typedef std::vector<ContourPositionPair> ContourPositionPairList;
    typedef std::map<unsigned int, ContourPositionPairList> ContourListMap;

    // ReduceContourSetFilter::Pointer m_ReduceFilter;
    // ComputeContourSetNormalsFilter::Pointer m_NormalsFilter;
    CreateDistanceImageFromSurfaceFilter::Pointer m_InterpolateSurfaceFilter;

    double m_MinSpacing;
    double m_MaxSpacing;

    unsigned int m_DistanceImageVolume;

    Image *m_WorkingImage;

    Surface::Pointer m_Contours;

    //    vtkSmartPointer<vtkPolyData> m_PolyData;

    ContourListMap m_MapOfContourLists;

    mitk::Surface::Pointer m_InterpolationResult;

    //    unsigned int m_CurrentNumberOfReducedContours;

    int m_ActiveLabel;

    //    std::map<mitk::Image*, unsigned long> m_SegmentationObserverTags;
  };
}
#endif
