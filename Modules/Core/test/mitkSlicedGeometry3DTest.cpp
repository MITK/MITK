/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkImage.h"
#include "mitkPlaneGeometry.h"
#include "mitkSlicedGeometry3D.h"
#include "mitkTestingMacros.h"

#include <vnl/vnl_quaternion.h>
#include <vnl/vnl_quaternion.hxx>

#include <fstream>
#include <array>

static const mitk::ScalarType slicedGeometryEps = 1e-9; // Set epsilon to float precision for this test

static mitk::PlaneGeometry::Pointer createPlaneGeometry()
{
  auto planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->Initialize();
  return planeGeometry;
}

static mitk::SlicedGeometry3D::Pointer createSlicedGeometry(const mitk::Point3D &origin, const mitk::Vector3D &spacing, int numberOfSlices)
{
  auto slicedGeometry = mitk::SlicedGeometry3D::New();
  slicedGeometry->InitializeSlicedGeometry(static_cast<unsigned int>(numberOfSlices));
  slicedGeometry->SetOrigin(origin);
  slicedGeometry->SetSpacing(spacing);

  for (int i = 0; i < numberOfSlices; ++i)
  {
    auto planeGeometry = createPlaneGeometry();
    slicedGeometry->SetPlaneGeometry(planeGeometry, i);
  }

  return slicedGeometry;
}

static mitk::SlicedGeometry3D::Pointer createEvenlySpacedSlicedGeometry(mitk::PlaneGeometry::Pointer planeGeometry, mitk::ScalarType spacing, int numberOfSlices)
{
  auto slicedGeometry = mitk::SlicedGeometry3D::New();
  slicedGeometry->InitializeEvenlySpaced(planeGeometry, spacing, numberOfSlices);
  return slicedGeometry;
}

template<class T> static T createArray(mitk::ScalarType x, mitk::ScalarType y, mitk::ScalarType z)
{
  T array;
  mitk::FillVector3D(array, x, y, z);
  return array;
}

static mitk::Point3D createPoint(mitk::ScalarType x, mitk::ScalarType y, mitk::ScalarType z)
{
  return createArray<mitk::Point3D>(x, y, z);
}

static mitk::Vector3D createVector(mitk::ScalarType x, mitk::ScalarType y, mitk::ScalarType z)
{
  return createArray<mitk::Vector3D>(x, y, z);
}

