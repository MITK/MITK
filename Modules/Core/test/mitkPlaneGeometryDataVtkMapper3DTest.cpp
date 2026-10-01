/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIOUtil.h>
#include <mitkPlaneGeometryData.h>
#include <mitkPlaneGeometryDataVtkMapper3D.h>
#include <mitkRenderingTestHelper.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkObjectFactory.h>
#include <vtkOutputWindow.h>
#include <vtkSmartPointer.h>

namespace
{
  class ErrorCountingOutputWindow : public vtkOutputWindow
  {
  public:
    static ErrorCountingOutputWindow* New();
    vtkTypeMacro(ErrorCountingOutputWindow, vtkOutputWindow);

    void DisplayErrorText(const char*) override
    {
      ++m_NumberOfErrors;
    }

    unsigned int GetNumberOfErrors() const
    {
      return m_NumberOfErrors;
    }

  private:
    unsigned int m_NumberOfErrors = 0;
  };

  vtkStandardNewMacro(ErrorCountingOutputWindow);
}

class mitkPlaneGeometryDataVtkMapper3DTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlaneGeometryDataVtkMapper3DTestSuite);
  MITK_TEST(Render3D_NothingToClipThePlaneTo_NoVtkError);
  CPPUNIT_TEST_SUITE_END();

public:
  void Render3D_NothingToClipThePlaneTo_NoVtkError()
  {
    // Gives the scene its bounds, but is not visible everywhere and therefore leaves the
    // bounds the plane is clipped to empty.
    auto surfaceNode = mitk::DataNode::New();
    surfaceNode->SetData(mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("ball.stl")));
    surfaceNode->SetVisibility(false);

    // Like a crosshair plane: without a reference geometry and not counting for the bounds.
    auto planeGeometry = mitk::PlaneGeometry::New();
    planeGeometry->InitializeStandardPlane(100.0, 100.0);

    auto planeData = mitk::PlaneGeometryData::New();
    planeData->SetPlaneGeometry(planeGeometry);

    auto planeNode = mitk::DataNode::New();
    planeNode->SetData(planeData);
    planeNode->SetBoolProperty("includeInBoundingBox", false);

    mitk::RenderingTestHelper renderingHelper(300, 300);
    renderingHelper.AddNodeToStorage(surfaceNode);
    renderingHelper.AddNodeToStorage(planeNode);
    renderingHelper.SetMapperIDToRender3D();

    auto* mapper = dynamic_cast<mitk::PlaneGeometryDataVtkMapper3D*>(planeNode->GetMapper(mitk::BaseRenderer::Standard3D));
    CPPUNIT_ASSERT(nullptr != mapper);
    mapper->SetDataStorageForTexture(renderingHelper.GetDataStorage());

    vtkSmartPointer<vtkOutputWindow> previousOutputWindow = vtkOutputWindow::GetInstance();
    auto errorCountingOutputWindow = vtkSmartPointer<ErrorCountingOutputWindow>::New();
    vtkOutputWindow::SetInstance(errorCountingOutputWindow);

    renderingHelper.Render();

    vtkOutputWindow::SetInstance(previousOutputWindow);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The frame of a plane that is not there was rendered",
      0u, errorCountingOutputWindow->GetNumberOfErrors());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlaneGeometryDataVtkMapper3D)
