/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkToFProcessingCommon.h"

namespace mitk
{
  ToFProcessingCommon::ToFPoint3D ToFProcessingCommon::IndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance,
    ToFScalarType focalLengthX, ToFScalarType focalLengthY, ToFScalarType principalPointX, ToFScalarType principalPointY)
  {
    ToFPoint3D cartesianCoordinates;

    // calculate image coordinates in pixel units;
    // Note: pixel unit (pX) in x direction does normally not equal pixel unit (pY) in y direction.
    // Therefore, a transformation in one of the pixel units is necessary
    // Here, pX as image coordinate unit is chosen
    // pY = (focalLengthX / focalLengthY) * pX
    ToFScalarType imageX = i - principalPointX;
    ToFScalarType imageY = j - principalPointY;
    ToFScalarType imageY_in_pX = imageY * (focalLengthX / focalLengthY);

    //distance from pinhole to pixel (i,j) in pX units (pixel unit in x direction)
    ToFScalarType d_in_pX = sqrt(imageX*imageX + imageY_in_pX*imageY_in_pX + focalLengthX*focalLengthX);

    cartesianCoordinates[0] = distance * imageX / d_in_pX; //Strahlensatz: x / imageX = distance / d
    cartesianCoordinates[1] = distance * imageY_in_pX / d_in_pX; //Strahlensatz: y / imageY = distances / d
    cartesianCoordinates[2] = distance * focalLengthX / d_in_pX; //Strahlensatz: z / f = distance / d.

    return cartesianCoordinates;
  }

  //See also "Hacking the Kinect" - Jeff Kramer, Matt Parker, Daniel Herrera C., Nicolas Burrus, Florian Echtler, Chapter 7, Part 1 "Moving from Depth Map to Point Cloud.
  ToFProcessingCommon::ToFPoint3D ToFProcessingCommon::KinectIndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance,
    ToFScalarType focalLengthX, ToFScalarType focalLengthY, ToFScalarType principalPointX, ToFScalarType principalPointY)
  {
    ToFPoint3D cartesianCoordinates;

    cartesianCoordinates[0] = distance * (i - principalPointX) / focalLengthX;
    cartesianCoordinates[1] = distance * (j - principalPointY) / focalLengthY;
    cartesianCoordinates[2] = distance;

    return cartesianCoordinates;
  }

  ToFProcessingCommon::ToFPoint3D ToFProcessingCommon::CartesianToKinectIndexCoordinates(ToFScalarType cartesianPointX, ToFScalarType cartesianPointY, ToFScalarType cartesianPointZ, ToFScalarType focalLengthX, ToFScalarType focalLengthY, ToFScalarType principalPointX,                                                                                                     ToFScalarType principalPointY, bool calculateDistance)
  {
      ToFPoint3D indexCoordinatesAndDistanceValue;
      indexCoordinatesAndDistanceValue[0] = ((cartesianPointX*focalLengthX)/cartesianPointZ) + principalPointX;
      indexCoordinatesAndDistanceValue[1] = ((cartesianPointY*focalLengthY)/cartesianPointZ) + principalPointY;

      if (calculateDistance)
      { //There is no computation for kinect. See KinectIndexToCartesianCoordinates.
        indexCoordinatesAndDistanceValue[2] = cartesianPointZ;
      }
      else
      {
        indexCoordinatesAndDistanceValue[2] = 0.0;
      }
      return indexCoordinatesAndDistanceValue;
  }

  ToFProcessingCommon::ToFPoint3D ToFProcessingCommon::IndexToCartesianCoordinatesWithInterpixdist(unsigned int i, unsigned int j, ToFScalarType distance, ToFScalarType focalLength,
    ToFScalarType interPixelDistanceX, ToFScalarType interPixelDistanceY,
    ToFScalarType principalPointX, ToFScalarType principalPointY)
  {
    ToFPoint3D cartesianCoordinates;

    // calculate image coordinates in mm;
    ToFScalarType imageX = (( i - principalPointX ) * interPixelDistanceX);
    ToFScalarType imageY = (( j - principalPointY ) * interPixelDistanceY);

    //distance from pinhole to pixel
    ToFScalarType d = sqrt(imageX*imageX + imageY*imageY + focalLength*focalLength);

    cartesianCoordinates[0] = (distance)*imageX / d; //Strahlensatz: x / imageX = (distance) / d
    cartesianCoordinates[1] = (distance)*imageY / d; //Strahlensatz: y / imageY = (distance) / d
    cartesianCoordinates[2] = ((distance*focalLength) / d); //Strahlensatz: z / f = distance / d.

    return cartesianCoordinates;
  }



  ToFProcessingCommon::ToFPoint3D ToFProcessingCommon::CartesianToIndexCoordinates(ToFScalarType cartesianPointX, ToFScalarType cartesianPointY,ToFScalarType cartesianPointZ,
    ToFScalarType focalLengthX, ToFScalarType focalLengthY,
    ToFScalarType principalPointX, ToFScalarType principalPointY, bool calculateDistance)
  {
    ToFPoint3D indexCoordinatesAndDistanceValue;

    ToFScalarType imageX = cartesianPointX*focalLengthX/cartesianPointZ; //Strahlensatz: cartesianPointX / imageX = cartesianPointZ / focalLengthX
    ToFScalarType imageY = cartesianPointY*focalLengthY/cartesianPointZ; //Strahlensatz: cartesianPointY / imageY = cartesianPointZ / focalLengthY

    indexCoordinatesAndDistanceValue[0] = imageX + principalPointX;
    indexCoordinatesAndDistanceValue[1] = imageY + principalPointY;

    // Note: pixel unit (pX) in x direction does normally not equal pixel unit (pY) in y direction.
    // Therefore, a transformation in one of the pixel units is necessary (for calculation of the distance value only)
    // Here, pX as image coordinate unit is chosen
    // pY = (focalLengthX / focalLengthY) * pX
    ToFScalarType imageY_in_pX = imageY * focalLengthX/focalLengthY;

    //distance from pinhole to pixel
    ToFScalarType d_in_pX = sqrt(imageX*imageX + imageY_in_pX*imageY_in_pX + focalLengthX*focalLengthX);

    if (calculateDistance)
    {
      indexCoordinatesAndDistanceValue[2] = d_in_pX*(cartesianPointZ) / focalLengthX;
    }
    else
    {
      indexCoordinatesAndDistanceValue[2] = 0.0;
    }
    return indexCoordinatesAndDistanceValue;
  }

  ToFProcessingCommon::ToFPoint3D ToFProcessingCommon::CartesianToIndexCoordinatesWithInterpixdist(ToFScalarType cartesianPointX, ToFScalarType cartesianPointY,ToFScalarType cartesianPointZ,
    ToFScalarType focalLength, ToFScalarType interPixelDistanceX, ToFScalarType interPixelDistanceY,
    ToFScalarType principalPointX, ToFScalarType principalPointY, bool calculateDistance)
  {
    ToFPoint3D indexCoordinatesAndDistanceValue;

    ToFScalarType imageX = cartesianPointX*focalLength/cartesianPointZ;
    ToFScalarType imageY = cartesianPointY*focalLength/cartesianPointZ;

    indexCoordinatesAndDistanceValue[0] = imageX/interPixelDistanceX + principalPointX;
    indexCoordinatesAndDistanceValue[1] = imageY/interPixelDistanceY + principalPointY;

    ToFScalarType d = sqrt(imageX*imageX + imageY*imageY + focalLength*focalLength);

    if (calculateDistance)
    {
      indexCoordinatesAndDistanceValue[2] = d*(cartesianPointZ) / focalLength;
    }
    else
    {
      indexCoordinatesAndDistanceValue[2] = 0.0;
    }
    return indexCoordinatesAndDistanceValue;
  }

  ToFProcessingCommon::ToFPoint3D ToFProcessingCommon::ContinuousKinectIndexToCartesianCoordinates(mitk::Point2D continuousIndex, ToFScalarType distance, ToFScalarType focalLengthX, ToFScalarType focalLengthY, ToFScalarType principalPointX, ToFScalarType principalPointY)
  {
    ToFPoint3D cartesianCoordinates;

    cartesianCoordinates[0] = distance * (continuousIndex[0] - principalPointX) / focalLengthX;
    cartesianCoordinates[1] = distance * (continuousIndex[1] - principalPointY) / focalLengthY;
    cartesianCoordinates[2] = distance;

    return cartesianCoordinates;
  }

  ToFProcessingCommon::ToFScalarType ToFProcessingCommon::CalculateViewAngle( mitk::CameraIntrinsics::Pointer intrinsics, unsigned int dimX )
  {
    ToFScalarType viewAngle = 180*(atan2(intrinsics->GetPrincipalPointX(),intrinsics->GetFocalLengthX()) + atan2((dimX-intrinsics->GetPrincipalPointX()),intrinsics->GetFocalLengthX()))/vnl_math::pi;
    return viewAngle;
  }
}
