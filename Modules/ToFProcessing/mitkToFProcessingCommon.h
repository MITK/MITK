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
  * The coordinate conversion follows the model of a common pinhole camera where the origin of the camera
  * coordinate system (world coordinates) is at the pinhole
  * \image html ../Modules/ToFProcessing/Documentation/PinholeCameraModel.png
  * The definition of the image plane and its coordinate systems (pixel and mm) is depicted in the following image
  * \image html ../Modules/ToFProcessing/Documentation/ImagePlane.png
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
    \param focalLengthX focal length of optical system in pixel units in x-direction (mostly obtained from camera calibration)
    \param focalLengthY focal length of optical system in pixel units in y-direction (mostly obtained from camera calibration)
    \param principalPointX x coordinate of principal point on image plane in pixel
    \param principalPointY y coordinate of principal point on image plane in pixel
    \return cartesian coordinates for current index will be written here
    */
    static ToFPoint3D IndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance,
      ToFScalarType focalLengthX, ToFScalarType focalLengthY, ToFScalarType principalPointX, ToFScalarType principalPointY);

    /*!
    \brief Convert index based distances to cartesian coordinates
    \param i index in x direction of image plane
    \param j index in y direction of image plane
    \param distance distance value at given index in mm
    \param focalLength focal length of optical system in pixel units (mostly obtained from camera calibration)
    \param principalPoint coordinates of principal point on image plane in pixel
    \return cartesian coordinates for current index will be written here
    */
    inline static ToFPoint3D IndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance,
      ToFPoint2D focalLength, ToFPoint2D principalPoint)
    {
      return IndexToCartesianCoordinates(i,j,distance,focalLength[0],focalLength[1],principalPoint[0],principalPoint[1]);
    }

    /*!
    \brief Convert index based distances to cartesian coordinates
    \param index index coordinates
    \param distance distance value at given index in mm
    \param focalLength focal length of optical system in pixel units (mostly obtained from camera calibration)
    \param principalPoint coordinates of principal point on image plane in pixel
    \return cartesian coordinates for current index will be written here
    */
    inline static ToFPoint3D IndexToCartesianCoordinates(mitk::Index3D index, ToFScalarType distance,
      ToFPoint2D focalLength, ToFPoint2D principalPoint)
    {
      return IndexToCartesianCoordinates(index[0],index[1],distance,focalLength[0],focalLength[1],principalPoint[0], principalPoint[1]);
    }
    /*!
    \brief Convenience method to convert index based distances to cartesian coordinates using array as input
    \param i index in x direction of image plane
    \param j index in y direction of image plane
    \param distance distance value at given index in mm
    \param focalLength focal length of optical system in pixel units (mostly obtained from camera calibration)
    \param principalPoint coordinates of principal point on image plane in pixel
    \return cartesian coordinates for current index will be written here
    */
    inline static ToFPoint3D IndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance,
      ToFScalarType focalLength[2], ToFScalarType principalPoint[2])
    {
      return IndexToCartesianCoordinates(i,j,distance,focalLength[0],focalLength[1],principalPoint[0],principalPoint[1]);
    }


    /*!
    \brief Convert index based distances to cartesian coordinates
    \param i index in x direction of image plane
    \param j index in y direction of image plane
    \param distance distance value at given index in mm
    \param focalLength focal length of optical system in mm (mostly obtained from camera calibration)
    \param interPixelDistanceX distance in x direction between adjacent pixels in mm
    \param interPixelDistanceY distance in y direction between adjacent pixels in mm
    \param principalPointX x coordinate of principal point on image plane in pixel
    \param principalPointY y coordinate of principal point on image plane in pixel
    \return cartesian coordinates for current index will be written here
    */
    static ToFPoint3D IndexToCartesianCoordinatesWithInterpixdist(unsigned int i, unsigned int j, ToFScalarType distance, ToFScalarType focalLength,
      ToFScalarType interPixelDistanceX, ToFScalarType interPixelDistanceY, ToFScalarType principalPointX, ToFScalarType principalPointY);

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
    inline static ToFPoint3D IndexToCartesianCoordinatesWithInterpixdist(unsigned int i, unsigned int j, ToFScalarType distance, ToFScalarType focalLength,
      ToFPoint2D interPixelDistance, ToFPoint2D principalPoint)
    {
      return IndexToCartesianCoordinatesWithInterpixdist(i,j,distance,focalLength,interPixelDistance[0],interPixelDistance[1],principalPoint[0],principalPoint[1]);
    }
    /*!
    \brief Convert index based distances to cartesian coordinates
    \param index index coordinates
    \param distance distance value at given index in mm
    \param focalLength focal length of optical system (mostly obtained from camera calibration)
    \param interPixelDistance distance between adjacent pixels in mm for x and y direction
    \param principalPoint coordinates of principal point on image plane in pixel
    \return cartesian coordinates for current index will be written here
    */
    inline static ToFPoint3D IndexToCartesianCoordinatesWithInterpixdist(mitk::Index3D index, ToFScalarType distance, ToFScalarType focalLength,
      ToFPoint2D interPixelDistance, ToFPoint2D principalPoint)
    {
      return IndexToCartesianCoordinatesWithInterpixdist(index[0],index[1],distance,focalLength,interPixelDistance[0], interPixelDistance[1],principalPoint[0], principalPoint[1]);
    }
    /*!
    \brief Convenience method to convert index based distances to cartesian coordinates using array as input
    \param i index in x direction of image plane
    \param j index in y direction of image plane
    \param distance distance value at given index in mm
    \param focalLength focal length of optical system in mm (mostly obtained from camera calibration)
    \param interPixelDistance distance between adjacent pixels in mm
    \param principalPoint coordinates of principal point on image plane in pixel
    \return cartesian coordinates for current index will be written here
    */
    inline static ToFPoint3D IndexToCartesianCoordinatesWithInterpixdist(unsigned int i, unsigned int j, ToFScalarType distance, ToFScalarType focalLength,
      ToFScalarType interPixelDistance[2], ToFScalarType principalPoint[2])
    {
      return IndexToCartesianCoordinatesWithInterpixdist(i,j,distance,focalLength,interPixelDistance[0],interPixelDistance[1],principalPoint[0],principalPoint[1]);
    }


    /*!
    \brief Convert cartesian coordinates to index based distances
    \param cartesianPointX x coordinate of point (of a surface or point set) to convert in 3D coordinates
    \param cartesianPointY y coordinate of point (of a surface or point set) to convert in 3D coordinates
    \param cartesianPointZ z coordinate of point (of a surface or point set) to convert in 3D coordinates
    \param focalLengthX focal length of optical system in pixel units in x-direction (mostly obtained from camera calibration)
    \param focalLengthY focal length of optical system in pixel units in y-direction (mostly obtained from camera calibration)
    \param principalPointX x coordinate of principal point on image plane in pixel
    \param principalPointY y coordinate of principal point on image plane in pixel
    \param calculateDistance if this flag is set, the distance value is stored in the z position of the output otherwise z=0
    \return a ToFPoint3D. (int)ToFPoint3D[0]+0.5 and (int)ToFPoint3D[0]+0.5 will return the x and y index coordinates. ToFPoint3D[2] contains the distance value
    */
    static ToFPoint3D CartesianToIndexCoordinates(ToFScalarType cartesianPointX, ToFScalarType cartesianPointY,ToFScalarType cartesianPointZ,
                                                  ToFScalarType focalLengthX, ToFScalarType focalLengthY,
                                                  ToFScalarType principalPointX, ToFScalarType principalPointY, bool calculateDistance=true);

    /*!
    \brief Convenience method to convert cartesian coordinates to index based distances using arrays
    \param cartesianPoint point (of a surface or point set) to convert in 3D coordinates
    \param focalLength focal length of optical system in pixel units (mostly obtained from camera calibration)
    \param principalPoint coordinates of principal point on image plane in pixel
    \param calculateDistance if this flag is set, the distance value is stored in the z position of the output otherwise z=0
    \return a ToFPoint3D. (int)ToFPoint3D[0]+0.5 and (int)ToFPoint3D[0]+0.5 will return the x and y index coordinates. ToFPoint3D[2] contains the distance value
    */
    inline static ToFPoint3D CartesianToIndexCoordinates(ToFScalarType cartesianPoint[3], ToFScalarType focalLength[2],
                                                         ToFScalarType principalPoint[2], bool calculateDistance=true)
    {
      return CartesianToIndexCoordinates(cartesianPoint[0],cartesianPoint[1],cartesianPoint[2],focalLength[0], focalLength[1],
                                         principalPoint[0],principalPoint[1],calculateDistance);
    }
    /*!
    \brief Convert cartesian coordinates to index based distances
    \param cartesianPoint point (of a surface or point set) to convert in 3D coordinates
    \param focalLength focal length of optical system in pixel units (mostly obtained from camera calibration)
    \param principalPoint coordinates of principal point on image plane in pixel
    \param calculateDistance if this flag is set, the distance value is stored in the z position of the output otherwise z=0
    \return a ToFPoint3D. (int)ToFPoint3D[0]+0.5 and (int)ToFPoint3D[0]+0.5 will return the x and y index coordinates. ToFPoint3D[2] contains the distance value
    */
    inline static ToFPoint3D CartesianToIndexCoordinates(ToFPoint3D cartesianPoint, ToFPoint2D focalLength,
      ToFPoint2D principalPoint, bool calculateDistance=true)
    {
      return CartesianToIndexCoordinates(cartesianPoint[0],cartesianPoint[1],cartesianPoint[2],focalLength[0], focalLength[1],
        principalPoint[0],principalPoint[1],calculateDistance);
    }


    /*!
    \brief Convert cartesian coordinates to index based distances
    \param cartesianPointX x coordinate of point (of a surface or point set) to convert in 3D coordinates
    \param cartesianPointY y coordinate of point (of a surface or point set) to convert in 3D coordinates
    \param cartesianPointZ z coordinate of point (of a surface or point set) to convert in 3D coordinates
    \param focalLength focal length of optical system in mm (mostly obtained from camera calibration)
    \param interPixelDistanceX distance in x direction between adjacent pixels in mm
    \param interPixelDistanceY distance in y direction between adjacent pixels in mm
    \param principalPointX x coordinate of principal point on image plane in pixel
    \param principalPointY y coordinate of principal point on image plane in pixel
    \param calculateDistance if this flag is set, the distance value is stored in the z position of the output otherwise z=0
    \return a ToFPoint3D. (int)ToFPoint3D[0]+0.5 and (int)ToFPoint3D[0]+0.5 will return the x and y index coordinates. ToFPoint3D[2] contains the distance value
    */
    static ToFPoint3D CartesianToIndexCoordinatesWithInterpixdist(ToFScalarType cartesianPointX, ToFScalarType cartesianPointY,ToFScalarType cartesianPointZ,
                                                  ToFScalarType focalLength, ToFScalarType interPixelDistanceX, ToFScalarType interPixelDistanceY,
                                                  ToFScalarType principalPointX, ToFScalarType principalPointY, bool calculateDistance=true);

    /*!
    \brief Convenience method to convert cartesian coordinates to index based distances using arrays
    \param cartesianPoint point (of a surface or point set) to convert in 3D coordinates
    \param focalLength focal length of optical system in mm (mostly obtained from camera calibration)
    \param interPixelDistance distance between adjacent pixels in mm for x and y direction
    \param principalPoint coordinates of principal point on image plane in pixel
    \param calculateDistance if this flag is set, the distance value is stored in the z position of the output otherwise z=0
    \return a ToFPoint3D. (int)ToFPoint3D[0]+0.5 and (int)ToFPoint3D[0]+0.5 will return the x and y index coordinates. ToFPoint3D[2] contains the distance value
    */
    inline static ToFPoint3D CartesianToIndexCoordinatesWithInterpixdist(ToFScalarType cartesianPoint[3], ToFScalarType focalLength,
                                                         ToFScalarType interPixelDistance[2], ToFScalarType principalPoint[2],
                                                         bool calculateDistance=true)
    {
      return CartesianToIndexCoordinatesWithInterpixdist(cartesianPoint[0],cartesianPoint[1],cartesianPoint[2],focalLength,
                                         interPixelDistance[0],interPixelDistance[1],principalPoint[0],principalPoint[1],calculateDistance);
    }
    /*!
    \brief Convert cartesian coordinates to index based distances
    \param cartesianPoint point (of a surface or point set) to convert in 3D coordinates
    \param focalLength focal length of optical system in mm (mostly obtained from camera calibration)
    \param interPixelDistance distance between adjacent pixels in mm for x and y direction
    \param principalPoint coordinates of principal point on image plane in pixel
    \param calculateDistance if this flag is set, the distance value is stored in the z position of the output otherwise z=0
    \return a ToFPoint3D. (int)ToFPoint3D[0]+0.5 and (int)ToFPoint3D[0]+0.5 will return the x and y index coordinates. ToFPoint3D[2] contains the distance value
    */
    inline static ToFPoint3D CartesianToIndexCoordinatesWithInterpixdist(ToFPoint3D cartesianPoint, ToFScalarType focalLength,
      ToFPoint2D interPixelDistance, ToFPoint2D principalPoint, bool calculateDistance=true)
    {
      return CartesianToIndexCoordinatesWithInterpixdist(cartesianPoint[0],cartesianPoint[1],cartesianPoint[2],focalLength,
        interPixelDistance[0],interPixelDistance[1],principalPoint[0],principalPoint[1],calculateDistance);
    }
  };
}
#endif
