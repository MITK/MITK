/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#ifndef MITKTOFPROCESSINGCOMMON_H
#define MITKTOFPROCESSINGCOMMON_H

#include "mitkToFProcessingExports.h"
#include "mitkVector.h"
#include <vnl/vnl_math.h>



namespace mitk
{
  /**
  * @brief Helper class providing functions which are useful for multiple usage
  *
  * Currently the following methods are provided:
  * <ul>
  * <li> Conversion from 2D image coordinates to 3D world coordinates (IndexToCartesianCoordinates())
  * <li> Conversion from 3D world coordinates to 2D image coordinates (CartesianToIndexCoordinates())
  * </ul>
  *
  *
  * @ingroup ToFProcessing
  */
  class mitkToFProcessing_EXPORT ToFProcessingCommon
  {
  public:
    typedef double ToFScalarType;
    typedef itk::Point<ToFScalarType, 2> ToFPoint2D;
    typedef itk::Point<ToFScalarType, 3> ToFPoint3D;
    typedef itk::Vector<ToFScalarType, 2> ToFVector2D;
    typedef itk::Vector<ToFScalarType, 3> ToFVector3D;


    /*!
  \brief Convert index based distances to cartesian coordinates
  \param i index in x direction of image plane
  \param j index in y direction of image plane
  \param distance distance value at given index in mm
  \param focalLength focal length of optical system in mm (mostly obtained from camera calibration)
  \param interPixelDistance distance between adjacent pixels in mm
  \param principalPoint coordinates of principal point on image plane in pixel
  \return cartesian coordinates for current index will be written here
  */  
    static ToFPoint3D IndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance, ToFScalarType focalLength,
                                                  ToFPoint2D interPixelDistance, ToFPoint2D principalPoint);
    /*!
  \brief Convert index based distances to cartesian coordinates
  \param index index coordinates
  \param distance distance value at given index in mm
  \param focalLength focal length of optical system (mostly obtained from camera calibration)
  \param interPixelDistance distance between adjacent pixels in mm for x and y direction
  \param principalPoint coordinates of principal point on image plane in pixel
  \return cartesian coordinates for current index will be written here
  */
    inline static ToFPoint3D IndexToCartesianCoordinates(mitk::Index3D index, ToFScalarType distance, ToFScalarType focalLength,
                                                         ToFPoint2D interPixelDistance, ToFPoint2D principalPoint)
    {
      return IndexToCartesianCoordinates(index[0],index[1],distance,focalLength,interPixelDistance,principalPoint);
    }

    /*!
  \brief Convert index based distances to cartesian coordinates
  \param cartesianPoint point (of a surface or point set) to convert in 3D coordinates
  \param distance distance value at given index in mm
  \param focalLength focal length of optical system in mm (mostly obtained from camera calibration)
  \param interPixelDistance distance between adjacent pixels in mm for x and y direction
  \param principalPoint coordinates of principal point on image plane in pixel
  \param calculateDistance if this flag is set, the distance value is stored in the z position of the output otherwise z=0
  \return a ToFPoint3D. (int)ToFPoint3D[0]+0.5 and (int)ToFPoint3D[0]+0.5 will return the x and y index coordinates. ToFPoint3D[2] contains the distance value
  */
    static ToFPoint3D CartesianToIndexCoordinates(ToFPoint3D cartesianPoint, ToFScalarType focalLength,
                                                  ToFPoint2D interPixelDistance, ToFPoint2D principalPoint, bool calculateDistance=true);
  };
}
#endif
