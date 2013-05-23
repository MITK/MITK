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

#include "mitkTimeGeometry.h"
#include "mitkGeometry3D.h"

#include "mitkRotationOperation.h"
#include "mitkInteractionConst.h"
#include <mitkMatrixConvert.h>
#include <mitkImageCast.h>

#include "mitkTestingMacros.h"
#include <fstream>
#include <mitkVector.h>


#include <mitkStandaloneDataStorage.h>
#include "mitkImageGenerator.h"
#include <limits>

class mitkTimeGeometryTestClass
{
public:
  void Translation_Image_MovedOrigin(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    // DimX, DimY, DimZ,
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::Geometry3D::Pointer geometry = image->GetTimeGeometry()->GetGeometryForTimeStep(0);
    mitk::Point3D imageOrigin = geometry->GetOrigin();
    mitk::Point3D expectedOrigin;
    expectedOrigin[0] = 0;
    expectedOrigin[1] = 0;
    expectedOrigin[2] = 0;
    MITK_TEST_CONDITION(mitk::Equal(imageOrigin, expectedOrigin), "Original origin match expected origin");

    expectedOrigin[0] = 0.325;
    expectedOrigin[1] = 0.487;
    expectedOrigin[2] = 0.78;

    mitk::Vector3D translationVector;
    translationVector[0] = expectedOrigin[0];
    translationVector[1] = expectedOrigin[1];
    translationVector[2] = expectedOrigin[2];

    for (mitk::TimeStepType timeStep = 0; timeStep < image->GetTimeGeometry()->GetNumberOfTimeSteps(); ++timeStep)
    {
      image->GetTimeGeometry()->GetGeometryForTimeStep(timeStep)->Translate(translationVector);
    }
    imageOrigin = image->GetGeometry(0)->GetOrigin();
    MITK_TEST_CONDITION(mitk::Equal(imageOrigin, expectedOrigin), "Translated origin match expected origin");

    expectedOrigin[0] = 2*translationVector[0];
    expectedOrigin[1] = 2*translationVector[1];
    expectedOrigin[2] = 2*translationVector[2];

    for (mitk::TimeStepType timeStep = 0; timeStep < image->GetTimeGeometry()->GetNumberOfTimeSteps(); ++timeStep)
    {
      image->GetTimeGeometry()->GetGeometryForTimeStep(timeStep)->Translate(translationVector);
    }
    imageOrigin = image->GetGeometry(0)->GetOrigin();
    MITK_TEST_CONDITION(mitk::Equal(imageOrigin, expectedOrigin), "Translated origin match expected origin");

  }


  void Rotate_Image_RotatedPoint(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();
    mitk::DataNode::Pointer dataNode = mitk::DataNode::New();

    // DimX, DimY, DimZ,
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    dataNode->SetData(image);
    ds->Add(dataNode);
    mitk::Geometry3D::Pointer geometry = image->GetTimeGeometry()->GetGeometryForTimeStep(0);
    mitk::Point3D expectedPoint;
    expectedPoint[0] = 3*0.5;
    expectedPoint[1] = 3*0.33;
    expectedPoint[2] = 3*0.78;
    mitk::Point3D originalPoint;
    originalPoint[0] = 3;
    originalPoint[1] = 3;
    originalPoint[2] = 3;
    mitk::Point3D worldPoint;
    geometry->IndexToWorld(originalPoint, worldPoint);
    MITK_TEST_CONDITION(mitk::Equal(worldPoint, expectedPoint), "Index-to-World without rotation as expected ");

    mitk::Point3D pointOfRotation;
    pointOfRotation[0] = 0;
    pointOfRotation[1] = 0;
    pointOfRotation[2] = 0;
    mitk::Vector3D vectorOfRotation;
    vectorOfRotation[0] = 1;
    vectorOfRotation[1] = 0.5;
    vectorOfRotation[2] = 0.2;
    float angleOfRotation = 73.0;
    mitk::RotationOperation* rotation = new mitk::RotationOperation(mitk::OpROTATE,pointOfRotation, vectorOfRotation, angleOfRotation);

    image->GetTimeGeometry()->ExecuteOperation(rotation);

    expectedPoint[0] = 2.6080379;
    expectedPoint[1] = -0.75265157;
    expectedPoint[2] = 1.1564401;

    image->GetGeometry(0)->IndexToWorld(originalPoint,worldPoint);
    MITK_TEST_CONDITION(mitk::Equal(worldPoint, expectedPoint), "Rotation returns expected values ");
  }

