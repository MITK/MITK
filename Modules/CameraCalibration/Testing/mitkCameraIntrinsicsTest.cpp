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

#include <mitkCameraIntrinsics.h>
#include <mitkTestingMacros.h>
#include <mitkCvMatCompare.h>
#include <itksys/SystemTools.hxx>

/**Documentation
 *  test for the class "CameraIntrinsics".
 */
int mitkCameraIntrinsicsTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("CameraIntrinsics")

  cv::Mat _CameraMatrix = cv::Mat::zeros(3, 3, cv::DataType<double>::type);
  _CameraMatrix.at<double>(2,2) = 1.0;
  _CameraMatrix.at<double>(0,0) = 1662.54;
  _CameraMatrix.at<double>(1,1) = 1678.23;
  _CameraMatrix.at<double>(0,2) = 1030.23;
  _CameraMatrix.at<double>(1,2) = 632.32;

  cv::Mat _DistorsionCoeffs = cv::Mat::zeros(1, 5, cv::DataType<double>::type);
  _DistorsionCoeffs.at<double>(0,0) = -0.443212;
  _DistorsionCoeffs.at<double>(0,1) = 0.231012;
  _DistorsionCoeffs.at<double>(0,2) = -0.013612;
  _DistorsionCoeffs.at<double>(0,3) = -0.005312;
  _DistorsionCoeffs.at<double>(0,4) = 0;

  mitk::CameraIntrinsics::Pointer intrinsics
      = mitk::CameraIntrinsics::New();

  intrinsics->SetIntrinsics(_CameraMatrix, _DistorsionCoeffs);

  double epsilon = 0.001;
  bool equals = false;
  mitk::CvMatCompare _CvMatCompare( &epsilon, &equals );

  {
    cv::Mat _CameraMatrixResult = intrinsics->GetCameraMatrix();
    _CvMatCompare.SetMatrices( &_CameraMatrix, &_CameraMatrixResult );
    _CvMatCompare.Update();
    MITK_TEST_CONDITION_REQUIRED( equals,
                       "Testing CameraIntrinsics::GetCameraMatrix()");
  }

  {
    cv::Mat _DistorsionCoeffsResult = intrinsics->GetDistorsionCoeffs();
    _CvMatCompare.SetMatrices( &_DistorsionCoeffs, &_DistorsionCoeffsResult );
    _CvMatCompare.Update();
    MITK_TEST_CONDITION_REQUIRED( equals,
                       "Testing CameraIntrinsics::GetDistorsionCoeffs()");
  }


  {
    mitk::CameraIntrinsics::Pointer copy
        = intrinsics->Clone();

    cv::Mat _CameraMatrixResult = copy->GetCameraMatrix();
    cv::Mat _DistorsionCoeffsResult = copy->GetDistorsionCoeffs();

    bool distCoeffsEquals = false;
    _CvMatCompare.SetMatrices( &_DistorsionCoeffs, &_DistorsionCoeffsResult );
    _CvMatCompare.Update();
    distCoeffsEquals = equals;
    _CvMatCompare.SetMatrices( &_CameraMatrix, &_CameraMatrixResult );
    _CvMatCompare.Update();
    MITK_TEST_CONDITION( distCoeffsEquals && equals,
                       "Testing CameraIntrinsics::Clone()");
  }


  {
    std::string filename = "mitkCameraIntrinsicsTest.xml";
    intrinsics->ToXMLFile(filename);
    intrinsics->SetValid(true);

    mitk::CameraIntrinsics::Pointer copy
        = mitk::CameraIntrinsics::New();
    copy->FromXMLFile(filename);

    cv::Mat _CameraMatrixResult = copy->GetCameraMatrix();
    cv::Mat _DistorsionCoeffsResult = copy->GetDistorsionCoeffs();

//    MITK_INFO << "_CameraMatrix:" << std::endl
//        << mitk::ToString(_CameraMatrix);
//    MITK_INFO << "_CameraMatrixResult:" << std::endl
//        << mitk::ToString(_CameraMatrixResult);
//    MITK_INFO << "_DistorsionCoeffs:" << std::endl
//        << mitk::ToString(_DistorsionCoeffs);
//    MITK_INFO << "_DistorsionCoeffsResult:" << std::endl
//        << mitk::ToString(_DistorsionCoeffsResult);

    bool distCoeffsEquals = false;
    _CvMatCompare.SetMatrices( &_DistorsionCoeffs, &_DistorsionCoeffsResult );
    _CvMatCompare.Update();
    distCoeffsEquals = equals;
    _CvMatCompare.SetMatrices( &_CameraMatrix, &_CameraMatrixResult );
    _CvMatCompare.Update();
    MITK_TEST_CONDITION( distCoeffsEquals && equals
                         && copy->IsValid(),
                       "Testing CameraIntrinsics::ToXMLFile() and"
                       " CameraIntrinsics::FromXMLFile()");

    if(itksys::SystemTools::FileExists(filename.c_str() , true))
      itksys::SystemTools::RemoveFile(filename.c_str());
  }


  // TODO

  // always end with this!
  MITK_TEST_END();
}
