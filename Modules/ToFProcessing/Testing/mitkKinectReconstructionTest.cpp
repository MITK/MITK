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

#include <mitkTestingMacros.h>
#include <mitkToFDistanceImageToSurfaceFilter.h>

#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkToFProcessingCommon.h>
#include <mitkVector.h>
#include <mitkToFTestingCommon.h>
#include <mitkIOUtil.h>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

/**
 * @brief mitkKinectReconstructionTest Testing method for the Kinect reconstruction mode. Specially meant for Kinect.
 * This tests loads a special data set from MITK-Data and compares it to a previously generated surface.
 * This test has no dependency to the mitkKinectModule, although it is thematically connected to it.
 */
int mitkKinectReconstructionTest(int  argc , char* argv[])
{
    MITK_TEST_BEGIN("mitkKinectReconstructionTest");

    MITK_TEST_CONDITION_REQUIRED(argc > 3, "Testing if enough arguments are set.");
    std::string calibrationFilePath(argv[1]);
    std::string kinectImagePath(argv[2]);
    std::string groundTruthSurfacePath(argv[3]);

    mitk::ToFDistanceImageToSurfaceFilter::Pointer distToSurf = mitk::ToFDistanceImageToSurfaceFilter::New();
    mitk::CameraIntrinsics::Pointer intrinsics = mitk::CameraIntrinsics::New();
    //load our personal kinect calibration
    intrinsics->FromXMLFile(calibrationFilePath);
    MITK_TEST_CONDITION_REQUIRED(intrinsics.IsNotNull(), "Testing if a calibration file could be loaded.");
    distToSurf->SetCameraIntrinsics(intrinsics);
    distToSurf->SetReconstructionMode(mitk::ToFDistanceImageToSurfaceFilter::Kinect);
    //load a data set
    mitk::Image::Pointer kinectImage = mitk::IOUtil::LoadImage(kinectImagePath);
    MITK_TEST_CONDITION_REQUIRED(kinectImage.IsNotNull(), "Testing if a kinect image could be loaded.");
    distToSurf->SetInput(kinectImage);
    distToSurf->Update();

    //load ground truth data
    mitk::Surface::Pointer groundTruth = mitk::IOUtil::LoadSurface(groundTruthSurfacePath);
    MITK_TEST_CONDITION_REQUIRED(groundTruth.IsNotNull(), "Testing if ground truth could be loaded.");

    mitk::Surface::Pointer resultOfFilter = distToSurf->GetOutput();
    MITK_TEST_CONDITION_REQUIRED(resultOfFilter.IsNotNull(), "Testing if any output was generated.");

    MITK_TEST_CONDITION_REQUIRED( mitk::ToFTestingCommon::VtkPolyDatasEqual(resultOfFilter->GetVtkPolyData(),
                                                                            groundTruth->GetVtkPolyData() ),
                                  "Testing if point sets are equal (with a small epsilon).");
    MITK_TEST_END();
}
