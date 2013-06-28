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

//MITK
#include "mitkTestingMacros.h"
#include "mitkRenderingTestHelper.h"
#include "mitkNodePredicateDataType.h"
#include "mitkDataStorage.h"

//VTK
#include <vtkRegressionTestImage.h>


int mitkImageVtkMapper2DReinitTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // reinit render window on first image in datastorage
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkImageVtkMapper2DReinitTest")

  mitkRenderingTestHelper renderingHelper(640, 480, argc, argv);

  //perform reinit on first image in datastorage
  mitk::DataStorage::Pointer dataStorage = renderingHelper.GetDataStorage();
  mitk::RenderingManager::GetInstance()->InitializeViews(
    dataStorage->GetSubset( mitk::NodePredicateDataType::New("Image") )->GetElement(0)->GetData()->GetGeometry()
  );

  //use this to generate a reference screenshot or save the file:
  bool generateReferenceScreenshot = false;
  if(generateReferenceScreenshot)
  {
    renderingHelper.SetAutomaticallyCloseRenderWindow(false);
    renderingHelper.Render();
    renderingHelper.SaveReferenceScreenShot("C:\\Users\\schroedt\\Pictures\\RenderingTestData\\Ref.png");
  }

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

  MITK_TEST_END();
}
