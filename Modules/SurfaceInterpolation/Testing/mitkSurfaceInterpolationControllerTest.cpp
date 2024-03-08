/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSurfaceInterpolationController.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include "mitkImageAccessByItk.h"
#include <mitkImageTimeSelector.h>
#include <mitkLabelSetImage.h>

#include <vtkDebugLeaks.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkPolygon.h>
#include <vtkRegularPolygonSource.h>

template <typename ImageType>
void ClearBufferProcessing(ImageType* itkImage)
{
  itkImage->FillBuffer(0);
}

class mitkSurfaceInterpolationControllerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceInterpolationControllerTestSuite);
  MITK_TEST(TestSingleton);
  MITK_TEST(TestSetCurrentInterpolationSession);
  MITK_TEST(TestRemoveAllInterpolationSessions);
  MITK_TEST(TestRemoveInterpolationSession);
  MITK_TEST(TestOnSegmentationDeleted);
  MITK_TEST(TestOnLabelRemoved);

  MITK_TEST(TestSetCurrentInterpolationSession4D);
  MITK_TEST(TestRemoveAllInterpolationSessions4D);
  MITK_TEST(TestRemoveInterpolationSession4D);
  MITK_TEST(TestOnSegmentationDeleted4D);

  /// \todo Workaround for memory leak in TestAddNewContour. Bug 18096.
  vtkDebugLeaks::SetExitError(0);

  MITK_TEST(TestAddNewContours);
  MITK_TEST(TestRemoveContours);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::SurfaceInterpolationController::Pointer m_Controller;