  void Scale_Image_ScaledPoint(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    // DimX, DimY, DimZ,
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::Geometry3D::Pointer geometry = image->GetTimeGeometry()->GetGeometryForTimeStep(0);
    mitk::Point3D expectedPoint;
    expectedPoint[0] = 3*0.5;
    expectedPoint[1] = 3*0.33;
    expectedPoint[2] = 3*0.78;
    mitk::Point3D originalPoint;
    originalPoint[0] = 3;
    originalPoint[1] = 3;
    originalPoint[2] = 3;
    mitk::Point3D worldPoint;
    geometry->IndexToWorld(originalPoint, worldPoint);
    MITK_TEST_CONDITION(mitk::Equal(worldPoint, expectedPoint), "Index-to-World with old Scaling as expected ");

    mitk::Vector3D newSpacing;
    newSpacing[0] = 2;
    newSpacing[1] = 1.254;
    newSpacing[2] = 0.224;
    image->SetSpacing(newSpacing);

    expectedPoint[0] = 3*2;
    expectedPoint[1] = 3*1.254;
    expectedPoint[2] = 3*0.224;

    image->GetGeometry(0)->IndexToWorld(originalPoint,worldPoint);
    MITK_TEST_CONDITION(mitk::Equal(worldPoint, expectedPoint), "Index-toWorld with new Scaling returns expected values ");
  }

  void GetMinimumTimePoint_4DImage_Zero(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimePointType expectedTimePoint = geometry->GetMinimumTimePoint();
    MITK_TEST_CONDITION(mitk::Equal(expectedTimePoint, 0), "Returns correct minimum time point ");
  }

    void GetMaximumTimePoint_4DImage_DimT(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimePointType expectedTimePoint = geometry->GetMaximumTimePoint();
    MITK_TEST_CONDITION(mitk::Equal(expectedTimePoint, DimT), "Returns correct maximum time point ");
  }

  void GetNumberOfTimeSteps_Image_ReturnDimT(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimeStepType expectedTimeSteps = geometry->GetNumberOfTimeSteps();
    MITK_TEST_CONDITION(mitk::Equal(expectedTimeSteps, DimT), "Returns correct number of time Steps ");
  }

  void GetMinimumTimePoint_3DImage_Min(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimePointType expectedTimePoint = geometry->GetMinimumTimePoint();
    MITK_TEST_CONDITION(mitk::Equal(expectedTimePoint, -std::numeric_limits<mitk::TimePointType>().max()), "Returns correct minimum time point ");
  }

  void GetMaximumTimePoint_3DImage_Max(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimePointType expectedTimePoint = geometry->GetMaximumTimePoint();
    MITK_INFO << expectedTimePoint;
    MITK_INFO << std::numeric_limits<mitk::TimePointType>().max();
    MITK_TEST_CONDITION(mitk::Equal(expectedTimePoint, std::numeric_limits<mitk::TimePointType>().max()), "Returns correct maximum time point ");
  }

  void GetTimeBounds_4DImage_ZeroAndDimT(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimeBounds expectedTimeBounds = geometry->GetTimeBounds();
    MITK_TEST_CONDITION(mitk::Equal(expectedTimeBounds[0], 0), "Returns correct minimum time point ");
    MITK_TEST_CONDITION(mitk::Equal(expectedTimeBounds[1], DimT), "Returns correct maximum time point ");
  }

  void GetTimeBounds_3DImage_ZeroAndDimT(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimeBounds expectedTimeBounds = geometry->GetTimeBounds();
    MITK_TEST_CONDITION(mitk::Equal(expectedTimeBounds[0], -std::numeric_limits<mitk::TimePointType>().max()), "Returns correct minimum time point ");
    MITK_TEST_CONDITION(mitk::Equal(expectedTimeBounds[1], std::numeric_limits<mitk::TimePointType>().max()), "Returns correct maximum time point ");
  }

  void IsValidTimePoint_ImageValidTimePoint_True(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    bool isValid = geometry->IsValidTimePoint(DimT-1);
    MITK_TEST_CONDITION(mitk::Equal(isValid, true), "Is valid time Point correct minimum time point ");
  }

  void IsValidTimePoint_ImageNegativInvalidTimePoint_False(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    bool isValid = geometry->IsValidTimePoint(-DimT);
    MITK_TEST_CONDITION(mitk::Equal(isValid, false), "Is invalid time Point correct minimum time point ");
  }

