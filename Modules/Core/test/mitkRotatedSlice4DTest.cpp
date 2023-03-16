/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

  auto numTimeSteps = std::min(2, static_cast<int>(image4D->GetTimeSteps()));

  for (int ts = 0; ts < numTimeSteps; ++ts)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(image4D);
    timeSelector->SetTimeNr(ts);
    timeSelector->Update();
    mitk::Image::Pointer image3D = timeSelector->GetOutput();

    int sliceNumber = std::min(5, static_cast<int>(image3D->GetSlicedGeometry()->GetSlices()));

    mitk::PlaneGeometry::Pointer plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(image3D->GetGeometry(), mitk::AnatomicalPlane::Coronal, sliceNumber, true, false);

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
