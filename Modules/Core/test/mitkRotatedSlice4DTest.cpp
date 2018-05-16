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

#include "mitkExtractSliceFilter.h"
#include "mitkIOUtil.h"
#include "mitkImagePixelReadAccessor.h"
#include "mitkImageTimeSelector.h"
#include "mitkInteractionConst.h"
#include "mitkRotationOperation.h"
#include "mitkTestingMacros.h"
#include <ctime>

/*
* The mitkRotatedSlice4DTest loads a 4D image and extracts a specifically rotated slice in each time step's volume.
*/
int mitkRotatedSlice4DTest(int, char *argv[])
{
  MITK_TEST_BEGIN("mitkRotatedSlice4DTest");

  std::string filename = argv[1];

  // load 4D image
  mitk::Image::Pointer image4D = mitk::IOUtil::Load<mitk::Image>(filename);
  // check inputs
  if (image4D.IsNull())
  {
    MITK_INFO << "Could not load the file";
    return false;
  }

  // for each time step...
  for (unsigned int ts = 0; ts < image4D->GetTimeSteps(); ts++)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image4D);
    timeSelector->SetTimeNr(ts);
    timeSelector->Update();
    mitk::Image::Pointer image3D = timeSelector->GetOutput();

    int sliceNumber = 5;

    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(image3D->GetGeometry(), mitk::PlaneGeometry::Frontal, sliceNumber, true, false);

    // rotate about an arbitrary point and axis...
    float angle = 30;
    mitk::Point3D point;
    point.Fill(sliceNumber);
    mitk::Vector3D rotationAxis;
    rotationAxis[0] = 1;
    rotationAxis[1] = 2;
    rotationAxis[2] = 3;
    rotationAxis.Normalize();

    // Create Rotation Operation
    auto *op = new mitk::RotationOperation(mitk::OpROTATE, point, rotationAxis, angle);
    plane->ExecuteOperation(op);
    delete op;

    // Now extract
    mitk::ExtractSliceFilter::Pointer extractor = mitk::ExtractSliceFilter::New();
    extractor->SetInput(image3D);
    extractor->SetWorldGeometry(plane);
    extractor->Update();
    mitk::Image::Pointer extractedPlane;
    extractedPlane = extractor->GetOutput();

    std::stringstream ss;
    ss << " : Valid slice in timestep " << ts;

    MITK_TEST_CONDITION_REQUIRED(extractedPlane.IsNotNull(), ss.str().c_str());
  }
  MITK_TEST_END();
}