public:
  mitk::Image::Pointer createImage(unsigned int *dimensions)
  {
    mitk::Image::Pointer newImage = mitk::Image::New();
    // mitk::LabelSetImage::Pointer newImage = mitk::LabelSetImage::New();
    mitk::PixelType p_type = mitk::MakeScalarPixelType<unsigned char>();
    newImage->Initialize(p_type, 3, dimensions);
    AccessFixedDimensionByItk(newImage, ClearBufferProcessing, 3);
    return newImage;
  }

  mitk::LabelSetImage::Pointer createLabelSetImage(unsigned int *dimensions)
  {
    mitk::Image::Pointer image = createImage(dimensions);
    mitk::LabelSetImage::Pointer newImage = mitk::LabelSetImage::New();
    newImage->InitializeByLabeledImage(image);
    return newImage;
  }

  mitk::Image::Pointer createImage4D(unsigned int *dimensions)
  {
    mitk::Image::Pointer newImage = mitk::Image::New();
    mitk::PixelType p_type = mitk::MakeScalarPixelType<unsigned char>();
    newImage->Initialize(p_type, 4, dimensions);
    return newImage;
  }

  mitk::LabelSetImage::Pointer createLabelSetImage4D(unsigned int *dimensions)
  {
    mitk::Image::Pointer image = createImage4D(dimensions);
    mitk::LabelSetImage::Pointer newImage = mitk::LabelSetImage::New();
    newImage->InitializeByLabeledImage(image);
    return newImage;
  }

  void setUp() override
  {
    m_Controller = mitk::SurfaceInterpolationController::GetInstance();
    m_Controller->RemoveAllInterpolationSessions();

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
    CPPUNIT_ASSERT_MESSAGE("SurfaceInterpolationController pointers are not equal!",
                           m_Controller.GetPointer() == controller2.GetPointer());
  }

  void TestSetCurrentInterpolationSession()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage(dimensions1);

    unsigned int dimensions2[] = {20, 10, 30};
    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage(dimensions2);

    // Test 1
    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);

    // Test 2
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation() == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 3
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 4
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // // Test 5
    m_Controller->SetCurrentInterpolationSession(nullptr);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation() == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);
  }

  mitk::PlaneGeometry::Pointer CreatePlaneGeometry(mitk::ScalarType zIndex)
  {
    mitk::Point3D origin;
    mitk::Vector3D right, bottom, spacing;
    mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

    auto planegeometry = mitk::PlaneGeometry::New();
    widthInMM = 100;
    heightInMM = 200;
    thicknessInMM = 1.0;
    mitk::FillVector3D(origin, 4.5, 7.3, zIndex*thicknessInMM);
    mitk::FillVector3D(right, widthInMM, 0, 0);
    mitk::FillVector3D(bottom, 0, heightInMM, 0);
    mitk::FillVector3D(spacing, 1.0, 1.0, thicknessInMM);

    planegeometry->InitializeStandardPlane(right, bottom);
    planegeometry->SetOrigin(origin);
    planegeometry->SetSpacing(spacing);
    return planegeometry;
  }

  mitk::Surface::Pointer CreateContour(int numOfSides)
  {
    double center_1[3] = { 1.25f, 3.43f, 4.44f };
    double normal_1[3] = { 0.25f, 1.76f, 0.93f };
    vtkSmartPointer<vtkRegularPolygonSource> p_source = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source->SetNumberOfSides(numOfSides*10);
    p_source->SetCenter(center_1);
    p_source->SetRadius(4);
    p_source->SetNormal(normal_1);
    p_source->Update();
    vtkPolyData* poly_1 = p_source->GetOutput();
    mitk::Surface::Pointer surf_1 = mitk::Surface::New();
    surf_1->SetVtkPolyData(poly_1);
    return surf_1;
  }

  void TestRemoveAllInterpolationSessions()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10};
    auto segmentation_1 = createLabelSetImage(dimensions1);

    unsigned int dimensions2[] = {20, 10, 30};
    auto segmentation_2 = createLabelSetImage(dimensions2);

    // Test 1
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    m_Controller->RemoveAllInterpolationSessions();
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 0",
                           m_Controller->GetNumberOfInterpolationSessions() == 0);
  }

  void TestRemoveInterpolationSession()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage(dimensions1);

    unsigned int dimensions2[] = {20, 10, 30};
    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage(dimensions2);

    // Test 1
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test current segmentation should not be null if another one was removed
    m_Controller->RemoveInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation() == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is null after another one was removed",
                           m_Controller->GetCurrentSegmentation() != nullptr);

    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test current segmentation should not be null if another one was removed
    m_Controller->RemoveInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is not null after session was removed",
                           m_Controller->GetCurrentSegmentation() == nullptr);
  }

  void TestOnSegmentationDeleted()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10};
    auto segmentation_1 = createLabelSetImage(dimensions1);

    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    segmentation_1 = nullptr;

    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 0",
                           m_Controller->GetNumberOfInterpolationSessions() == 0);
  }

  void TestAddNewContours()
  {
    // Create segmentation image
    unsigned int dimensions1[] = {10, 10, 10, 4};
    auto segmentation_1 = createLabelSetImage4D(dimensions1);
    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    auto surf_1 = CreateContour(3);
    auto surf_2 = CreateContour(3);
    auto surf_3 = CreateContour(3);
    auto surf_3New = CreateContour(3);

    auto planeGeometry1 = CreatePlaneGeometry(1);
    auto planeGeometry2 = CreatePlaneGeometry(2);
    auto planeGeometry3 = CreatePlaneGeometry(3);

    mitk::SurfaceInterpolationController::CPIVector cpis= { {surf_1, planeGeometry1, 1, 0},
      {surf_2, planeGeometry2, 1, 0}, {surf_3, planeGeometry3, 1, 0}, {surf_3New, planeGeometry3, 1, 0},
      {surf_1, planeGeometry1, 1, 1}, {surf_1, planeGeometry1, 2, 0}, {surf_2, planeGeometry3, 2, 0}
    };
    // Add contours
    m_Controller->AddNewContours(cpis, true);

    // Check if all contours are there
    auto contours = m_Controller->GetContours(1, 0);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", contours->size() == 3);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_1->GetVtkPolyData()), *((*contours)[0].Contour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_2->GetVtkPolyData()), *((*contours)[1].Contour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_3New->GetVtkPolyData()), *((*contours)[2].Contour->GetVtkPolyData()), 0.000001, true));

    contours = m_Controller->GetContours(1, 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", contours->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_1->GetVtkPolyData()), *((*contours)[0].Contour->GetVtkPolyData()), 0.000001, true));

    contours = m_Controller->GetContours(2, 0);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", contours->size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_1->GetVtkPolyData()), *((*contours)[0].Contour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_2->GetVtkPolyData()), *((*contours)[1].Contour->GetVtkPolyData()), 0.000001, true));

    contours = m_Controller->GetContours(2, 1);
    CPPUNIT_ASSERT_MESSAGE("Invalid CPIs exists!", contours == nullptr);
    contours = m_Controller->GetContours(3, 0);
    CPPUNIT_ASSERT_MESSAGE("Invalid CPIs exists!", contours == nullptr);

    // Create another segmentation image
    unsigned int dimensions2[] = {20, 20, 20, 4};
    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage4D(dimensions2);
    m_Controller->SetCurrentInterpolationSession(segmentation_2);

    auto planeGeometry4 = CreatePlaneGeometry(4);
    auto planeGeometry5 = CreatePlaneGeometry(5);
    auto planeGeometry6 = CreatePlaneGeometry(6);

    auto surf_4 = CreateContour(4);
    auto surf_5 = CreateContour(5);
    auto surf_6 = CreateContour(6);

    mitk::SurfaceInterpolationController::CPIVector cpis2 = { {surf_1, planeGeometry1, 1, 0},
      {surf_2, planeGeometry2, 1, 0}, {surf_4, planeGeometry4, 1, 0},
      {surf_5, planeGeometry5, 1, 1}, {surf_6, planeGeometry6, 2, 0}
    };

    contours = m_Controller->GetContours(1, 0);
    CPPUNIT_ASSERT_MESSAGE("Invalid CPIs exists!", contours == nullptr);

    m_Controller->AddNewContours(cpis2);
    // Check if all contours are there
    contours = m_Controller->GetContours(1, 0);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", contours->size() == 3);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_1->GetVtkPolyData()), *((*contours)[0].Contour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_2->GetVtkPolyData()), *((*contours)[1].Contour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_4->GetVtkPolyData()), *((*contours)[2].Contour->GetVtkPolyData()), 0.000001, true));

    contours = m_Controller->GetContours(1, 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", contours->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_5->GetVtkPolyData()), *((*contours)[0].Contour->GetVtkPolyData()), 0.000001, true));

    contours = m_Controller->GetContours(2, 0);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", contours->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_6->GetVtkPolyData()), *((*contours)[0].Contour->GetVtkPolyData()), 0.000001, true));

    contours = m_Controller->GetContours(2, 1);
    CPPUNIT_ASSERT_MESSAGE("Invalid CPIs exists!", contours == nullptr);
    contours = m_Controller->GetContours(3, 0);
    CPPUNIT_ASSERT_MESSAGE("Invalid CPIs exists!", contours == nullptr);

    // Check if all contours of segmentation_1 are still there
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    contours = m_Controller->GetContours(1, 0);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", contours->size() == 3);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_1->GetVtkPolyData()), *((*contours)[0].Contour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_2->GetVtkPolyData()), *((*contours)[1].Contour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_3New->GetVtkPolyData()), *((*contours)[2].Contour->GetVtkPolyData()), 0.000001, true));

    contours = m_Controller->GetContours(1, 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", contours->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_1->GetVtkPolyData()), *((*contours)[0].Contour->GetVtkPolyData()), 0.000001, true));

    contours = m_Controller->GetContours(2, 0);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", contours->size() == 2);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_1->GetVtkPolyData()), *((*contours)[0].Contour->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
      mitk::Equal(*(surf_2->GetVtkPolyData()), *((*contours)[1].Contour->GetVtkPolyData()), 0.000001, true));

    contours = m_Controller->GetContours(2, 1);
    CPPUNIT_ASSERT_MESSAGE("Invalid CPIs exists!", contours == nullptr);
    contours = m_Controller->GetContours(3, 0);
    CPPUNIT_ASSERT_MESSAGE("Invalid CPIs exists!", contours == nullptr);
  }

  void TestRemoveContours()
  {
    // Create segmentation image
    unsigned int dimensions1[] = {12, 12, 12, 3};
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage4D(dimensions1);
    m_Controller->SetCurrentInterpolationSession(segmentation_1);


    auto surf_1 = CreateContour(3);
    auto surf_2 = CreateContour(3);
    auto surf_3 = CreateContour(3);

    auto planeGeometry1 = CreatePlaneGeometry(1);
    auto planeGeometry2 = CreatePlaneGeometry(2);
    auto planeGeometry3 = CreatePlaneGeometry(3);

    mitk::SurfaceInterpolationController::CPIVector cpis = { {surf_1, planeGeometry1, 1, 0},
      {surf_2, planeGeometry2, 1, 1}, {surf_3, planeGeometry3, 1, 2},
      {surf_1, planeGeometry1, 2, 0}, {surf_2, planeGeometry3, 2, 0}
    };
    m_Controller->AddNewContours(cpis);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 0)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 1)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 2)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0)->size() == 2);

    //Remove unkown label
    m_Controller->RemoveContours(segmentation_1, 9);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 0)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 1)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 2)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0)->size() == 2);

    //Remove unkown time step
    m_Controller->RemoveContours(segmentation_1, 1,3);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 0)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 1)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 2)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0)->size() == 2);


    m_Controller->RemoveContours(segmentation_1, 1, 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 0)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 1) == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 2)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0)->size() == 2);

    m_Controller->RemoveContours(segmentation_1, 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 0) == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 1) == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 2) == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0)->size() == 2);

    m_Controller->RemoveContours(segmentation_1, 2);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0) == nullptr);
  }

  void TestOnLabelRemoved()
  {
    // Create segmentation image
    unsigned int dimensions[] = { 20, 10, 30, 4 };
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage4D(dimensions);
    segmentation_1->AddLabel(mitk::Label::New(1, "Label1"), 0);
    segmentation_1->AddLabel(mitk::Label::New(2, "Label2"), 0);
    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage4D(dimensions);
    segmentation_2->AddLabel(mitk::Label::New(1, "Label1"), 0);
    segmentation_2->AddLabel(mitk::Label::New(2, "Label2"), 0);

    auto surf_1 = CreateContour(3);
    auto surf_2 = CreateContour(3);
    auto surf_3 = CreateContour(3);

    auto planeGeometry1 = CreatePlaneGeometry(1);
    auto planeGeometry2 = CreatePlaneGeometry(2);
    auto planeGeometry3 = CreatePlaneGeometry(3);

    mitk::SurfaceInterpolationController::CPIVector cpis = { {surf_1, planeGeometry1, 1, 0},
      {surf_2, planeGeometry2, 1, 1}, {surf_3, planeGeometry3, 1, 2},
      {surf_1, planeGeometry1, 2, 0}, {surf_2, planeGeometry3, 2, 0}
    };

    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    m_Controller->AddNewContours(cpis);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 0)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 1)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 2)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0)->size() == 2);

    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    m_Controller->AddNewContours(cpis);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 0)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 1)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 2)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0)->size() == 2);

    segmentation_1->RemoveLabel(1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 0)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 1)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 2)->size() == 1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0)->size() == 2);
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 0) == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 1) == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(1, 2) == nullptr);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetContours(2, 0)->size() == 2);
  }


  bool AssertImagesEqual4D(mitk::LabelSetImage *img1, mitk::LabelSetImage *img2)
  {
    auto selector1 = mitk::ImageTimeSelector::New();
    selector1->SetInput(img1);
    selector1->SetChannelNr(0);
    auto selector2 = mitk::ImageTimeSelector::New();
    selector2->SetInput(img2);
    selector2->SetChannelNr(0);

    int numTs1 = img1->GetTimeSteps();
    int numTs2 = img2->GetTimeSteps();
    if (numTs1 != numTs2)
    {
      return false;
    }

    /*mitk::ImagePixelWriteAccessor<unsigned char,4> accessor( img1 );
    itk::Index<4> ind;
    ind[0] = 5;
    ind[1] = 5;
    ind[2] = 5;
    ind[3] = 2;
    accessor.SetPixelByIndex( ind, 7 );*/

    for (int ts = 0; ts < numTs1; ++ts)
    {
      selector1->SetTimeNr(ts);
      selector2->SetTimeNr(ts);

      selector1->Update();
      selector2->Update();

      mitk::Image::Pointer imgSel1 = selector1->GetOutput();
      mitk::Image::Pointer imgSel2 = selector2->GetOutput();

      MITK_ASSERT_EQUAL(imgSel1, imgSel2, "Segmentation images are not equal");
    }

    return true;
  }

  void TestSetCurrentInterpolationSession4D()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10, 5};
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage4D(dimensions1);
    // mitk::Image * segmentationImage_1 = dynamic_cast<mitk::Image *>(segmentation_1.GetPointer());

    unsigned int dimensions2[] = {20, 10, 30, 4};
    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage4D(dimensions2);

    // Test 1
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    auto currentSegmentation = dynamic_cast<mitk::LabelSetImage *>(m_Controller->GetCurrentSegmentation());
    AssertImagesEqual4D(currentSegmentation, segmentation_1->Clone());
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);

    // Test 2
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    // MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_2->Clone(), "Segmentation images are not
    // equal");
    currentSegmentation = dynamic_cast<mitk::LabelSetImage *>(m_Controller->GetCurrentSegmentation());
    // AssertImagesEqual4D(currentSegmentation, segmentation_2->Clone());
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           currentSegmentation == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 3
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    // MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not
    // equal");
    currentSegmentation = dynamic_cast<mitk::LabelSetImage *>(m_Controller->GetCurrentSegmentation());
    AssertImagesEqual4D(currentSegmentation, segmentation_1->Clone());
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 4
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    // MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not
    // equal");
    currentSegmentation = dynamic_cast<mitk::LabelSetImage *>(m_Controller->GetCurrentSegmentation());
    AssertImagesEqual4D(currentSegmentation, segmentation_1->Clone());
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 5
    m_Controller->SetCurrentInterpolationSession(nullptr);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation()==nullptr);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);
  }

  void TestRemoveAllInterpolationSessions4D()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10, 4};
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage4D(dimensions1);

    unsigned int dimensions2[] = {20, 10, 30, 5};
    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage4D(dimensions2);

    // Test 1
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    m_Controller->RemoveAllInterpolationSessions();
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 0",
                           m_Controller->GetNumberOfInterpolationSessions() == 0);
  }

  void TestRemoveInterpolationSession4D()
  {
    // Create image for testing
    unsigned int dimensions1[] = {10, 10, 10, 3};
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage4D(dimensions1);

    unsigned int dimensions2[] = {20, 10, 30, 6};
    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage4D(dimensions2);

    // Test 1
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test current segmentation should not be null if another one was removed
    m_Controller->RemoveInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation() == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is null after another one was removed",
                           m_Controller->GetCurrentSegmentation()!=nullptr);

    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test current segmentation should not be null if another one was removed
    m_Controller->RemoveInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is not null after session was removed",
                           m_Controller->GetCurrentSegmentation()==nullptr);
  }

  void TestOnSegmentationDeleted4D()
  {
    {
      // Create image for testing
      unsigned int dimensions1[] = {10, 10, 10, 7};
      mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage4D(dimensions1);
      m_Controller->SetCurrentInterpolationSession(segmentation_1);
    }
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 0",
                           m_Controller->GetNumberOfInterpolationSessions() == 0);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkSurfaceInterpolationController)