void mitkSlicedGeometry3D_ChangeImageGeometryConsideringOriginOffset_Test()
{
  MITK_TEST_OUTPUT(<< "====== mitkSlicedGeometry3D_ChangeImageGeometryConsideringOriginOffset_Test() ======");

  auto origin = createPoint(91.3, -13.3, 0);
  auto spacing = createVector(1.0, 0.9, 0.3);
  auto numberOfSlices = 5;
  auto slicedGeometry = createSlicedGeometry(origin, spacing, numberOfSlices);

  MITK_TEST_OUTPUT(<< "SlicedGeometry3D isn't an image geometry by default");
  MITK_TEST_CONDITION_REQUIRED(slicedGeometry->GetImageGeometry() == false, "");

  MITK_TEST_OUTPUT(<< "First and last PlaneGeometry in SlicedGeometry3D are not image geometries");
  auto firstPlaneGeometry = slicedGeometry->GetPlaneGeometry(0);
  auto lastPlaneGeometry = slicedGeometry->GetPlaneGeometry(numberOfSlices - 1);
  MITK_TEST_CONDITION_REQUIRED(firstPlaneGeometry->GetImageGeometry() == false, "");
  MITK_TEST_CONDITION_REQUIRED(lastPlaneGeometry->GetImageGeometry() == false, "");

  auto originOfSlicedGeometry = slicedGeometry->GetOrigin();
  auto originOfFirstPlaneGeometry = firstPlaneGeometry->GetOrigin();
  auto originOfLastPlaneGeometry = lastPlaneGeometry->GetOrigin();
  auto firstCornerPointOfSlicedGeometry = slicedGeometry->GetCornerPoint(0);
  auto secondCornerPointOfFirstPlaneGeometry = firstPlaneGeometry->GetCornerPoint(1);
  auto thirdCornerPointOfLastPlaneGeometry = lastPlaneGeometry->GetCornerPoint(2);

  MITK_TEST_OUTPUT(<< "Calling SlicedGeometry3D::ChangeImageGeometryConsideringOriginOffset(true)");
  slicedGeometry->ChangeImageGeometryConsideringOriginOffset(true);
  MITK_TEST_OUTPUT(<< "SlicedGeometry3D is an image geometry");
  MITK_TEST_CONDITION_REQUIRED(slicedGeometry->GetImageGeometry() == true, "");
  MITK_TEST_OUTPUT(<< "First and last PlaneGeometry in SlicedGeometry3D are image geometries");
  MITK_TEST_CONDITION_REQUIRED(firstPlaneGeometry->GetImageGeometry() == true, "");
  MITK_TEST_CONDITION_REQUIRED(lastPlaneGeometry->GetImageGeometry() == true, "");

  MITK_TEST_OUTPUT(<< "Corner points of geometries didn't change");
  MITK_TEST_CONDITION_REQUIRED(slicedGeometry->GetCornerPoint(0) == firstCornerPointOfSlicedGeometry, "");
  MITK_TEST_CONDITION_REQUIRED(firstPlaneGeometry->GetCornerPoint(1) == secondCornerPointOfFirstPlaneGeometry, "");
  MITK_TEST_CONDITION_REQUIRED(lastPlaneGeometry->GetCornerPoint(2) == thirdCornerPointOfLastPlaneGeometry, "");

  MITK_TEST_OUTPUT(<< "Offsets were added to geometry origins");
  MITK_TEST_CONDITION_REQUIRED(slicedGeometry->GetOrigin() == originOfSlicedGeometry + slicedGeometry->GetSpacing() * 0.5, "");
  MITK_TEST_CONDITION_REQUIRED(firstPlaneGeometry->GetOrigin() == originOfFirstPlaneGeometry + firstPlaneGeometry->GetSpacing() * 0.5, "");
  MITK_TEST_CONDITION_REQUIRED(lastPlaneGeometry->GetOrigin() == originOfLastPlaneGeometry + lastPlaneGeometry->GetSpacing() * 0.5, "");

  MITK_TEST_OUTPUT(<< "Calling SlicedGeometry3D::ChangeImageGeometryConsideringOriginOffset(false)");
  slicedGeometry->ChangeImageGeometryConsideringOriginOffset(false);
  MITK_TEST_OUTPUT(<< "SlicedGeometry3D isn't an image geometry anymore");
  MITK_TEST_CONDITION_REQUIRED(slicedGeometry->GetImageGeometry() == false, "");
  MITK_TEST_OUTPUT(<< "First and last PlaneGeometry in SlicedGeometry3D are no image geometries anymore");
  MITK_TEST_CONDITION_REQUIRED(firstPlaneGeometry->GetImageGeometry() == false, "");
  MITK_TEST_CONDITION_REQUIRED(lastPlaneGeometry->GetImageGeometry() == false, "");

  MITK_TEST_OUTPUT(<< "Corner points of geometries didn't change");
  MITK_TEST_CONDITION_REQUIRED(slicedGeometry->GetCornerPoint(0) == firstCornerPointOfSlicedGeometry, "");
  MITK_TEST_CONDITION_REQUIRED(firstPlaneGeometry->GetCornerPoint(1) == secondCornerPointOfFirstPlaneGeometry, "");
  MITK_TEST_CONDITION_REQUIRED(lastPlaneGeometry->GetCornerPoint(2) == thirdCornerPointOfLastPlaneGeometry, "");

  MITK_TEST_OUTPUT(<< "Offsets were subtracted from geometry origins");
  MITK_TEST_CONDITION_REQUIRED(slicedGeometry->GetOrigin() == originOfSlicedGeometry, "");
  MITK_TEST_CONDITION_REQUIRED(firstPlaneGeometry->GetOrigin() == originOfFirstPlaneGeometry, "");
  MITK_TEST_CONDITION_REQUIRED(lastPlaneGeometry->GetOrigin() == originOfLastPlaneGeometry, "");
}

int mitkSlicedGeometry3DTest(int, char *[])
{
  mitk::ScalarType width = 100.0;
  mitk::ScalarType widthInMM = width;

  mitk::ScalarType height = 200.0;
  mitk::ScalarType heightInMM = height;

  mitk::ScalarType thicknessInMM = 3.0;

  auto right = createVector(widthInMM, 0.0, 0.0);
  auto bottom = createVector(0.0, heightInMM, 0.0);
  auto spacing = createVector(1.0, 1.0, thicknessInMM);

  auto planeGeometry = mitk::PlaneGeometry::New();
  planeGeometry->InitializeStandardPlane(right, bottom, &spacing);

  auto numberOfSlices = 5;
  auto slicedGeometry = createEvenlySpacedSlicedGeometry(planeGeometry, thicknessInMM, numberOfSlices);
  auto firstPlaneGeometry = slicedGeometry->GetPlaneGeometry(0);

  MITK_TEST_OUTPUT(<< "Check if first PlaneGeometry of evenly spaced SlicedGeometry is valid");
  MITK_TEST_CONDITION_REQUIRED(firstPlaneGeometry != nullptr, "");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(firstPlaneGeometry->GetAxisVector(0), planeGeometry->GetAxisVector(0), slicedGeometryEps), "");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(firstPlaneGeometry->GetAxisVector(1), planeGeometry->GetAxisVector(1), slicedGeometryEps), "");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(firstPlaneGeometry->GetAxisVector(2), planeGeometry->GetAxisVector(2), slicedGeometryEps), "");

  auto lastPlaneGeometry = slicedGeometry->GetPlaneGeometry(numberOfSlices - 1);
  auto expectedOriginOfLastSlice = createPoint(0.0, 0.0, thicknessInMM * (numberOfSlices - 1));

  MITK_TEST_OUTPUT(<< "Check if origin of last PlaneGeometry is at expected location");
  MITK_TEST_CONDITION_REQUIRED(mitk::Equal(lastPlaneGeometry->GetOrigin(), expectedOriginOfLastSlice, slicedGeometryEps), "");

  mitkSlicedGeometry3D_ChangeImageGeometryConsideringOriginOffset_Test();

  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
