/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTransferFunction.h"

#include "mitkTestingMacros.h"

#include <iostream>

/**
*  Unit test for class mitk::TransferFunction.
*  
*  argc and argv are the command line parameters which were passed to 
*  the ADD_TEST command in the CMakeLists.txt file. For the automatic
*  tests, argv is either empty for the simple tests or contains the filename
*  of a test image for the image tests (see CMakeLists.txt).
*/
int mitkTransferFunctionTest(int /* argc */, char* /*argv*/[])
{
  // always start with this!
  MITK_TEST_BEGIN("TransferFunction");

  // let's create an object of our class  
  mitk::TransferFunction::Pointer myTransferFunction = mitk::TransferFunction::New();

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED( myTransferFunction.IsNotNull(), "Testing instantiation" );

  /*************************************************************************/
  // Create and set control point arrays for scalar opacity transfer function
  mitk::TransferFunction::ControlPoints scalarOpacityPoints;
  scalarOpacityPoints.push_back( std::make_pair( 0.0, 0.0 ) );
  scalarOpacityPoints.push_back( std::make_pair( 5.0, 0.3 ) );
  scalarOpacityPoints.push_back( std::make_pair( 10.0, 1.0 ) );
  myTransferFunction->SetScalarOpacityPoints( scalarOpacityPoints );
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->GetScalarOpacityFunction()->GetSize() == 3,
    "Adding three point/value pairs to scalar opacity transfer function via VTK interface" );

  // Create and set control point arrays for gradient opacity transfer function
  mitk::TransferFunction::ControlPoints gradientOpacityPoints;
  gradientOpacityPoints.push_back( std::make_pair( 0.0, 0.2 ) );
  gradientOpacityPoints.push_back( std::make_pair( 3.0, 0.7 ) );
  gradientOpacityPoints.push_back( std::make_pair( 7.0, 0.8 ) );
  gradientOpacityPoints.push_back( std::make_pair( 15.0, 0.9 ) );
  myTransferFunction->SetGradientOpacityPoints( gradientOpacityPoints );
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->GetGradientOpacityFunction()->GetSize() == 4,
    "Adding four point/value pairs to gradient opacity transfer function via VTK interface" );

  // Create and set control point arrays for color transfer function
  mitk::TransferFunction::RGBControlPoints colorPoints;
  itk::RGBPixel< double > rgb0, rgb1, rgb2, rgb3;
  rgb0[0] = 0.1f; rgb0[1] = 0.3f; rgb0[2] = 0.5f;
  colorPoints.push_back( std::make_pair( 2.0, rgb0 ) );
  rgb1[0] = 0.3; rgb1[1] = 0.8; rgb1[2] = 0.9;
  colorPoints.push_back( std::make_pair( 3.0, rgb1 ) );
  rgb2[0] = 0.6; rgb2[1] = 0.5; rgb2[2] = 0.4;
  colorPoints.push_back( std::make_pair( 4.0, rgb2 ) );
  rgb3[0] = 0.7; rgb3[1] = 0.1; rgb3[2] = 0.2;
  colorPoints.push_back( std::make_pair( 5.0, rgb3 ) );

  myTransferFunction->SetRGBPoints( colorPoints );
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->GetColorTransferFunction()->GetSize() == 4,
    "Adding four point/value pairs to color transfer function via VTK interface" );


  /*************************************************************************/
  // Add a new point to scalar opacity transfer function
  myTransferFunction->AddScalarOpacityPoint( 3.0, 0.2 );
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->GetScalarOpacityFunction()->GetSize() == 4,
    "Adding new point/value to scalar opacity transfer function via MITK interface" );

  // Add a new point to gradient opacity transfer function
  myTransferFunction->AddGradientOpacityPoint( 19.0, 1.0 );
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->GetGradientOpacityFunction()->GetSize() == 5,
    "Adding new point/value to gradient opacity transfer function via MITK interface" );

  // Add a new point to color transfer function
  myTransferFunction->AddRGBPoint( 1.0, 0.0, 0.1, 0.2 );
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->GetColorTransferFunction()->GetSize() == 5,
    "Adding new point/value to color transfer function via MITK interface" );
  

  /*************************************************************************/
  // Retrieve two points from scalar opacity transfer function
  double v1, v2;
  v1 = myTransferFunction->GetScalarOpacityFunction()->GetValue( 3.0 );
  v2 = myTransferFunction->GetScalarOpacityFunction()->GetValue( 10.0 );
  MITK_TEST_CONDITION_REQUIRED(
    (fabs( v1 - 0.2 ) < 1.0e-5) && (fabs( v2 - 1.0 ) < 1.0e-5),
    "Retrieving values at two points from scalar opacity transfer function" );

  // Retrieve two points from gradient opacity transfer function
  v1 = myTransferFunction->GetGradientOpacityFunction()->GetValue( 0.0 );
  v2 = myTransferFunction->GetGradientOpacityFunction()->GetValue( 19.0 );
  MITK_TEST_CONDITION_REQUIRED(
    (fabs( v1 - 0.2 ) < 1.0e-5) && (fabs( v2 - 1.0 ) < 1.0e-5),
    "Retrieving values at two points from gradient opacity transfer function" );

  // Retrieve two points from color transfer function
  vtkFloatingPointType vrgb1[3], vrgb2[3];
  myTransferFunction->GetColorTransferFunction()->GetColor( 1.0, vrgb1 );
  myTransferFunction->GetColorTransferFunction()->GetColor( 4.0, vrgb2 );
  std::cout << vrgb2[0] << ", " << vrgb2[1] << ", " << vrgb2[2] << std::endl;
  MITK_TEST_CONDITION_REQUIRED(
       (fabs( vrgb1[0] - 0.0 ) < 1.0e-5)
    && (fabs( vrgb1[1] - 0.1 ) < 1.0e-5)
    && (fabs( vrgb1[2] - 0.2 ) < 1.0e-5)
    && (fabs( vrgb2[0] - 0.6 ) < 1.0e-5)
    && (fabs( vrgb2[1] - 0.5 ) < 1.0e-5)
    && (fabs( vrgb2[2] - 0.4 ) < 1.0e-5),
    "Retrieving values at two points from color transfer function" );


  /*************************************************************************/
  // Remove point from scalar opacity transfer function (should return new
  // number of points)
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->RemoveScalarOpacityPoint( 3.0 ) == 1,
    "Removing point from scalar opacity transfer function (should return point index)" );
  
  // Remove point from scalar opacity transfer function (should return new
  // number of points)
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->RemoveGradientOpacityPoint( 0.0 ) == 0,
    "Removing point from gradient opacity transfer function (should return point index)" );

  // Remove point from color transfer function (should return new
  // number of points)
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->RemoveRGBPoint( 5.0 ) == 4,
    "Removing point from color transfer function (should return point ndex)" );


  /*************************************************************************/
  // Try to remove non-existing point from scalar opacity transfer function (should return -1)
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->RemoveScalarOpacityPoint( 2.5 ) == -1,
    "Trying to remove non-existing point from scalar opacity transfer function (should return -1)" );

  // Try to remove non-existing point from gradient opacity transfer function (should return -1)
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->RemoveGradientOpacityPoint( 2.5 ) == -1,
    "Trying to remove non-existing point from gradient opacity transfer function (should return -1)" );

  // Try to remove non-existing point from color transfer function (should return -1)
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->RemoveRGBPoint( 2.5 ) == -1,
    "Trying to remove non-existing point from color transfer function (should return -1)" );


  /*************************************************************************/
  // Retrieve copy of scalar opacity points (check for correct content and size)
  mitk::TransferFunction::ControlPoints newScalarOpacityPoints =
    myTransferFunction->GetScalarOpacityPoints();
  MITK_TEST_CONDITION_REQUIRED(
    (newScalarOpacityPoints.size() == 3)
    && (fabs(newScalarOpacityPoints[2].first - 10.0) < 1.0e-5)
    && (fabs(newScalarOpacityPoints[2].second - 1.0) < 1.0e-5),
    "Retrieving copy of scalar opacity points (checking for correct content and size)" );

  // Retrieve copy of gradient opacity points (check for correct content and size)
  mitk::TransferFunction::ControlPoints newGradientOpacityPoints =
    myTransferFunction->GetGradientOpacityPoints();
  MITK_TEST_CONDITION_REQUIRED(
    (newGradientOpacityPoints.size() == 4)
    && (fabs(newGradientOpacityPoints[3].first - 19.0) < 1.0e-5)
    && (fabs(newGradientOpacityPoints[3].second - 1.0) < 1.0e-5),
    "Retrieving copy of gradient opacity points (checking for correct content and size)" );

  // Retrieve copy of color transfer points (check for correct content and size)
  mitk::TransferFunction::RGBControlPoints newRGBPoints =
    myTransferFunction->GetRGBPoints();
  MITK_TEST_CONDITION_REQUIRED(
    (newRGBPoints.size() == 4)
    && (fabs(newRGBPoints[3].first - 4.0) < 1.0e-5)
    && (fabs(newRGBPoints[3].second[0] - 0.6) < 1.0e-5)
    && (fabs(newRGBPoints[3].second[1] - 0.5) < 1.0e-5)
    && (fabs(newRGBPoints[3].second[2] - 0.4) < 1.0e-5),
    "Retrieving copy of color transfer function points (checking for correct content and size)" );


  /*************************************************************************/
  // Clear scalar opacity points (resulting array should be empty)
  myTransferFunction->ClearScalarOpacityPoints();
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->GetScalarOpacityPoints().size() == 0,
    "Clearing scalar opacity points (resulting array should be empty)" );

  myTransferFunction->ClearGradientOpacityPoints();
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->GetGradientOpacityPoints().size() == 0,
    "Clearing gradient opacity points (resulting array should be empty)" );

  myTransferFunction->ClearRGBPoints();
  MITK_TEST_CONDITION_REQUIRED(
    myTransferFunction->GetRGBPoints().size() == 0,
    "Clearing color transfer function points (resulting array should be empty)" );


  // always end with this!
  MITK_TEST_END()
}

