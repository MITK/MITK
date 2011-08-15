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
    void AddNewSurface(Surface::Pointer newSurface/*, RestorePlanePostionOperation &op*/);

    /*
     * If a contour at the position represented by the RestorePlaneOperation already exists
       this contour is returned. Otherwise NULL is returned.
     */
    //Surface::Pointer GetSurface (RestorePlanePositionOperation &op);

    /*
     * Interpolates the 3D surface from the given extracted contours
     */
    Surface::Pointer Interpolate ();



 protected:

   SurfaceInterpolationController();

   ~SurfaceInterpolationController();

 private:

   typedef
   struct{
     Surface::Pointer contour;
     //RestorePlanePosition* position;
   }ContourPositionPair;

   std::vector<ContourPositionPair> m_ContourList;

 };
}


#endif