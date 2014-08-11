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

#include <mitkSurfaceInterpolationController.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkRegularPolygonSource.h>

class mitkSurfaceInterpolationControllerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceInterpolationControllerTestSuite);
  MITK_TEST(TestSingleton);
  MITK_TEST(TestSetCurrentInterpolationSession);
  MITK_TEST(TestRemoveAllInterpolationSessions);
  MITK_TEST(TestRemoveInterpolationSession);
  MITK_TEST(TestOnSegmentationDeleted);
//  MITK_TEST(TestSetSegmentationImage); //Duplicate
//  MITK_TEST(TestAddNewContour);
//  MITK_TEST(TestSetDistanceImageVolume);
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::SurfaceInterpolationController::Pointer m_Controller;

public:

  mitk::Image::Pointer createImage(unsigned int *dimensions)
  {
    mitk::Image::Pointer newImage = mitk::Image::New();
    mitk::PixelType p_type = mitk::MakeScalarPixelType<unsigned char>();
    newImage->Initialize(p_type, 3, dimensions);
    return newImage;
  }

  void setUp()
  {
    m_Controller = mitk::SurfaceInterpolationController::GetInstance();

    vtkSmartPointer<vtkRegularPolygonSource> polygonSource = vtkSmartPointer<vtkRegularPolygonSource>::New();
    polygonSource->SetRadius(100);
    polygonSource->SetNumberOfSides(7);
    polygonSource->Update();
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(polygonSource->GetOutput());
  }

  void TestSingleton()
  {
    mitk::SurfaceInterpolationController::Pointer controller2 = mitk::SurfaceInterpolationController::GetInstance();
    CPPUNIT_ASSERT_MESSAGE("SurfaceInterpolationController pointers are not equal!", m_Controller.GetPointer() == controller2.GetPointer());
  }

  void TestSetCurrentInterpolationSession()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::Image::Pointer segmentation_1 = createImage(dimensions1);

    unsigned int dimensions2[] = {20, 10, 30};
    mitk::Image::Pointer segmentation_2 = createImage(dimensions2);

    // Test 1
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not equal");
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1", m_Controller->GetNumberOfInterpolationSessions() == 1);

    // Test 2
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_2->Clone(), "Segmentation images are not equal");
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2", m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 3
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not equal");
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2", m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 4
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not equal");
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2", m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 5
    m_Controller->SetCurrentInterpolationSession(0);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().IsNull());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2", m_Controller->GetNumberOfInterpolationSessions() == 2);
  }

  void TestRemoveAllInterpolationSessions()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::Image::Pointer segmentation_1 = createImage(dimensions1);

    unsigned int dimensions2[] = {20, 10, 30};
    mitk::Image::Pointer segmentation_2 = createImage(dimensions2);

    // Test 1
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    m_Controller->RemoveAllInterpolationSessions();
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 0", m_Controller->GetNumberOfInterpolationSessions() == 0);
  }

  void TestRemoveInterpolationSession()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::Image::Pointer segmentation_1 = createImage(dimensions1);

    unsigned int dimensions2[] = {20, 10, 30};
    mitk::Image::Pointer segmentation_2 = createImage(dimensions2);

    // Test 1
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2", m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test current segmentation should not be null if another one was removed
    m_Controller->RemoveInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1", m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is null after another one was removed", m_Controller->GetCurrentSegmentation().IsNotNull());

    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2", m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test current segmentation should not be null if another one was removed
    m_Controller->RemoveInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1", m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is not null after session was removed", m_Controller->GetCurrentSegmentation().IsNull());
  }


  void TestOnSegmentationDeleted()
  {
    {
      // Create image for testing
      unsigned int dimensions1[] = {10, 10, 10};
      mitk::Image::Pointer segmentation_1 = createImage(dimensions1);
      m_Controller->SetCurrentInterpolationSession(segmentation_1);
    }
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 0", m_Controller->GetNumberOfInterpolationSessions() == 0);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkSurfaceInterpolationController)
