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
#include "mitkToFProcessingCommon.h"

namespace mitk
{
  ToFProcessingCommon::ToFPoint3D ToFProcessingCommon::IndexToCartesianCoordinates(unsigned int i, unsigned int j, ToFScalarType distance, ToFScalarType focalLength,
    ToFScalarType interPixelDistanceX, ToFScalarType interPixelDistanceY,
    ToFScalarType principalPointX, ToFScalarType principalPointY)
  {
    ToFPoint3D cartesianCoordinates;

    // calculate image coordinates in mm;
    ToFScalarType imageX = (( i - principalPointX ) * interPixelDistanceX);
    ToFScalarType imageY = (( j - principalPointY ) * interPixelDistanceY);

    //distance from pinhole to pixel
    ToFScalarType d = sqrt(imageX*imageX + imageY*imageY + focalLength*focalLength);

    cartesianCoordinates[0] = (distance-d)*imageX / d; //Strahlensatz: x / imageX = (distance-d) / d
    cartesianCoordinates[1] = (distance-d)*imageY / d; //Strahlensatz: y / imageY = (distance-d) / d
    cartesianCoordinates[2] = ((distance*focalLength) / d) - focalLength; //Strahlensatz: z+f / f = distance / d.

    return cartesianCoordinates;
  }

  ToFProcessingCommon::ToFPoint3D ToFProcessingCommon::CartesianToIndexCoordinates(ToFScalarType cartesianPointX, ToFScalarType cartesianPointY,ToFScalarType cartesianPointZ,
    ToFScalarType focalLength, ToFScalarType interPixelDistanceX, ToFScalarType interPixelDistanceY, 
    ToFScalarType principalPointX, ToFScalarType principalPointY, bool calculateDistance)
  {
    ToFPoint3D indexCoordinatesAndDistanceValue;

    ToFScalarType imageX = focalLength/cartesianPointZ * cartesianPointX;
    ToFScalarType imageY = focalLength/cartesianPointZ * cartesianPointY;

    indexCoordinatesAndDistanceValue[0] = imageX/interPixelDistanceX + principalPointX;
    indexCoordinatesAndDistanceValue[1] = imageY/interPixelDistanceY + principalPointY;

    ToFScalarType d = sqrt(imageX*imageX + imageY*imageY + focalLength*focalLength);

    if (calculateDistance)
    {
      indexCoordinatesAndDistanceValue[2] = d*(cartesianPointZ+focalLength) / focalLength;
    }
    else
    {
      indexCoordinatesAndDistanceValue[2] = 0.0;
    }
    return indexCoordinatesAndDistanceValue;
  }
}
