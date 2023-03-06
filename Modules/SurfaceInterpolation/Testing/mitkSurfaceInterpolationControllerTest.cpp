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

#include <mitkImageTimeSelector.h>
#include <mitkLabelSetImage.h>

#include <vtkDebugLeaks.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkPolygon.h>
#include <vtkRegularPolygonSource.h>

class mitkSurfaceInterpolationControllerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceInterpolationControllerTestSuite);
  MITK_TEST(TestSingleton);
  MITK_TEST(TestSetCurrentInterpolationSession);
  MITK_TEST(TestReplaceInterpolationSession);
  MITK_TEST(TestRemoveAllInterpolationSessions);
  MITK_TEST(TestRemoveInterpolationSession);
  MITK_TEST(TestOnSegmentationDeleted);

  MITK_TEST(TestSetCurrentInterpolationSession4D);
  MITK_TEST(TestReplaceInterpolationSession4D);
  MITK_TEST(TestRemoveAllInterpolationSessions4D);
  MITK_TEST(TestRemoveInterpolationSession4D);
  MITK_TEST(TestOnSegmentationDeleted4D);

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
    // mitk::LabelSetImage::Pointer newImage = mitk::LabelSetImage::New();
    mitk::PixelType p_type = mitk::MakeScalarPixelType<unsigned char>();
    newImage->Initialize(p_type, 3, dimensions);
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
    m_Controller->SetCurrentTimePoint(0);

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

    MITK_ASSERT_EQUAL(
      m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not equal");
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);

    // Test 2
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    MITK_ASSERT_EQUAL(
      m_Controller->GetCurrentSegmentation(), segmentation_2->Clone(), "Segmentation images are not equal");
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 3
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    MITK_ASSERT_EQUAL(
      m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not equal");
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 4
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    MITK_ASSERT_EQUAL(
      m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not equal");
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // // Test 5
    m_Controller->SetCurrentInterpolationSession(nullptr);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().IsNull());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);
  }

  mitk::PlaneGeometry::Pointer GetPlaneGeometry()
  {
    mitk::Point3D origin;
    mitk::Vector3D right, bottom, normal, spacing;
    mitk::ScalarType width, height;
    mitk::ScalarType widthInMM, heightInMM, thicknessInMM;

    auto planegeometry = mitk::PlaneGeometry::New();
    width = 100;
    widthInMM = width;
    height = 200;
    heightInMM = height;
    thicknessInMM = 1.0;
    mitk::FillVector3D(origin, 4.5, 7.3, 11.2);
    mitk::FillVector3D(right, widthInMM, 0, 0);
    mitk::FillVector3D(bottom, 0, heightInMM, 0);
    mitk::FillVector3D(normal, 0, 0, thicknessInMM);
    mitk::FillVector3D(spacing, 1.0, 1.0, thicknessInMM);

    planegeometry->InitializeStandardPlane(right, bottom);
    planegeometry->SetOrigin(origin);
    planegeometry->SetSpacing(spacing);
    return planegeometry;
  }

  void TestReplaceInterpolationSession()
  {
    // Create segmentation image
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage(dimensions1);

    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    // Create some contours
    double center_1[3] = {1.25f, 3.43f, 4.44f};
    double normal_1[3] = {0.25f, 1.76f, 0.93f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source->SetNumberOfSides(20);
    p_source->SetCenter(center_1);
    p_source->SetRadius(4);
    p_source->SetNormal(normal_1);
    p_source->Update();
    vtkPolyData *poly_1 = p_source->GetOutput();
    mitk::Surface::Pointer surf_1 = mitk::Surface::New();
    surf_1->SetVtkPolyData(poly_1);
    vtkSmartPointer<vtkIntArray> int1Array = vtkSmartPointer<vtkIntArray>::New();
    int1Array->InsertNextValue(1);
    int1Array->InsertNextValue(0);
    int1Array->InsertNextValue(0);
    surf_1->GetVtkPolyData()->GetFieldData()->AddArray(int1Array);
    vtkSmartPointer<vtkDoubleArray> double1Array = vtkSmartPointer<vtkDoubleArray>::New();
    double1Array->InsertNextValue(center_1[0]);
    double1Array->InsertNextValue(center_1[1]);
    double1Array->InsertNextValue(center_1[2]);
    surf_1->GetVtkPolyData()->GetFieldData()->AddArray(double1Array);

    double center_2[3] = {4.0f, 4.0f, 4.0f};
    double normal_2[3] = {1.0f, 0.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_2 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_2->SetNumberOfSides(80);
    p_source_2->SetCenter(center_2);
    p_source_2->SetRadius(4);
    p_source_2->SetNormal(normal_2);
    p_source_2->Update();
    vtkPolyData *poly_2 = p_source_2->GetOutput();
    mitk::Surface::Pointer surf_2 = mitk::Surface::New();
    surf_2->SetVtkPolyData(poly_2);
    vtkSmartPointer<vtkIntArray> int2Array = vtkSmartPointer<vtkIntArray>::New();
    int2Array->InsertNextValue(1);
    int2Array->InsertNextValue(0);
    int2Array->InsertNextValue(0);
    surf_2->GetVtkPolyData()->GetFieldData()->AddArray(int2Array);
    vtkSmartPointer<vtkDoubleArray> doubleArray = vtkSmartPointer<vtkDoubleArray>::New();
    doubleArray->InsertNextValue(center_2[0]);
    doubleArray->InsertNextValue(center_2[1]);
    doubleArray->InsertNextValue(center_2[2]);
    surf_2->GetVtkPolyData()->GetFieldData()->AddArray(doubleArray);

    std::vector<mitk::Surface::Pointer> surfaces;
    surfaces.push_back(surf_1);
    surfaces.push_back(surf_2);

    const mitk::PlaneGeometry * planeGeometry1 = GetPlaneGeometry();
    const mitk::PlaneGeometry * planeGeometry2 = GetPlaneGeometry();

    std::vector<const mitk::PlaneGeometry*> planeGeometries;
    planeGeometries.push_back(planeGeometry1);
    planeGeometries.push_back(planeGeometry2);

    // Add contours
    m_Controller->AddNewContours(surfaces, planeGeometries, true);

    // Check if all contours are there
    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo1;

    mitk::ScalarType n[3];
    vtkPolygon::ComputeNormal(surf_1->GetVtkPolyData()->GetPoints(), n);
    contourInfo1.ContourNormal = n;
    contourInfo1.ContourPoint = center_1;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo2;

    vtkPolygon::ComputeNormal(surf_2->GetVtkPolyData()->GetPoints(), n);

    contourInfo2.ContourNormal = n;
    contourInfo2.ContourPoint = center_2;


    const mitk::Surface *contour_1 = m_Controller->GetContour(contourInfo1);
    const mitk::Surface *contour_2 = m_Controller->GetContour(contourInfo2);


    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 2);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_1->GetVtkPolyData()), *(contour_1->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_2->GetVtkPolyData()), *(contour_2->GetVtkPolyData()), 0.000001, true));

    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);

    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage(dimensions1);
    bool success = m_Controller->ReplaceInterpolationSession(segmentation_1, segmentation_2);

    CPPUNIT_ASSERT_MESSAGE("Replace session failed!", success == true);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());

    unsigned int dimensions2[] = {10, 20, 10};
    mitk::Image::Pointer segmentation_3 = createLabelSetImage(dimensions2);
    success = m_Controller->ReplaceInterpolationSession(segmentation_1, segmentation_3);
    CPPUNIT_ASSERT_MESSAGE("Replace session failed!", success == false);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());
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
    mitk::Image::Pointer segmentation_2 = createLabelSetImage(dimensions2);

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
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is null after another one was removed",
                           m_Controller->GetCurrentSegmentation().IsNotNull());

    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test current segmentation should not be null if another one was removed
    m_Controller->RemoveInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is not null after session was removed",
                           m_Controller->GetCurrentSegmentation().IsNull());
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

  void TestAddNewContour()
  {
    // Create segmentation image
    unsigned int dimensions1[] = {10, 10, 10};
    mitk::Image::Pointer segmentation_1 = createLabelSetImage(dimensions1);
    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    // Create some contours
    double center_1[3] = {1.25f, 3.43f, 4.44f};
    double normal_1[3] = {0.25f, 1.76f, 0.93f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source->SetNumberOfSides(20);
    p_source->SetCenter(center_1);
    p_source->SetRadius(4);
    p_source->SetNormal(normal_1);
    p_source->Update();
    vtkPolyData *poly_1 = p_source->GetOutput();
    mitk::Surface::Pointer surf_1 = mitk::Surface::New();
    surf_1->SetVtkPolyData(poly_1);
    vtkSmartPointer<vtkIntArray> int1Array = vtkSmartPointer<vtkIntArray>::New();
    int1Array->InsertNextValue(1);
    int1Array->InsertNextValue(0);
    int1Array->InsertNextValue(0);
    surf_1->GetVtkPolyData()->GetFieldData()->AddArray(int1Array);
    vtkSmartPointer<vtkDoubleArray> double1Array = vtkSmartPointer<vtkDoubleArray>::New();
    double1Array->InsertNextValue(center_1[0]);
    double1Array->InsertNextValue(center_1[1]);
    double1Array->InsertNextValue(center_1[2]);
    surf_1->GetVtkPolyData()->GetFieldData()->AddArray(double1Array);

    double center_2[3] = {4.0f, 4.0f, 4.0f};
    double normal_2[3] = {1.0f, 0.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_2 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_2->SetNumberOfSides(80);
    p_source_2->SetCenter(center_2);
    p_source_2->SetRadius(4);
    p_source_2->SetNormal(normal_2);
    p_source_2->Update();
    vtkPolyData *poly_2 = p_source_2->GetOutput();
    mitk::Surface::Pointer surf_2 = mitk::Surface::New();
    surf_2->SetVtkPolyData(poly_2);
    vtkSmartPointer<vtkIntArray> int2Array = vtkSmartPointer<vtkIntArray>::New();
    int2Array->InsertNextValue(1);
    int2Array->InsertNextValue(0);
    int2Array->InsertNextValue(0);
    surf_2->GetVtkPolyData()->GetFieldData()->AddArray(int2Array);
    vtkSmartPointer<vtkDoubleArray> double2Array = vtkSmartPointer<vtkDoubleArray>::New();
    double2Array->InsertNextValue(center_2[0]);
    double2Array->InsertNextValue(center_2[1]);
    double2Array->InsertNextValue(center_2[2]);
    surf_2->GetVtkPolyData()->GetFieldData()->AddArray(double2Array);

    double center_3[3] = {4.0f, 4.0f, 3.0f};
    double normal_3[3] = {0.0f, 0.0f, 1.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_3 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_3->SetNumberOfSides(10);
    p_source_3->SetCenter(center_3);
    p_source_3->SetRadius(4);
    p_source_3->SetNormal(normal_3);
    p_source_3->Update();
    vtkPolyData *poly_3 = p_source_3->GetOutput();
    mitk::Surface::Pointer surf_3 = mitk::Surface::New();
    surf_3->SetVtkPolyData(poly_3);
    vtkSmartPointer<vtkIntArray> int3Array = vtkSmartPointer<vtkIntArray>::New();
    int3Array->InsertNextValue(1);
    int3Array->InsertNextValue(0);
    int3Array->InsertNextValue(0);
    surf_3->GetVtkPolyData()->GetFieldData()->AddArray(int3Array);
    vtkSmartPointer<vtkDoubleArray> double3Array = vtkSmartPointer<vtkDoubleArray>::New();
    double3Array->InsertNextValue(center_3[0]);
    double3Array->InsertNextValue(center_3[1]);
    double3Array->InsertNextValue(center_3[2]);
    surf_3->GetVtkPolyData()->GetFieldData()->AddArray(double3Array);

    std::vector<mitk::Surface::Pointer> surfaces;
    surfaces.push_back(surf_1);
    surfaces.push_back(surf_2);
    surfaces.push_back(surf_3);

    const mitk::PlaneGeometry * planeGeometry1 = GetPlaneGeometry();
    const mitk::PlaneGeometry * planeGeometry2 = GetPlaneGeometry();
    const mitk::PlaneGeometry * planeGeometry3 = GetPlaneGeometry();

    std::vector<const mitk::PlaneGeometry*> planeGeometries;
    planeGeometries.push_back(planeGeometry1);
    planeGeometries.push_back(planeGeometry2);
    planeGeometries.push_back(planeGeometry3);

    // Add contours
    m_Controller->AddNewContours(surfaces, planeGeometries, true);

    mitk::ScalarType n[3];

    // Check if all contours are there
    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo1;
    vtkPolygon::ComputeNormal(surf_1->GetVtkPolyData()->GetPoints(), n);
    contourInfo1.ContourNormal = n;
    contourInfo1.ContourPoint = center_1;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo2;
    vtkPolygon::ComputeNormal(surf_2->GetVtkPolyData()->GetPoints(), n);
    contourInfo2.ContourNormal = n;
    contourInfo2.ContourPoint = center_2;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo3;
    vtkPolygon::ComputeNormal(surf_3->GetVtkPolyData()->GetPoints(), n);
    contourInfo3.ContourNormal = n;
    contourInfo3.ContourPoint = center_3;

    const mitk::Surface *contour_1 = m_Controller->GetContour(contourInfo1);
    const mitk::Surface *contour_2 = m_Controller->GetContour(contourInfo2);
    const mitk::Surface *contour_3 = m_Controller->GetContour(contourInfo3);

    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 3);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_1->GetVtkPolyData()), *(contour_1->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_2->GetVtkPolyData()), *(contour_2->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_3->GetVtkPolyData()), *(contour_3->GetVtkPolyData()), 0.000001, true));

    // Create another segmentation image
    unsigned int dimensions2[] = {20, 20, 20};
    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage(dimensions2);
    m_Controller->SetCurrentInterpolationSession(segmentation_2);

    // Create some contours
    double center_4[3] = {10.0f, 10.0f, 10.0f};
    double normal_4[3] = {0.0f, 1.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_4 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_4->SetNumberOfSides(8);
    p_source_4->SetCenter(center_4);
    p_source_4->SetRadius(5);
    p_source_4->SetNormal(normal_4);
    p_source_4->Update();
    vtkPolyData *poly_4 = p_source_4->GetOutput();
    mitk::Surface::Pointer surf_4 = mitk::Surface::New();
    surf_4->SetVtkPolyData(poly_4);
    vtkSmartPointer<vtkIntArray> int4Array = vtkSmartPointer<vtkIntArray>::New();
    int4Array->InsertNextValue(2);
    int4Array->InsertNextValue(0);
    int4Array->InsertNextValue(0);
    surf_4->GetVtkPolyData()->GetFieldData()->AddArray(int4Array);
    vtkSmartPointer<vtkDoubleArray> double4Array = vtkSmartPointer<vtkDoubleArray>::New();
    double4Array->InsertNextValue(center_4[0]);
    double4Array->InsertNextValue(center_4[1]);
    double4Array->InsertNextValue(center_4[2]);
    surf_4->GetVtkPolyData()->GetFieldData()->AddArray(double4Array);

    double center_5[3] = {3.0f, 10.0f, 10.0f};
    double normal_5[3] = {1.0f, 0.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_5 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_5->SetNumberOfSides(16);
    p_source_5->SetCenter(center_5);
    p_source_5->SetRadius(8);
    p_source_5->SetNormal(normal_5);
    p_source_5->Update();
    vtkPolyData *poly_5 = p_source_5->GetOutput();
    mitk::Surface::Pointer surf_5 = mitk::Surface::New();
    surf_5->SetVtkPolyData(poly_5);
    vtkSmartPointer<vtkIntArray> int5Array = vtkSmartPointer<vtkIntArray>::New();
    int5Array->InsertNextValue(2);
    int5Array->InsertNextValue(0);
    int5Array->InsertNextValue(0);
    surf_5->GetVtkPolyData()->GetFieldData()->AddArray(int5Array);
    vtkSmartPointer<vtkDoubleArray> double5Array = vtkSmartPointer<vtkDoubleArray>::New();
    double5Array->InsertNextValue(center_5[0]);
    double5Array->InsertNextValue(center_5[1]);
    double5Array->InsertNextValue(center_5[2]);
    surf_5->GetVtkPolyData()->GetFieldData()->AddArray(double5Array);

    double center_6[3] = {10.0f, 10.0f, 3.0f};
    double normal_6[3] = {0.0f, 0.0f, 1.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_6 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_6->SetNumberOfSides(100);
    p_source_6->SetCenter(center_6);
    p_source_6->SetRadius(5);
    p_source_6->SetNormal(normal_6);
    p_source_6->Update();
    vtkPolyData *poly_6 = p_source_6->GetOutput();
    mitk::Surface::Pointer surf_6 = mitk::Surface::New();
    surf_6->SetVtkPolyData(poly_6);
    vtkSmartPointer<vtkIntArray> int6Array = vtkSmartPointer<vtkIntArray>::New();
    int6Array->InsertNextValue(2);
    int6Array->InsertNextValue(0);
    int6Array->InsertNextValue(0);
    surf_6->GetVtkPolyData()->GetFieldData()->AddArray(int6Array);
    vtkSmartPointer<vtkDoubleArray> double6Array = vtkSmartPointer<vtkDoubleArray>::New();
    double6Array->InsertNextValue(center_6[0]);
    double6Array->InsertNextValue(center_6[1]);
    double6Array->InsertNextValue(center_6[2]);
    surf_6->GetVtkPolyData()->GetFieldData()->AddArray(double6Array);

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo4;
    vtkPolygon::ComputeNormal(surf_4->GetVtkPolyData()->GetPoints(), n);
    contourInfo4.ContourNormal = n;
    contourInfo4.ContourPoint = center_4;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo5;
    vtkPolygon::ComputeNormal(surf_5->GetVtkPolyData()->GetPoints(), n);
    contourInfo5.ContourNormal = n;
    contourInfo5.ContourPoint = center_5;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo6;
    vtkPolygon::ComputeNormal(surf_6->GetVtkPolyData()->GetPoints(), n);
    contourInfo6.ContourNormal = n;
    contourInfo6.ContourPoint = center_6;

    const mitk::PlaneGeometry * planeGeometry4 = GetPlaneGeometry();
    const mitk::PlaneGeometry * planeGeometry5 = GetPlaneGeometry();
    const mitk::PlaneGeometry * planeGeometry6 = GetPlaneGeometry();

    std::vector<mitk::Surface::Pointer> surfaces2;
    surfaces2.push_back(surf_4);
    surfaces2.push_back(surf_5);
    surfaces2.push_back(surf_6);

    std::vector<const mitk::PlaneGeometry*> planeGeometries2;
    planeGeometries2.push_back(planeGeometry4);
    planeGeometries2.push_back(planeGeometry5);
    planeGeometries2.push_back(planeGeometry6);

    m_Controller->AddNewContours(surfaces2, planeGeometries2, true);

    // Check if all contours are there
    auto contour_4 = m_Controller->GetContour(contourInfo4);
    auto contour_5 = m_Controller->GetContour(contourInfo5);
    auto contour_6 = m_Controller->GetContour(contourInfo6);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 3);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_4->GetVtkPolyData()), *(contour_4->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_5->GetVtkPolyData()), *(contour_5->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_6->GetVtkPolyData()), *(contour_6->GetVtkPolyData()), 0.000001, true));

    // Modify some contours
    vtkSmartPointer<vtkRegularPolygonSource> p_source_7 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_7->SetNumberOfSides(200);
    p_source_7->SetCenter(3.0, 10.0, 10.0);
    p_source_7->SetRadius(5);
    p_source_7->SetNormal(1, 0, 0);
    p_source_7->Update();
    vtkPolyData *poly_7 = p_source_7->GetOutput();
    mitk::Surface::Pointer surf_7 = mitk::Surface::New();
    surf_7->SetVtkPolyData(poly_7);
    vtkSmartPointer<vtkIntArray> int7Array = vtkSmartPointer<vtkIntArray>::New();
    int7Array->InsertNextValue(2);
    int7Array->InsertNextValue(0);
    int7Array->InsertNextValue(0);
    surf_7->GetVtkPolyData()->GetFieldData()->AddArray(int7Array);
    vtkSmartPointer<vtkDoubleArray> double7Array = vtkSmartPointer<vtkDoubleArray>::New();
    double7Array->InsertNextValue(3.0);
    double7Array->InsertNextValue(10.0);
    double7Array->InsertNextValue(10.0);
    surf_7->GetVtkPolyData()->GetFieldData()->AddArray(double7Array);


    std::vector<mitk::Surface::Pointer> surfaces3;
    surfaces3.push_back(surf_7);

    const mitk::PlaneGeometry * planeGeometry7 = GetPlaneGeometry();
    std::vector<const mitk::PlaneGeometry*> planeGeometries3;
    planeGeometries3.push_back(planeGeometry7);

    m_Controller->AddNewContours(surfaces3, planeGeometries3, true);

    mitk::ScalarType center_7[3];
    center_7[0] = 3.0;
    center_7[1] = 10.0;
    center_7[2] = 10.0;
    vtkPolygon::ComputeNormal(surf_7->GetVtkPolyData()->GetPoints(), n);
    contourInfo5.ContourNormal = n;
    contourInfo5.ContourPoint = center_7;

    auto contour_7 = m_Controller->GetContour(contourInfo5);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_7->GetVtkPolyData()), *(contour_7->GetVtkPolyData()), 0.000001, true));

    // Change session and test if all contours are available
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    auto contour_8 = m_Controller->GetContour(contourInfo1);
    auto contour_9 = m_Controller->GetContour(contourInfo2);
    auto contour_10 = m_Controller->GetContour(contourInfo3);
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 3);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_1->GetVtkPolyData()), *(contour_8->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_2->GetVtkPolyData()), *(contour_9->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_3->GetVtkPolyData()), *(contour_10->GetVtkPolyData()), 0.000001, true));
  }

  void TestRemoveContour()
  {
    // Create segmentation image
    unsigned int dimensions1[] = {12, 12, 12};
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage(dimensions1);
    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    // Create some contours
    double center_1[3] = {4.0f, 4.0f, 4.0f};
    double normal_1[3] = {0.0f, 1.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source->SetNumberOfSides(20);
    p_source->SetCenter(center_1);
    p_source->SetRadius(4);
    p_source->SetNormal(normal_1);
    p_source->Update();
    vtkPolyData *poly_1 = p_source->GetOutput();
    mitk::Surface::Pointer surf_1 = mitk::Surface::New();
    surf_1->SetVtkPolyData(poly_1);
    vtkSmartPointer<vtkIntArray> int1Array = vtkSmartPointer<vtkIntArray>::New();
    int1Array->InsertNextValue(1);
    int1Array->InsertNextValue(0);
    int1Array->InsertNextValue(0);
    surf_1->GetVtkPolyData()->GetFieldData()->AddArray(int1Array);
    vtkSmartPointer<vtkDoubleArray> double1Array = vtkSmartPointer<vtkDoubleArray>::New();
    double1Array->InsertNextValue(center_1[0]);
    double1Array->InsertNextValue(center_1[1]);
    double1Array->InsertNextValue(center_1[2]);
    surf_1->GetVtkPolyData()->GetFieldData()->AddArray(double1Array);

    double center_2[3] = {4.0f, 4.0f, 4.0f};
    double normal_2[3] = {1.0f, 0.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_2 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_2->SetNumberOfSides(80);
    p_source_2->SetCenter(center_2);
    p_source_2->SetRadius(4);
    p_source_2->SetNormal(normal_2);
    p_source_2->Update();
    vtkPolyData *poly_2 = p_source_2->GetOutput();
    mitk::Surface::Pointer surf_2 = mitk::Surface::New();
    surf_2->SetVtkPolyData(poly_2);
    vtkSmartPointer<vtkIntArray> int2Array = vtkSmartPointer<vtkIntArray>::New();
    int2Array->InsertNextValue(1);
    int2Array->InsertNextValue(0);
    int2Array->InsertNextValue(0);
    surf_2->GetVtkPolyData()->GetFieldData()->AddArray(int2Array);
    vtkSmartPointer<vtkDoubleArray> double2Array = vtkSmartPointer<vtkDoubleArray>::New();
    double2Array->InsertNextValue(center_2[0]);
    double2Array->InsertNextValue(center_2[1]);
    double2Array->InsertNextValue(center_2[2]);
    surf_2->GetVtkPolyData()->GetFieldData()->AddArray(double2Array);

    std::vector<mitk::Surface::Pointer> surfaces;
    surfaces.push_back(surf_1);
    surfaces.push_back(surf_2);

    const mitk::PlaneGeometry * planeGeometry1 = GetPlaneGeometry();
    const mitk::PlaneGeometry * planeGeometry2 = GetPlaneGeometry();
    std::vector<const mitk::PlaneGeometry*> planeGeometries;
    planeGeometries.push_back(planeGeometry1);
    planeGeometries.push_back(planeGeometry2);

    m_Controller->AddNewContours(surfaces, planeGeometries, true);
    // // Add contours
    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 2);

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo3;
    contourInfo3.Contour = surf_1->Clone();
    contourInfo3.ContourNormal = normal_1;
    contourInfo3.ContourPoint = center_1;
    // Shift the new contour so that it is different
    contourInfo3.ContourPoint += 0.5;

    bool success = m_Controller->RemoveContour(contourInfo3);
    CPPUNIT_ASSERT_MESSAGE("Remove failed - contour was unintentionally removed!",
                           (m_Controller->GetNumberOfContours() == 2) && !success);

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo2;
    contourInfo2.ContourNormal = normal_2;
    contourInfo2.ContourPoint = center_2;
    contourInfo2.Contour = surf_2;
    success = m_Controller->RemoveContour(contourInfo2);
    CPPUNIT_ASSERT_MESSAGE("Remove failed - contour was not removed!",
                           (m_Controller->GetNumberOfContours() == 1) && success);

    // // Let's see if the other contour No. 1 is still there
    contourInfo3.ContourPoint -= 0.5;
    const mitk::Surface *remainingContour = m_Controller->GetContour(contourInfo3);
    CPPUNIT_ASSERT_MESSAGE(
      "Remove failed - contour was accidentally removed!",
      (m_Controller->GetNumberOfContours() == 1) &&
        mitk::Equal(*(surf_1->GetVtkPolyData()), *(remainingContour->GetVtkPolyData()), 0.000001, true) && success);
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
    auto currentSegmentation = dynamic_cast<mitk::LabelSetImage *>(m_Controller->GetCurrentSegmentation().GetPointer());
    AssertImagesEqual4D(currentSegmentation, segmentation_1->Clone());
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);

    // Test 2
    m_Controller->SetCurrentInterpolationSession(segmentation_2);
    // MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_2->Clone(), "Segmentation images are not
    // equal");
    currentSegmentation = dynamic_cast<mitk::LabelSetImage *>(m_Controller->GetCurrentSegmentation().GetPointer());
    // AssertImagesEqual4D(currentSegmentation, segmentation_2->Clone());
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           currentSegmentation == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 3
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    // MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not
    // equal");
    currentSegmentation = dynamic_cast<mitk::LabelSetImage *>(m_Controller->GetCurrentSegmentation().GetPointer());
    AssertImagesEqual4D(currentSegmentation, segmentation_1->Clone());
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 4
    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    // MITK_ASSERT_EQUAL(m_Controller->GetCurrentSegmentation(), segmentation_1->Clone(), "Segmentation images are not
    // equal");
    currentSegmentation = dynamic_cast<mitk::LabelSetImage *>(m_Controller->GetCurrentSegmentation().GetPointer());
    AssertImagesEqual4D(currentSegmentation, segmentation_1->Clone());
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_1.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test 5
    m_Controller->SetCurrentInterpolationSession(nullptr);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal", m_Controller->GetCurrentSegmentation().IsNull());
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);
  }

  void TestReplaceInterpolationSession4D()
  {
    // Create segmentation image
    unsigned int dimensions1[] = {10, 10, 10, 5};
    mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage4D(dimensions1);
    m_Controller->SetCurrentInterpolationSession(segmentation_1);

    m_Controller->SetCurrentTimePoint(0);

    // Create some contours
    double center_1[3] = {1.25f, 3.43f, 4.44f};
    double normal_1[3] = {0.25f, 1.76f, 0.93f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source->SetNumberOfSides(20);
    p_source->SetCenter(center_1);
    p_source->SetRadius(4);
    p_source->SetNormal(normal_1);
    p_source->Update();
    vtkPolyData *poly_1 = p_source->GetOutput();
    mitk::Surface::Pointer surf_1 = mitk::Surface::New();
    surf_1->SetVtkPolyData(poly_1);
    vtkSmartPointer<vtkIntArray> int1Array = vtkSmartPointer<vtkIntArray>::New();
    int1Array->InsertNextValue(1);
    int1Array->InsertNextValue(0);
    int1Array->InsertNextValue(0);
    surf_1->GetVtkPolyData()->GetFieldData()->AddArray(int1Array);
    vtkSmartPointer<vtkDoubleArray> double1Array = vtkSmartPointer<vtkDoubleArray>::New();
    double1Array->InsertNextValue(center_1[0]);
    double1Array->InsertNextValue(center_1[1]);
    double1Array->InsertNextValue(center_1[2]);
    surf_1->GetVtkPolyData()->GetFieldData()->AddArray(double1Array);

    double center_2[3] = {4.0f, 4.0f, 4.0f};
    double normal_2[3] = {1.0f, 0.0f, 0.0f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_2 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_2->SetNumberOfSides(80);
    p_source_2->SetCenter(center_2);
    p_source_2->SetRadius(4);
    p_source_2->SetNormal(normal_2);
    p_source_2->Update();
    vtkPolyData *poly_2 = p_source_2->GetOutput();
    mitk::Surface::Pointer surf_2 = mitk::Surface::New();
    surf_2->SetVtkPolyData(poly_2);
    vtkSmartPointer<vtkIntArray> int2Array = vtkSmartPointer<vtkIntArray>::New();
    int2Array->InsertNextValue(1);
    int2Array->InsertNextValue(0);
    int2Array->InsertNextValue(0);
    surf_2->GetVtkPolyData()->GetFieldData()->AddArray(int2Array);
    vtkSmartPointer<vtkDoubleArray> double2Array = vtkSmartPointer<vtkDoubleArray>::New();
    double2Array->InsertNextValue(center_2[0]);
    double2Array->InsertNextValue(center_2[1]);
    double2Array->InsertNextValue(center_2[2]);
    surf_2->GetVtkPolyData()->GetFieldData()->AddArray(double2Array);


    std::vector<mitk::Surface::Pointer> surfaces;
    surfaces.push_back(surf_1);
    surfaces.push_back(surf_2);

    const mitk::PlaneGeometry * planeGeometry1 = GetPlaneGeometry();
    const mitk::PlaneGeometry * planeGeometry2 = GetPlaneGeometry();
    std::vector<const mitk::PlaneGeometry*> planeGeometries;
    planeGeometries.push_back(planeGeometry1);
    planeGeometries.push_back(planeGeometry2);
    // Add contours
    m_Controller->AddNewContours(surfaces, planeGeometries, true);


    // Add contours for another timestep
    m_Controller->SetCurrentTimePoint(2);

    double center_3[3] = {1.3f, 3.5f, 4.6f};
    double normal_3[3] = {0.20f, 1.6f, 0.8f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_3 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_3->SetNumberOfSides(20);
    p_source_3->SetCenter(center_3);
    p_source_3->SetRadius(4);
    p_source_3->SetNormal(normal_3);
    p_source_3->Update();
    vtkPolyData *poly_3 = p_source_3->GetOutput();
    mitk::Surface::Pointer surf_3 = mitk::Surface::New();
    surf_3->SetVtkPolyData(poly_3);
    vtkSmartPointer<vtkIntArray> int3Array = vtkSmartPointer<vtkIntArray>::New();
    int3Array->InsertNextValue(1);
    int3Array->InsertNextValue(0);
    int3Array->InsertNextValue(2);
    surf_3->GetVtkPolyData()->GetFieldData()->AddArray(int3Array);
    vtkSmartPointer<vtkDoubleArray> double3Array = vtkSmartPointer<vtkDoubleArray>::New();
    double3Array->InsertNextValue(center_3[0]);
    double3Array->InsertNextValue(center_3[1]);
    double3Array->InsertNextValue(center_3[2]);
    surf_3->GetVtkPolyData()->GetFieldData()->AddArray(double3Array);

    double center_4[3] = {1.32f, 3.53f, 4.8f};
    double normal_4[3] = {0.22f, 1.5f, 0.85f};
    vtkSmartPointer<vtkRegularPolygonSource> p_source_4 = vtkSmartPointer<vtkRegularPolygonSource>::New();
    p_source_4->SetNumberOfSides(20);
    p_source_4->SetCenter(center_4);
    p_source_4->SetRadius(4);
    p_source_4->SetNormal(normal_4);
    p_source_4->Update();
    vtkPolyData *poly_4 = p_source_4->GetOutput();
    mitk::Surface::Pointer surf_4 = mitk::Surface::New();
    surf_4->SetVtkPolyData(poly_4);
    vtkSmartPointer<vtkIntArray> int4Array = vtkSmartPointer<vtkIntArray>::New();
    int4Array->InsertNextValue(1);
    int4Array->InsertNextValue(0);
    int4Array->InsertNextValue(2);
    surf_4->GetVtkPolyData()->GetFieldData()->AddArray(int4Array);
    vtkSmartPointer<vtkDoubleArray> double4Array = vtkSmartPointer<vtkDoubleArray>::New();
    double4Array->InsertNextValue(center_4[0]);
    double4Array->InsertNextValue(center_4[1]);
    double4Array->InsertNextValue(center_4[2]);
    surf_4->GetVtkPolyData()->GetFieldData()->AddArray(double4Array);


    std::vector<mitk::Surface::Pointer> surfaces2;
    surfaces2.push_back(surf_3);
    surfaces2.push_back(surf_4);

    const mitk::PlaneGeometry * planeGeometry3 = GetPlaneGeometry();
    const mitk::PlaneGeometry * planeGeometry4 = GetPlaneGeometry();
    std::vector<const mitk::PlaneGeometry*> planeGeometries2;
    planeGeometries2.push_back(planeGeometry3);
    planeGeometries2.push_back(planeGeometry4);
    // Add contours
    m_Controller->AddNewContours(surfaces2, planeGeometries2, true);

    m_Controller->SetCurrentTimePoint(0);

    // Check if all contours are there
    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo1;
    contourInfo1.ContourNormal = normal_1;
    contourInfo1.ContourPoint = center_1;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo2;
    contourInfo2.ContourNormal = normal_2;
    contourInfo2.ContourPoint = center_2;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo3;
    mitk::ScalarType n[3];
    vtkPolygon::ComputeNormal(surf_3->GetVtkPolyData()->GetPoints(), n);
    contourInfo3.ContourNormal = n;
    contourInfo3.ContourPoint = center_3;

    mitk::SurfaceInterpolationController::ContourPositionInformation contourInfo4;
    // mitk::ScalarType n[3];
    vtkPolygon::ComputeNormal(surf_4->GetVtkPolyData()->GetPoints(), n);
    contourInfo4.ContourNormal = n;
    contourInfo4.ContourPoint = center_4;

    const mitk::Surface *contour_1 = m_Controller->GetContour(contourInfo1);
    const mitk::Surface *contour_2 = m_Controller->GetContour(contourInfo2);

    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_1->GetVtkPolyData()), *(contour_1->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_2->GetVtkPolyData()), *(contour_2->GetVtkPolyData()), 0.000001, true));

    m_Controller->SetCurrentTimePoint(2);
    const mitk::Surface *contour_3 = m_Controller->GetContour(contourInfo3);
    const mitk::Surface *contour_4 = m_Controller->GetContour(contourInfo4);

    CPPUNIT_ASSERT_MESSAGE("Wrong number of contours!", m_Controller->GetNumberOfContours() == 2);
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_3->GetVtkPolyData()), *(contour_3->GetVtkPolyData()), 0.000001, true));
    CPPUNIT_ASSERT_MESSAGE("Contours not equal!",
                           mitk::Equal(*(surf_4->GetVtkPolyData()), *(contour_4->GetVtkPolyData()), 0.000001, true));

    mitk::LabelSetImage::Pointer segmentation_2 = createLabelSetImage4D(dimensions1);
    bool success = m_Controller->ReplaceInterpolationSession(segmentation_1, segmentation_2);

    CPPUNIT_ASSERT_MESSAGE("Replace session failed!", success == true);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Segmentation images are not equal",
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());
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
                           m_Controller->GetCurrentSegmentation().GetPointer() == segmentation_2.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is null after another one was removed",
                           m_Controller->GetCurrentSegmentation().IsNotNull());

    m_Controller->SetCurrentInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 2",
                           m_Controller->GetNumberOfInterpolationSessions() == 2);

    // Test current segmentation should not be null if another one was removed
    m_Controller->RemoveInterpolationSession(segmentation_1);
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 1",
                           m_Controller->GetNumberOfInterpolationSessions() == 1);
    CPPUNIT_ASSERT_MESSAGE("Current segmentation is not null after session was removed",
                           m_Controller->GetCurrentSegmentation().IsNull());
  }

  void TestOnSegmentationDeleted4D()
  {
    {
      // Create image for testing
      unsigned int dimensions1[] = {10, 10, 10, 7};
      mitk::LabelSetImage::Pointer segmentation_1 = createLabelSetImage4D(dimensions1);
      m_Controller->SetCurrentInterpolationSession(segmentation_1);
      m_Controller->SetCurrentTimePoint(3);
    }
    CPPUNIT_ASSERT_MESSAGE("Number of interpolation session not 0",
                           m_Controller->GetNumberOfInterpolationSessions() == 0);
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkSurfaceInterpolationController)
