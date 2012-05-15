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

#include <mitkTestingMacros.h>
#include <mitkToFProcessingCommon.h>

/**Documentation
 *  test for the class "ToFProcessingCommon".
 */
int mitkToFProcessingCommonTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFProcessingCommon");

  unsigned int i = 10;
  unsigned int j = 50;
  mitk::ScalarType distance = 1000;
  mitk::ScalarType focalLength = 10;
  mitk::Point2D interPixelDistance;
  interPixelDistance[0] = 0.05;
  interPixelDistance[1] = 0.05;
  mitk::Point2D principalPoint;
  principalPoint[0] = 100;
  principalPoint[1] = 100;
  // expected coordinate
  mitk::ToFProcessingCommon::ToFPoint3D expectedCoordinate;
  expectedCoordinate[0] = -400.0988;
  expectedCoordinate[1] = -222.2771;
  expectedCoordinate[2] =  889.1084;
  // resulting coordinate
  mitk::ToFProcessingCommon::ToFPoint3D resultingCoordinate = mitk::ToFProcessingCommon::IndexToCartesianCoordinates(i,j,distance,focalLength,interPixelDistance,principalPoint);
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(expectedCoordinate,resultingCoordinate),"Testing IndexToCartesianCoordinates()");
  // expected index
  mitk::ToFProcessingCommon::ToFPoint3D expectedIndex;
  expectedIndex[0] = i;
  expectedIndex[1] = j;
  expectedIndex[2] = 1000;
  mitk::ToFProcessingCommon::ToFPoint3D resultingIndex = mitk::ToFProcessingCommon::CartesianToIndexCoordinates(expectedCoordinate,focalLength,interPixelDistance,principalPoint);
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(expectedIndex,resultingIndex),"Testing CartesianToIndexCoordinates()");

  MITK_TEST_END();

}


