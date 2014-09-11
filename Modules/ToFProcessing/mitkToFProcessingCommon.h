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

#include <MitkToFProcessingExports.h>
#include <mitkCameraIntrinsics.h>
#include "mitkNumericTypes.h"
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
  class MitkToFProcessing_EXPORT ToFProcessingCommon
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
    inline static ToFPoint3D IndexToCartesianCoordinates(itk::Index<3> index, ToFScalarType distance,
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
    inline static ToFPoint3D IndexToCartesianCoordinatesWithInterpixdist(itk::Index<3> index, ToFScalarType distance, ToFScalarType focalLength,
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

    /** \ingroup KinectReconstruction
     * @{
     *
     * @brief KinectIndexToCartesianCoordinates Convert a pixel (i,j) with value d to a 3D world point. This conversion is meant for Kinect and slightly different then ToF reconstruction. See also "Hacking the Kinect" - Jeff Kramer, Matt Parker, Daniel Herrera C., Nicolas Burrus, Florian Echtler, Chapter 7, Part 1 "Moving from Depth Map to Point Cloud.
     * @param i Pixel index i.
     * @param j Pixel index j.
     * @param distance Distance value d in mm as obtained from OpenNI.
     * @param focalLengthX Focallength from calibration.
     * @param focalLengthY Focallength from calibration.
     * @param principalPointX Principal point from calibration.
     * @param principalPointY Principal point from calibration.
     * @return a ToFPoint3D. The point in world coordinates (mm).
     */
    static ToFProcessingCommon::ToFPoint3D KinectIndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance, ToFScalarType focalLengthX, ToFScalarType focalLengthY, ToFScalarType principalPointX, ToFScalarType principalPointY);

    inline static ToFPoint3D KinectIndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance,
      ToFScalarType focalLength[2], ToFScalarType principalPoint[2])
    {
      return KinectIndexToCartesianCoordinates(i,j,distance,focalLength[0],focalLength[1],principalPoint[0],principalPoint[1]);
    }

    inline static ToFPoint3D KinectIndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance,
      ToFPoint2D focalLength, ToFPoint2D principalPoint)
    {
      return KinectIndexToCartesianCoordinates(i,j,distance,focalLength[0],focalLength[1],principalPoint[0],principalPoint[1]);
    }

    inline static ToFPoint3D KinectIndexToCartesianCoordinates(itk::Index<3> index, ToFScalarType distance, ToFPoint2D focalLength, ToFPoint2D principalPoint)
    {
        return KinectIndexToCartesianCoordinates(index[0],index[1],distance,focalLength[0],focalLength[1],principalPoint[0], principalPoint[1]);
    }
    /** @}*/
    /** \ingroup KinectReconstructionInverse
     * @{
     * @brief CartesianCoordinatesToKinectIndexCoordinates Transform a 3D world point back to distance image pixel coordinates.
     * @param cartesianPointX x value of the cartesian point.
     * @param cartesianPointY y value of the cartesian point.
     * @param cartesianPointZ z value of the cartesian point.
     * @param focalLengthX x value of the focal length (from calibration).
     * @param focalLengthY y value of the focal length (from calibration).
     * @param principalPointX x value of the principal point (from calibration).
     * @param principalPointY y value of the principal point (from calibration).
     * @param calculateDistance Do you want to compute also the distance of the distance image? For Kinect, this value is always the same in cartesian and index coordinates.
     * @return A ToFPoint3D containing the pixel indices (i,j) in [0] and [1] and (optionally) the distance value in [2] (or just 0.0).
     */
    static ToFPoint3D CartesianToKinectIndexCoordinates(ToFScalarType cartesianPointX, ToFScalarType cartesianPointY,
                                                                   ToFScalarType cartesianPointZ, ToFScalarType focalLengthX,
                                                                   ToFScalarType focalLengthY, ToFScalarType principalPointX,
                                                                   ToFScalarType principalPointY, bool calculateDistance=true);

    inline static ToFProcessingCommon::ToFPoint3D CartesianToKinectIndexCoordinates(ToFPoint3D cartesianPoint, ToFPoint2D focalLength, ToFPoint2D  principalPoint, bool calculateDistance=true)
    {
      return CartesianToKinectIndexCoordinates( cartesianPoint[0], cartesianPoint[1], cartesianPoint[2], focalLength[0], focalLength[1], principalPoint[0], principalPoint[1], calculateDistance);
    }
    /** @}*/
    /**
     * @brief ContinuousKinectIndexToCartesianCoordinates This method is escpially meant for reconstructing a Kinect point
     * with continuous index coordinates (i.e. not exactly a pixel position, but a point interpolated between two pixels).
     * The only difference to KinectIndexToCartesianCoordinates() is that ContinuousKinectIndexToCartesianCoordinates does not
     * cast to unsigned int for the index.
     * @param continuousIndex The continuous coordinates (e.g. 0.5; 0.5).
     * @param distance Distance value d in mm as obtained from OpenNI.
     * @param focalLengthX x value of the focal length (from calibration).
     * @param focalLengthY y value of the focal length (from calibration)
     * @param principalPointX x value of the principal point (from calibration).
     * @param principalPointY y value of the principal point (from calibration).
     * @return a ToFPoint3D. The point in world coordinates (mm).
     */
    static ToFProcessingCommon::ToFPoint3D ContinuousKinectIndexToCartesianCoordinates(mitk::Point2D continuousIndex, ToFScalarType distance, ToFScalarType focalLengthX, ToFScalarType focalLengthY, ToFScalarType principalPointX, ToFScalarType principalPointY);

    /**
    \brief Calculates the horizontal view angle of the camera with the given intrinsics
    \param intrinsics intrinsic parameters of the camera
    \param dimX dimension of the image in horizontal direction

    angle = atan(principalPoint[0]/focalLength[0]) + atan((dimX-principalPoint[0]/focalLength[0]))
    **/
    static ToFScalarType CalculateViewAngle(mitk::CameraIntrinsics::Pointer intrinsics, unsigned int dimX);
  };
}
#endif
