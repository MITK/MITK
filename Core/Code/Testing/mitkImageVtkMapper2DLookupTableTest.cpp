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
#include <mitkRenderingModeProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkLookupTable.h>
#include <mitkIOUtil.h>
#include <mitkNodePredicateDataType.h>

//VTK
#include <vtkLookupTable.h>

int mitkImageVtkMapper2DLookupTableTest(int argc, char* argv[])
{
  // load all arguments into a datastorage, take last argument as reference rendering
  // setup a renderwindow of fixed size X*Y
  // render the datastorage
  // compare rendering to reference image
  MITK_TEST_BEGIN("mitkImageVtkMapper2DLookupTableTest")

  mitk::RenderingTestHelper renderingHelper(640, 480, argc, argv);

  //define an arbitrary lookupTable
  vtkSmartPointer<vtkLookupTable> lookupTable = vtkSmartPointer<vtkLookupTable>::New();
  //use linear interpolation
  lookupTable->SetRampToLinear();
  //use the scalar range of the image to have a nice lookuptable
  lookupTable->SetTableRange(0 , 255);
  lookupTable->Build();

  // Fill in a few known colors, the rest will be generated
  lookupTable->SetTableValue(0,   1.0, 0.0, 0.0, 1.0); //map from red
  lookupTable->SetTableValue(255, 0.0, 0.0, 1.0, 1.0); //to blue

  mitk::LookupTable::Pointer myLookupTable = mitk::LookupTable::New();
  myLookupTable->SetVtkLookupTable( lookupTable );

  //set the rendering mode to use the transfer function
  renderingHelper.SetImageProperty("Image Rendering.Mode", mitk::RenderingModeProperty::New(mitk::RenderingModeProperty::LOOKUPTABLE_COLOR));
  //set the property for the image
  renderingHelper.SetImageProperty("LookupTable", mitk::LookupTableProperty::New(myLookupTable));

  //### Usage of CompareRenderWindowAgainstReference: See docu of mitkRrenderingTestHelper
  MITK_TEST_CONDITION( renderingHelper.CompareRenderWindowAgainstReference(argc, argv, 20.0) == true, "CompareRenderWindowAgainstReference test result positive?" );

  //use this to generate a reference screenshot or save the file:
  if(false)
  {
    renderingHelper.SaveReferenceScreenShot("/media/hdd/thomasHdd/Pictures/tmp/output.png");
  }

  MITK_TEST_END();
}
