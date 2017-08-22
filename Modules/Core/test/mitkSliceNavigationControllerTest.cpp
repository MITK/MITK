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

#include <mitkGeometry3D.h>
#include <mitkPlaneGeometry.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkSliceNavigationController.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

// T22254

class mitkSliceNavigationControllerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSliceNavigationControllerTestSuite);
  CPPUNIT_TEST(validateAxialViewDirection);
  CPPUNIT_TEST(validateCoronalViewDirection);
  CPPUNIT_TEST(validateSagittalViewDirection);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override
  {
    mitk::Point3D origin;
    mitk::FillVector3D(origin, 10.0, 20.0, 30.0);

    mitk::Vector3D firstAxisVector;
    mitk::FillVector3D(firstAxisVector, 100.0, 0.0, 0.0);

    mitk::Vector3D secondAxisVector;
    mitk::FillVector3D(secondAxisVector, 0.0, 50.0, 0.0);

    mitk::Vector3D spacing;
    mitk::FillVector3D(spacing, 1.0, 1.0, 2.0);

    auto planeGeometry = mitk::PlaneGeometry::New();
    planeGeometry->InitializeStandardPlane(firstAxisVector, secondAxisVector, &spacing);
    planeGeometry->SetOrigin(origin);

    unsigned int numberOfSlices = 100U;

    auto slicedGeometry3D = mitk::SlicedGeometry3D::New();
    slicedGeometry3D->InitializeEvenlySpaced(planeGeometry, numberOfSlices);

    m_Geometry3D = mitk::Geometry3D::New();
    m_Geometry3D->SetBounds(slicedGeometry3D->GetBounds());
    m_Geometry3D->SetIndexToWorldTransform(slicedGeometry3D->GetIndexToWorldTransform());
  }

  void tearDown() override
  {
  }

  void validateAxialViewDirection()
  {
    auto sliceNavigationController = mitk::SliceNavigationController::New();

    sliceNavigationController->SetInputWorldGeometry3D(m_Geometry3D);
    sliceNavigationController->SetViewDirection(mitk::SliceNavigationController::Axial);
    sliceNavigationController->Update();

    mitk::Point3D origin;
    mitk::FillVector3D(origin, 10.0, 70.0, 229.0);

    mitk::Vector3D firstAxisVector;
    mitk::FillVector3D(firstAxisVector, 100.0, 0.0, 0.0);

    mitk::Vector3D secondAxisVector;
    mitk::FillVector3D(secondAxisVector, 0.0, -50.0, 0.0);

    mitk::Vector3D thirdAxisVector;
    mitk::FillVector3D(thirdAxisVector, 0.0, 0.0, -200.0);

    std::cout << "Axial view direction" << std::endl;
    CPPUNIT_ASSERT(this->validateGeometry(sliceNavigationController->GetCurrentGeometry3D(), origin, firstAxisVector, secondAxisVector, thirdAxisVector));
  }

  void validateCoronalViewDirection()
  {
    auto sliceNavigationController = mitk::SliceNavigationController::New();

    sliceNavigationController->SetInputWorldGeometry3D(m_Geometry3D);
    sliceNavigationController->SetViewDirection(mitk::SliceNavigationController::Frontal);
    sliceNavigationController->Update();

    mitk::Point3D origin;
    mitk::FillVector3D(origin, 10.0, 69.5, 30.0);

    mitk::Vector3D firstAxisVector;
    mitk::FillVector3D(firstAxisVector, 100.0, 0.0, 0.0);

    mitk::Vector3D secondAxisVector;
    mitk::FillVector3D(secondAxisVector, 0.0, 0.0, 200.0);

    mitk::Vector3D thirdAxisVector;
    mitk::FillVector3D(thirdAxisVector, 0.0, -50.0, 0.0);

    std::cout << "Coronal view direction" << std::endl;
    CPPUNIT_ASSERT(this->validateGeometry(sliceNavigationController->GetCurrentGeometry3D(), origin, firstAxisVector, secondAxisVector, thirdAxisVector));
  }

  void validateSagittalViewDirection()
  {
    auto sliceNavigationController = mitk::SliceNavigationController::New();

    sliceNavigationController->SetInputWorldGeometry3D(m_Geometry3D);
    sliceNavigationController->SetViewDirection(mitk::SliceNavigationController::Sagittal);
    sliceNavigationController->Update();

    mitk::Point3D origin;
    mitk::FillVector3D(origin, 10.5, 20.0, 30.0);

    mitk::Vector3D firstAxisVector;
    mitk::FillVector3D(firstAxisVector, 0.0, 50.0, 0.0);

    mitk::Vector3D secondAxisVector;
    mitk::FillVector3D(secondAxisVector, 0.0, 0.0, 200.0);

    mitk::Vector3D thirdAxisVector;
    mitk::FillVector3D(thirdAxisVector, 100.0, 0.0, 0.0);

    std::cout << "Sagittal view direction" << std::endl;
    CPPUNIT_ASSERT(this->validateGeometry(sliceNavigationController->GetCurrentGeometry3D(), origin, firstAxisVector, secondAxisVector, thirdAxisVector));
  }

private:
  bool validateGeometry(mitk::BaseGeometry::ConstPointer geometry, const mitk::Point3D &origin, const mitk::Vector3D &firstAxisVector, const mitk::Vector3D &secondAxisVector, const mitk::Vector3D &thirdAxisVector)
  {
    bool result = true;

    std::cout << "  Origin" << std::endl;

    if (!mitk::Equal(geometry->GetOrigin(), origin, mitk::eps, true))
      result = false;

    std::cout << "  First axis vector" << std::endl;

    if (!mitk::Equal(geometry->GetAxisVector(0), firstAxisVector, mitk::eps, true))
      result = false;

    std::cout << "  Second axis vector" << std::endl;

    if (!mitk::Equal(geometry->GetAxisVector(1), secondAxisVector, mitk::eps, true))
      result = false;

    std::cout << "  Third axis vector" << std::endl;

    if (!mitk::Equal(geometry->GetAxisVector(2), thirdAxisVector, mitk::eps, true))
      result = false;

    return result;
  }

  mitk::Geometry3D::Pointer m_Geometry3D;
};

MITK_TEST_SUITE_REGISTRATION(mitkSliceNavigationController)
