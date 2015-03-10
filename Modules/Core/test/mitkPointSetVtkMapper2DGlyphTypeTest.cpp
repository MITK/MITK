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
#include <mitkNodePredicateDataType.h>
#include <mitkEnumerationProperty.h>
#include <mitkBaseProperty.h>
#include <mitkPointSet.h>

//VTK
#include <vtkRegressionTestImage.h>


int mitkPointSetVtkMapper2DGlyphTypeTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkPointSetVtkMapper2DGlyphTypeTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

  renderingHelper.SetViewDirection(mitk::SliceNavigationController::Sagittal);

  mitk::EnumerationProperty* eP = dynamic_cast<mitk::EnumerationProperty*> (renderingHelper.GetDataStorage()->GetNode(mitk::NodePredicateDataType::New("PointSet"))->GetProperty("Pointset.2D.shape"));
  // render triangles instead of crosses
  eP->SetValue(5);

  // disables anti-aliasing which is enabled on several graphics cards and
  // causes problems when doing a pixel-wise comparison to a reference image
  renderingHelper.GetVtkRenderWindow()->SetMultiSamples(0);

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv) == true, "CompareRenderWindowAgainstReference test result positive?" );

 //use this to generate a reference screenshot or save the file:
  if(false)
  {
    renderingHelper.SaveReferenceScreenShot("C:/development_ITK4/output.png");
  }

  MITK_TEST_END();
}