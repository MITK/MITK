/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef mitkSurfaceInterpolationController_h_Included
#define mitkSurfaceInterpolationController_h_Included

#include "mitkCommon.h"
#include "MitkExtExports.h"
//#include "mitkRestorePlanePositionOperation.h"
#include "mitkSurface.h"

#include "mitkCreateDistanceImageFromSurfaceFilter.h"
#include "mitkReduceContourSetFilter.h"
#include "mitkComputeContourSetNormalsFilter.h"

namespace mitk
{

 class MitkExt_EXPORT SurfaceInterpolationController : public itk::Object
 {

  public:

    mitkClassMacro(SurfaceInterpolationController, itk::Object);
    itkNewMacro(Self);

    static SurfaceInterpolationController* GetInstance();

    /*
     * Adds a new extracted contour to the list 
     */
    void AddNewContour(Surface::Pointer newContour/*, RestorePlanePostionOperation &op*/);

    /*
     * If a contour at the position represented by the RestorePlaneOperation already exists
       this contour is returned. Otherwise NULL is returned.
     */
    //Surface::Pointer GetSurface (RestorePlanePositionOperation &op);

    /*
     * Interpolates the 3D surface from the given extracted contours
     */
    Surface::Pointer Interpolate ();

    itkSetMacro(MinSpacing, double);
    itkSetMacro(MaxSpacing, double);
    itkSetMacro(WorkingImage, Image*);

   /* bool DataSetHasChanged();*/



 protected:

   SurfaceInterpolationController();

   ~SurfaceInterpolationController();

 private:

   struct ContourPositionPair {
     Surface::Pointer contour;
     //RestorePlanePosition* position;
   };

    typedef std::vector<ContourPositionPair> ContourPositionPairList;
    //typedef std::map< const RestorePlanePositionOperation*, Surface* > ContourPositionList;

    ContourPositionPairList m_ContourList;

    ReduceContourSetFilter::Pointer m_ReduceFilter;
    ComputeContourSetNormalsFilter::Pointer m_NormalsFilter;
    CreateDistanceImageFromSurfaceFilter::Pointer m_InterpolateSurfaceFilter;

    double m_MinSpacing;
    double m_MaxSpacing;

    const Image* m_WorkingImage;

   /* bool m_Modified;*/

 };
}
#endif