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
#include <mitkImagePixelReadAccessor.h>
#include <mitkSurface.h>
#include <mitkToFProcessingCommon.h>
#include <mitkNumericTypes.h>
#include <mitkToFTestingCommon.h>
#include <mitkIOUtil.h>

#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

/**
 * @brief mitkKinectReconstructionTest Testing method for the Kinect reconstruction mode. Specially meant for Kinect.
 * This tests loads a special data set from MITK-Data and compares it to a reference surface.
 * This test has no dependency to the mitkKinectModule, although it is thematically connected to it.
 */
int mitkKinectReconstructionTest(int  argc , char* argv[])
{
    MITK_TEST_BEGIN("mitkKinectReconstructionTest");

    MITK_TEST_CONDITION_REQUIRED(argc > 2, "Testing if enough arguments are set.");
    std::string calibrationFilePath(argv[1]);
    std::string kinectImagePath(argv[2]);

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

    mitk::Surface::Pointer resultOfFilter = distToSurf->GetOutput();
    MITK_TEST_CONDITION_REQUIRED(resultOfFilter.IsNotNull(), "Testing if any output was generated.");
    mitk::PointSet::Pointer resultPointSet = mitk::ToFTestingCommon::VtkPolyDataToMitkPointSet(resultOfFilter->GetVtkPolyData());

    // generate ground truth data
    mitk::PointSet::Pointer groundTruthPointSet = mitk::PointSet::New();
    mitk::ToFProcessingCommon::ToFPoint2D focalLength;
    focalLength[0] = intrinsics->GetFocalLengthX();
    focalLength[1] = intrinsics->GetFocalLengthY();
    mitk::ToFProcessingCommon::ToFPoint2D principalPoint;
    principalPoint[0] = intrinsics->GetPrincipalPointX();
    principalPoint[1] = intrinsics->GetPrincipalPointY();

    int xDimension = (int)kinectImage->GetDimension(0);
    int yDimension = (int)kinectImage->GetDimension(1);
    int pointCount = 0;
    mitk::ImagePixelReadAccessor<float,2> imageAcces(kinectImage, kinectImage->GetSliceData(0));
    for (int j=0; j<yDimension; j++)
    {
      for (int i=0; i<xDimension; i++)
      {
        itk::Index<2> pixel;
        pixel[0] = i;
        pixel[1] = j;

        mitk::ToFProcessingCommon::ToFScalarType distance = (double)imageAcces.GetPixelByIndex(pixel);

        mitk::Point3D currentPoint;
        currentPoint = mitk::ToFProcessingCommon::KinectIndexToCartesianCoordinates(i,j,distance,focalLength[0],focalLength[1],principalPoint[0],principalPoint[1]);

        if (distance>mitk::eps)
        {
          groundTruthPointSet->InsertPoint( pointCount, currentPoint );
          pointCount++;
        }
      }
    }
    MITK_TEST_CONDITION_REQUIRED( mitk::ToFTestingCommon::PointSetsEqual(resultPointSet,groundTruthPointSet),
                                  "Testing if point sets are equal (with a small epsilon).");
    MITK_TEST_END();
}
