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

// MITK
#include <mitkIOUtil.h>
#include <mitkLookupTableProperty.h>
#include <mitkNodePredicateDataType.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

// VTK
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>

/**
  A couple of tests around Surface rendering with lookup tables (LUT).
*/
class mitkSurfaceVtkMapper2D3DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceVtkMapper2D3DTestSuite);
  MITK_TEST(RenderLUT2D);
  MITK_TEST(RenderLUT3D);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::RenderingTestHelper m_RenderingTestHelper;
  std::vector<std::string> m_CommandlineArgs;
  std::string m_PathToBall;

public:
  /**
   * @brief mitkSurfaceVtkMapper2D3DTestSuite Because the RenderingTestHelper does not have an
   * empty default constructor, we need this constructor to initialize the helper with a
   * resolution.
   */
  mitkSurfaceVtkMapper2D3DTestSuite() : m_RenderingTestHelper(300, 300) {}
  /**
   * @brief Setup Initialize a fresh rendering test helper and a vector of strings
   * to simulate commandline arguments for vtkTesting::Test.
   */
  void setUp()
  {
    m_RenderingTestHelper = mitk::RenderingTestHelper(300, 300);

    m_PathToBall = GetTestDataFilePath("ball.stl");

    // Build a command line for the vtkTesting::Test method.
    // See VTK documentation and RenderingTestHelper for more information.
    // Use the following command line option to save the difference image
    // and the test image in some tmp folder
    // m_CommandlineArgs.push_back("-T");
    // m_CommandlineArgs.push_back("/path/to/save/tmp/difference/images/");
    m_CommandlineArgs.push_back("-V");
  }

  void tearDown() {}
  // Helper method to prepare a DataNode holding a mitk::Surface
  // for rendering of point scalars via a lookup table (LUT).
  void PrepareSurfaceRenderingWithLUT(mitk::DataNode &node)
  {
    mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(node.GetData());
    CPPUNIT_ASSERT(surface);

    vtkPolyData *polydata = surface->GetVtkPolyData();
    CPPUNIT_ASSERT(polydata);

    // Build lookup table entries, associate to points of vtkPolyData
    vtkSmartPointer<vtkDoubleArray> data_array = vtkSmartPointer<vtkDoubleArray>::New();
    data_array->Initialize();
    data_array->SetName("Funny LUT entries");
    data_array->SetNumberOfComponents(1);
    auto num_points = polydata->GetNumberOfPoints(); // initialize all points in polydata
    data_array->SetNumberOfTuples(num_points);

    for (unsigned int index = 0; index != num_points; ++index)
    {
      // just assign values 0..4 to points
      // (value 0 for point idx 0..249, value 1 for idx 250..499, etc.)
      data_array->SetValue(index, (index / 250) % 5); // values 0 .. 4
    }

    polydata->GetPointData()->SetScalars(data_array);

    mitk::LookupTable::Pointer mitk_lut = mitk::LookupTable::New();
    node.SetProperty("LookupTable", mitk::LookupTableProperty::New(mitk_lut));

    node.SetBoolProperty("scalar visibility", true);
    node.SetBoolProperty("color mode", true);
    node.SetFloatProperty("ScalarsRangeMinimum", 0);
    node.SetFloatProperty("ScalarsRangeMaximum", 3);

    // build the lut
    vtkSmartPointer<vtkLookupTable> vtk_lut = mitk_lut->GetVtkLookupTable();
    if (vtk_lut == nullptr)
    {
      vtk_lut = vtkSmartPointer<vtkLookupTable>::New();
      mitk_lut->SetVtkLookupTable(vtk_lut);
    }

    // Define the lookup table.
    vtk_lut->SetTableRange(0, 3);
    vtk_lut->SetNumberOfTableValues(4);

    vtk_lut->SetTableValue(0, 1, 0, 0);
    vtk_lut->SetTableValue(1, 0, 1, 0);
    vtk_lut->SetTableValue(2, 0, 0, 1);
    vtk_lut->SetTableValue(3, 1, 1, 0);

    vtk_lut->Build();
  }

  void RenderLUT2D()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToBall)[0]);
    PrepareSurfaceRenderingWithLUT(*node);
    m_RenderingTestHelper.AddNodeToStorage(node);

    // reference screenshot for this test
    m_CommandlineArgs.push_back(GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/ballLUT2D_300x300.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    m_RenderingTestHelper.SetViewDirection(mitk::SliceNavigationController::Sagittal);
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv()) == true);
  }

  void RenderLUT3D()
  {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(mitk::IOUtil::Load(m_PathToBall)[0]);
    PrepareSurfaceRenderingWithLUT(*node);
    m_RenderingTestHelper.AddNodeToStorage(node);

    // reference screenshot for this test
    m_CommandlineArgs.push_back(GetTestDataFilePath("RenderingTestData/ReferenceScreenshots/ballLUT3D_300x300.png"));
    // Convert vector of strings to argc/argv
    mitk::RenderingTestHelper::ArgcHelperClass arg(m_CommandlineArgs);
    m_RenderingTestHelper.SetMapperIDToRender3D();
    CPPUNIT_ASSERT(m_RenderingTestHelper.CompareRenderWindowAgainstReference(arg.GetArgc(), arg.GetArgv(), 50.0) == true);
    // m_RenderingTestHelper.SaveReferenceScreenShot("c:/dev/ballLUT3D_300x300.png");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSurfaceVtkMapper2D3D)
