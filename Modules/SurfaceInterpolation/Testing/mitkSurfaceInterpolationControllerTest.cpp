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
#include <vtkDebugLeaks.h>

class mitkSurfaceInterpolationControllerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceInterpolationControllerTestSuite);
  MITK_TEST(TestSingleton);
  MITK_TEST(TestSetCurrentInterpolationSession);
  MITK_TEST(TestReplaceInterpolationSession);
  MITK_TEST(TestRemoveAllInterpolationSessions);
  MITK_TEST(TestRemoveInterpolationSession);
  MITK_TEST(TestOnSegmentationDeleted);

  /// \todo Workaround for memory leak in TestAddNewContour. Bug 18096.
  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(TestAddNewContour);
  MITK_TEST(TestRemoveContour);
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

  void TestReplaceInterpolationSession()
  {
    // Create segmentation image
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::Image::Pointer segmentation_1 = createImage(dimensions1);
    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    // Create some contours
    double center_1[3] = {1.25f ,3.43f ,4.44f};
    double normal_1[3] = {0.25f ,1.76f, 0.93f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source->SetNumberOfSides(20);
    p_source->SetCenter(center_1);
    p_source->SetRadius(4);
    p_source->SetNormal(normal_1);
    p_source->Update();
    vtkPolyData* poly_1 = p_source->GetOutput();
    mitk::Surface::Pointer surf_1 = mitk::Surface::New();
    surf_1->SetVtkPolyData(poly_1);

    double center_2[3] = {4.0f ,4.0f ,4.0f};
    double normal_2[3] = {1.0f ,0.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_2 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_2->SetNumberOfSides(80);
    p_source_2->SetCenter(center_2);
    p_source_2->SetRadius(4);
    p_source_2->SetNormal(normal_2);
    p_source_2->Update();
    vtkPolyData* poly_2 = p_source_2->GetOutput();
    mitk::Surface::Pointer surf_2 = mitk::Surface::New();
    surf_2->SetVtkPolyData(poly_2);

    // Add contours
    m_Controller->AddNewContour(surf_1);
    m_Controller->AddNewContour(surf_2);

    // Check if all contours are there
    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo1;
    contourInfo1.contourNormal = normal_1;
    contourInfo1.contourPoint = center_1;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo2;
    contourInfo2.contourNormal = normal_2;
    contourInfo2.contourPoint = center_2;

    mitk::Image::Pointer segmentation_2 = createImage(dimensions1);
    bool success = m_Controller->ReplaceInterpolationSession(segmentation_1, segmentation_2);
    const mitk::Surface* contour_1 = m_Controller->GetContour(contourInfo1);
    const mitk::Surface* contour_2 = m_Controller->GetContour(contourInfo2);

    CPPUNIT_ASSERT_MESSAGE("Replace session failed!", success == true);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 2);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_1->GetVtkPolyData()), *(contour_1->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_2->GetVtkPolyData()), *(contour_2->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1", m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());

    unsigned int dimensions2[] = {10, 20, 10};
    mitk::Image::Pointer segmentation_3 = createImage(dimensions2);
    success = m_Controller->ReplaceInterpolationSession(segmentation_2, segmentation_3);
    CPPUNIT_ASSERT_MESSAGE("Replace session failed!", success == false);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 2);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1", m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());
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


  void TestAddNewContour()
  {
    // Create segmentation image
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::Image::Pointer segmentation_1 = createImage(dimensions1);
    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    // Create some contours
    double center_1[3] = {1.25f ,3.43f ,4.44f};
    double normal_1[3] = {0.25f ,1.76f, 0.93f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source->SetNumberOfSides(20);
    p_source->SetCenter(center_1);
    p_source->SetRadius(4);
    p_source->SetNormal(normal_1);
    p_source->Update();
    vtkPolyData* poly_1 = p_source->GetOutput();
    mitk::Surface::Pointer surf_1 = mitk::Surface::New();
    surf_1->SetVtkPolyData(poly_1);

    double center_2[3] = {4.0f ,4.0f ,4.0f};
    double normal_2[3] = {1.0f ,0.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_2 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_2->SetNumberOfSides(80);
    p_source_2->SetCenter(center_2);
    p_source_2->SetRadius(4);
    p_source_2->SetNormal(normal_2);
    p_source_2->Update();
    vtkPolyData* poly_2 = p_source_2->GetOutput();
    mitk::Surface::Pointer surf_2 = mitk::Surface::New();
    surf_2->SetVtkPolyData(poly_2);

    double center_3[3] = {4.0f ,4.0f ,3.0f};
    double normal_3[3] = {0.0f ,0.0f, 1.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_3 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_3->SetNumberOfSides(10);
    p_source_3->SetCenter(center_3);
    p_source_3->SetRadius(4);
    p_source_3->SetNormal(normal_3);
    p_source_3->Update();
    vtkPolyData* poly_3 = p_source_3->GetOutput();
    mitk::Surface::Pointer surf_3 = mitk::Surface::New();
    surf_3->SetVtkPolyData(poly_3);

    // Add contours
    m_Controller->AddNewContour(surf_1);
    m_Controller->AddNewContour(surf_2);
    m_Controller->AddNewContour(surf_3);

    // Check if all contours are there
    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo1;
    contourInfo1.contourNormal = normal_1;
    contourInfo1.contourPoint = center_1;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo2;
    contourInfo2.contourNormal = normal_2;
    contourInfo2.contourPoint = center_2;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo3;
    contourInfo3.contourNormal = normal_3;
    contourInfo3.contourPoint = center_3;

    const mitk::Surface* contour_1 = m_Controller->GetContour(contourInfo1);
    const mitk::Surface* contour_2 = m_Controller->GetContour(contourInfo2);
    const mitk::Surface* contour_3 = m_Controller->GetContour(contourInfo3);

    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 3);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_1->GetVtkPolyData()), *(contour_1->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_2->GetVtkPolyData()), *(contour_2->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_3->GetVtkPolyData()), *(contour_3->GetVtkPolyData()), 0.000001, true));


    // Create another segmentation image
    unsigned int dimensions2[] = {20, 20, 20};
    mitk::Image::Pointer segmentation_2 = createImage(dimensions2);
    m_Controller->SetCurrentInterpolationSession(segmentation_2);

    // Create some contours
    double center_4[3] = {10.0f ,10.0f ,10.0f};
    double normal_4[3] = {0.0f ,1.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_4 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_4->SetNumberOfSides(8);
    p_source_4->SetCenter(center_4);
    p_source_4->SetRadius(5);
    p_source_4->SetNormal(normal_4);
    p_source_4->Update();
    vtkPolyData* poly_4 = p_source_4->GetOutput();
    mitk::Surface::Pointer surf_4 = mitk::Surface::New();
    surf_4->SetVtkPolyData(poly_4);

    double center_5[3] = {3.0f ,10.0f ,10.0f};
    double normal_5[3] = {1.0f ,0.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_5 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_5->SetNumberOfSides(16);
    p_source_5->SetCenter(center_5);
    p_source_5->SetRadius(8);
    p_source_5->SetNormal(normal_5);
    p_source_5->Update();
    vtkPolyData* poly_5 = p_source_5->GetOutput();
    mitk::Surface::Pointer surf_5 = mitk::Surface::New();
    surf_5->SetVtkPolyData(poly_5);

    double center_6[3] = {10.0f ,10.0f ,3.0f};
    double normal_6[3] = {0.0f ,0.0f, 1.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_6 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_6->SetNumberOfSides(100);
    p_source_6->SetCenter(center_6);
    p_source_6->SetRadius(5);
    p_source_6->SetNormal(normal_6);
    p_source_6->Update();
    vtkPolyData* poly_6 = p_source_6->GetOutput();
    mitk::Surface::Pointer surf_6 = mitk::Surface::New();
    surf_6->SetVtkPolyData(poly_6);

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo4;
    contourInfo4.contourNormal = normal_4;
    contourInfo4.contourPoint = center_4;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo5;
    contourInfo5.contourNormal = normal_5;
    contourInfo5.contourPoint = center_5;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo6;
    contourInfo6.contourNormal = normal_6;
    contourInfo6.contourPoint = center_6;

    // Add contours
    m_Controller->AddNewContour(surf_4);
    m_Controller->AddNewContour(surf_5);
    m_Controller->AddNewContour(surf_6);

    // Check if all contours are there
    mitk::Surface* contour_4 = const_cast<mitk::Surface*>(m_Controller->GetContour(contourInfo4));
    mitk::Surface* contour_5 = const_cast<mitk::Surface*>(m_Controller->GetContour(contourInfo5));
    mitk::Surface* contour_6 = const_cast<mitk::Surface*>(m_Controller->GetContour(contourInfo6));
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 3);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_4->GetVtkPolyData()), *(contour_4->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_5->GetVtkPolyData()), *(contour_5->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_6->GetVtkPolyData()), *(contour_6->GetVtkPolyData()), 0.000001, true));

    // Modify some contours
    vtkSmartPointer<vtkRegularPolygonSource> p_source_7 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_7->SetNumberOfSides(200);
    p_source_7->SetCenter(3.0,10.0,10.0);
    p_source_7->SetRadius(5);
    p_source_7->SetNormal(1, 0, 0);
    p_source_7->Update();
    vtkPolyData* poly_7 = p_source_7->GetOutput();
    mitk::Surface::Pointer surf_7 = mitk::Surface::New();
    surf_7->SetVtkPolyData(poly_7);

    m_Controller->AddNewContour(surf_7);
    mitk::Surface* contour_7 = const_cast<mitk::Surface*>(m_Controller->GetContour(contourInfo5));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_7->GetVtkPolyData()), *(contour_7->GetVtkPolyData()), 0.000001, true));

    // Change session and test if all contours are available
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    mitk::Surface* contour_8 = const_cast<mitk::Surface*>(m_Controller->GetContour(contourInfo1));
    mitk::Surface* contour_9 = const_cast<mitk::Surface*>(m_Controller->GetContour(contourInfo2));
    mitk::Surface* contour_10 = const_cast<mitk::Surface*>(m_Controller->GetContour(contourInfo3));
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 3);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_1->GetVtkPolyData()), *(contour_8->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_2->GetVtkPolyData()), *(contour_9->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!", mitk::Equal(*(surf_3->GetVtkPolyData()), *(contour_10->GetVtkPolyData()), 0.000001, true));
  }

  void TestRemoveContour()
  {
    // Create segmentation image
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::Image::Pointer segmentation_1 = createImage(dimensions1);
    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    // Create some contours
    double center_1[3] = {4.0f ,4.0f ,4.0f};
    double normal_1[3] = {0.0f ,1.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source->SetNumberOfSides(20);
    p_source->SetCenter(center_1);
    p_source->SetRadius(4);
    p_source->SetNormal(normal_1);
    p_source->Update();
    vtkPolyData* poly_1 = p_source->GetOutput();
    mitk::Surface::Pointer surf_1 = mitk::Surface::New();
    surf_1->SetVtkPolyData(poly_1);

    double center_2[3] = {4.0f ,4.0f ,4.0f};
    double normal_2[3] = {1.0f ,0.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_2 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_2->SetNumberOfSides(80);
    p_source_2->SetCenter(center_2);
    p_source_2->SetRadius(4);
    p_source_2->SetNormal(normal_2);
    p_source_2->Update();
    vtkPolyData* poly_2 = p_source_2->GetOutput();
    mitk::Surface::Pointer surf_2 = mitk::Surface::New();
    surf_2->SetVtkPolyData(poly_2);

    // Add contours
    m_Controller->AddNewContour(surf_1);
    m_Controller->AddNewContour(surf_2);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 2);

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo3;
    contourInfo3.contour = surf_1->Clone();
    contourInfo3.contourNormal = normal_1;
    contourInfo3.contourPoint = center_1;
    // Shift the new contour so that it is different
    contourInfo3.contourPoint += 0.5;

    bool success = m_Controller->RemoveContour(contourInfo3);
    CPPUNIT_ASSERT_MESSAGE("Remove failed - contour was unintentionally removed!", (m_Controller->GetNumberOfContours() == 2) && !success);

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo2;
    contourInfo2.contourNormal = normal_2;
    contourInfo2.contourPoint = center_2;
    contourInfo2.contour = surf_2;
    success = m_Controller->RemoveContour(contourInfo2);
    CPPUNIT_ASSERT_MESSAGE("Remove failed - contour was not removed!", (m_Controller->GetNumberOfContours() == 1) && success);

    // Let's see if the other contour No. 1 is still there
    contourInfo3.contourPoint -= 0.5;
    const mitk::Surface* remainingContour = m_Controller->GetContour(contourInfo3);
    CPPUNIT_ASSERT_MESSAGE("Remove failed - contour was accidentally removed!",
                           (m_Controller->GetNumberOfContours() == 1) &&
                           mitk::Equal(*(surf_1->GetVtkPolyData()), *(remainingContour->GetVtkPolyData()), 0.000001, true) &&success);

  }
};
MITK_TEST_SUITE_REGISTRATION(mitkSurfaceInterpolationController)
