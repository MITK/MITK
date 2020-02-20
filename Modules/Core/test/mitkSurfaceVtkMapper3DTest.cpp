/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkNodePredicateDataType.h>
#include <mitkRenderingTestHelper.h>
#include <mitkSmartPointerProperty.h>
#include <mitkTestingMacros.h>

#include <vtkSmartPointer.h>
#include <vtkTexturedSphereSource.h>

mitk::Surface::Pointer CreateTexturedSphere()
{
  auto source = vtkSmartPointer<vtkTexturedSphereSource>::New();
  source->SetThetaResolution(64);
  source->SetPhiResolution(32);
  source->SetRadius(16.0);
  source->Update();

  auto surface = mitk::Surface::New();
  surface->SetVtkPolyData(source->GetOutput());

  return surface;
}

void AddGeneratedDataToStorage(mitk::DataStorage *dataStorage)
{
  auto node = mitk::DataNode::New();
  node->SetData(CreateTexturedSphere());

  dataStorage->Add(node);
}

int mitkSurfaceVtkMapper3DTest(int argc, char *argv[])
{
  MITK_TEST_BEGIN("mitkSurfaceVtkMapper3DTest")

  try
  {
    mitk::RenderingTestHelper openGlTest(640, 480);
  }
  catch (const mitk::TestNotRunException &e)
  {
    MITK_WARN << "Test not run: " << e.GetDescription();
    return 77;
  }

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

  renderingHelper.SetMapperID(mitk::BaseRenderer::Standard3D);
  renderingHelper.GetVtkRenderer()->SetBackground(0.5, 0.5, 0.5);

  auto dataStorage = renderingHelper.GetDataStorage();
  AddGeneratedDataToStorage(dataStorage);

  mitk::DataNode::Pointer textureNode = dataStorage->GetNode(mitk::NodePredicateDataType::New("Image"));
  mitk::Image::Pointer texture = dynamic_cast<mitk::Image *>(textureNode->GetData());

  mitk::DataNode::Pointer surfaceNode = dataStorage->GetNode(mitk::NodePredicateDataType::New("Surface"));
  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(surfaceNode->GetData());

  auto textureProperty = mitk::SmartPointerProperty::New(texture);
  surfaceNode->SetProperty("Surface.Texture", textureProperty);

  textureNode->SetVisibility(false);

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(dataStorage);

  // use this to generate a reference screenshot or save the file:
  bool generateReferenceScreenshot = false;
  if (generateReferenceScreenshot)
  {
    renderingHelper.SaveReferenceScreenShot("/home/kilgus/output.png");
  }

  MITK_TEST_CONDITION(
    true == renderingHelper.CompareRenderWindowAgainstReference(argc, argv),
    "CompareRenderWindowAgainstReference test result positive?");

  MITK_TEST_END();
}
