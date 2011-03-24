/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-12 14:05:50 +0100 (Fr, 12 Mrz 2010) $
Version:   $Revision: 16010 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  expectedCoordinate[0] = -395.5988;
  expectedCoordinate[1] = -219.7771;
  expectedCoordinate[2] =  879.1084;
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


