/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCommon.h"
#include "mitkContour.h"
#include "mitkContourMapper2D.h"
#include "mitkDataNode.h"
#include "mitkStandaloneDataStorage.h"

#include "mitkVtkPropRenderer.h"

#include <fstream>

int mitkContourMapper2DTest(int /*argc*/, char * /*argv*/ [])
{
  mitk::Contour::Pointer contour;
  mitk::ContourMapper2D::Pointer contourMapper;
  mitk::DataNode::Pointer node;

  contourMapper = mitk::ContourMapper2D::New();
  node = mitk::DataNode::New();
  std::cout << "Testing mitk::ContourMapper2D::New(): ";

  contour = mitk::Contour::New();
  node->SetData(contour);

  if (contour.IsNull())
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }
  else
  {
    std::cout << "[PASSED]" << std::endl;
  }

  contourMapper->SetDataNode(node);
  contourMapper->Update(nullptr);
  auto *testContour = (mitk::Contour *)contourMapper->GetInput();
  std::cout << testContour << std::endl;

  mitk::StandaloneDataStorage::Pointer ds = mitk::StandaloneDataStorage::New();

  ds->Add(node);

  mitk::BoundingBox::Pointer bounds = ds->ComputeBoundingBox();

  std::cout << "bounds: " << bounds << std::endl;

  bounds = ds->ComputeVisibleBoundingBox();
  std::cout << "visible bounds: " << bounds << std::endl;

  vtkRenderWindow *renWin = vtkRenderWindow::New();

  mitk::RenderingManager::GetInstance()->SetAntiAliasing(mitk::AntiAliasing::None);
  auto renderer = mitk::VtkPropRenderer::New("ContourRenderer", renWin);

  std::cout << "Testing mitk::BaseRenderer::SetData()" << std::endl;

  renderer->SetDataStorage(ds);

  std::cout << "[TEST DONE]" << std::endl;

  renWin->Delete();

  return EXIT_SUCCESS;
}