  void IsValidTimePoint_ImageInvalidTimePoint_False(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    bool isValid = geometry->IsValidTimePoint(DimT+1);
    MITK_TEST_CONDITION(mitk::Equal(isValid, false), "Is invalid time Point correct minimum time point ");
  }

  void IsValidTimeStep_ImageValidTimeStep_True(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    bool isValid = geometry->IsValidTimeStep(DimT-1);
    MITK_TEST_CONDITION(mitk::Equal(isValid, true), "Is valid time Point correct minimum time point ");
  }

  void IsValidTimeStep_ImageNegativInvalidTimeStep_False(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    bool isValid = geometry->IsValidTimeStep(-DimT);
    MITK_TEST_CONDITION(mitk::Equal(isValid, false), "Is invalid time Point correct minimum time point ");
  }

  void IsValidTimeStep_ImageInvalidTimeStep_False(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    bool isValid = geometry->IsValidTimeStep(DimT);
    MITK_TEST_CONDITION(mitk::Equal(isValid, false), "Is invalid time Point correct minimum time point ");
  }

  void TimeStepToTimePoint_ImageValidTimeStep_TimePoint(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimePointType timePoint= geometry->TimeStepToTimePoint(DimT-1);
    MITK_TEST_CONDITION(mitk::Equal(timePoint, DimT-1), "Calculated right time Point for Time Step ");
  }

  void TimeStepToTimePoint_ImageInvalidTimeStep_TimePoint(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimePointType timePoint= geometry->TimeStepToTimePoint(DimT+1);
    MITK_TEST_CONDITION(mitk::Equal(timePoint, DimT+1), "Calculated right time Point for invalid Time Step ");
  }

  void TimePointToTimeStep_ImageValidTimePoint_TimePoint(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimeStepType timePoint= geometry->TimePointToTimeStep(DimT-0.5);
    MITK_TEST_CONDITION(mitk::Equal(timePoint, DimT-1), "Calculated right time step for valid time point");
  }

  void TimePointToTimeStep_4DImageInvalidTimePoint_TimePoint(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimeStepType timePoint= geometry->TimePointToTimeStep(DimT+1.5);
    MITK_TEST_CONDITION(mitk::Equal(timePoint, DimT+1), "Calculated right time step for invalid time point");
  }

  void TimePointToTimeStep_4DImageNegativInvalidTimePoint_TimePoint(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::TimePointType negativTimePoint = (-1.0*DimT) - 1.5;
    mitk::TimeStepType timePoint= geometry->TimePointToTimeStep(negativTimePoint);
    MITK_TEST_CONDITION(mitk::Equal(timePoint, 0), "Calculated right time step for negativ invalid time point");
  }

  void GetgeometryForTimeStep_ImageValidTimeStep_CorrectGeometry(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::Geometry3D::Pointer geometry3D = geometry->GetGeometryForTimeStep(DimT-1);
    MITK_TEST_CONDITION(geometry3D.IsNotNull(), "Non-zero geometry returned");

    mitk::Point3D expectedPoint;
    expectedPoint[0] = 3*0.5;
    expectedPoint[1] = 3*0.33;
    expectedPoint[2] = 3*0.78;
    mitk::Point3D originalPoint;
    originalPoint[0] = 3;
    originalPoint[1] = 3;
    originalPoint[2] = 3;
    mitk::Point3D worldPoint;
    geometry3D->IndexToWorld(originalPoint, worldPoint);
    MITK_TEST_CONDITION(mitk::Equal(worldPoint, expectedPoint), "Geometry transformation match expection. ");
  }

  void GetgeometryForTimeStep_ImageInvalidTimeStep_CorrectGeometry(unsigned int DimX, unsigned int DimY, unsigned int DimZ, unsigned int DimT)
  {
    mitk::Image::Pointer image = mitk::ImageGenerator::GenerateRandomImage<float>(DimX, DimY, DimZ, DimT,0.5,0.33,0.78,100);
    mitk::TimeGeometry::Pointer geometry = image->GetTimeGeometry();
    mitk::Geometry3D::Pointer geometry3D = geometry->GetGeometryForTimeStep(DimT+1);
    MITK_TEST_CONDITION(geometry3D.IsNull(), "Null-Pointer geometry returned");
  }
};







int mitkTimeGeometryTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkTimeGeometryTest);

  mitkTimeGeometryTestClass testClass;

  MITK_TEST_OUTPUT(<< "Test for 3D image");
  testClass.Translation_Image_MovedOrigin(30,25,20,1);
  testClass.Rotate_Image_RotatedPoint(30,25,20,1);
  testClass.Scale_Image_ScaledPoint(30,25,20,1);
  testClass.GetNumberOfTimeSteps_Image_ReturnDimT(30,25,20,1);
  testClass.GetMinimumTimePoint_3DImage_Min(30,25,20,1);
  testClass.GetMaximumTimePoint_3DImage_Max(30,25,20,1);
  testClass.GetTimeBounds_3DImage_ZeroAndDimT(30,25,20,1);
  testClass.IsValidTimePoint_ImageValidTimePoint_True(30,25,20,1);
  testClass.IsValidTimeStep_ImageValidTimeStep_True(30,25,20,1);
  testClass.IsValidTimeStep_ImageNegativInvalidTimeStep_False(30,25,20,1);
  testClass.IsValidTimeStep_ImageInvalidTimeStep_False(30,25,20,1);
  testClass.TimeStepToTimePoint_ImageValidTimeStep_TimePoint(30,25,20,1);
  testClass.TimeStepToTimePoint_ImageInvalidTimeStep_TimePoint(30,25,20,1);
  testClass.TimePointToTimeStep_ImageValidTimePoint_TimePoint(30,25,20,1);
  testClass.GetgeometryForTimeStep_ImageValidTimeStep_CorrectGeometry(30,25,20,1);
  testClass.GetgeometryForTimeStep_ImageInvalidTimeStep_CorrectGeometry(30,25,20,1);

/*

  MITK_TEST_OUTPUT(<< "Test for 2D image");
  testClass.Translation_Image_MovedOrigin(30,25,1 ,1); // Test with 2D-Image
  testClass.Rotate_Image_RotatedPoint(30,25,1 ,1); // Test with 2D-Image
  testClass.Scale_Image_ScaledPoint(30,25,1 ,1); // Test with 2D-Image

*/

  MITK_TEST_OUTPUT(<< "Test for 3D+time image");
  testClass.Translation_Image_MovedOrigin(30,25,20,5); // Test with 3D+t-Image
  testClass.Rotate_Image_RotatedPoint(30,25,20,5); // Test with 3D+t-Image
  testClass.Scale_Image_ScaledPoint(30,25,20,5); // Test with 3D+t-Image
  testClass.GetNumberOfTimeSteps_Image_ReturnDimT(30,25,20,5);
  testClass.GetMinimumTimePoint_4DImage_Zero(30,25,20,5);
  testClass.GetMaximumTimePoint_4DImage_DimT(30,25,20,5);
  testClass.GetTimeBounds_4DImage_ZeroAndDimT(30,25,20,5);
  testClass.IsValidTimePoint_ImageValidTimePoint_True(30,25,20,5);
  testClass.IsValidTimePoint_ImageNegativInvalidTimePoint_False(30,25,20,5);
  testClass.IsValidTimePoint_ImageInvalidTimePoint_False(30,25,20,5);
  testClass.IsValidTimeStep_ImageValidTimeStep_True(30,25,20,5);
  testClass.IsValidTimeStep_ImageNegativInvalidTimeStep_False(30,25,20,5);
  testClass.IsValidTimeStep_ImageInvalidTimeStep_False(30,25,20,5);
  testClass.TimeStepToTimePoint_ImageValidTimeStep_TimePoint(30,25,20,5);
  testClass.TimeStepToTimePoint_ImageInvalidTimeStep_TimePoint(30,25,20,5);
  testClass.TimePointToTimeStep_ImageValidTimePoint_TimePoint(30,25,20,5);
  testClass.TimePointToTimeStep_4DImageInvalidTimePoint_TimePoint(30,25,20,5);
  testClass.TimePointToTimeStep_4DImageNegativInvalidTimePoint_TimePoint(30,25,20,5);
  testClass.GetgeometryForTimeStep_ImageValidTimeStep_CorrectGeometry(30,25,20,5);
  testClass.GetgeometryForTimeStep_ImageinvalidTimeStep_CorrectGeometry(30,25,20,5);

/*

  MITK_TEST_OUTPUT(<< "Test for 2D+time image");
  testClass.Translation_Image_MovedOrigin(30,25,1 ,5); // Test with 2D+t-Image
  testClass.Rotate_Image_RotatedPoint(30,25,1 ,5); // Test with 2D+t-Image
  testClass.Scale_Image_ScaledPoint(30,25,1 ,5); // Test with 2D+t-Image

*/

  MITK_TEST_END();


  return EXIT_SUCCESS;
}
