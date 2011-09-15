/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2010-05-27 16:06:53 +0200 (Do, 27 Mai 2010) $
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <mitkTestingMacros.h>
#include <mitkToFImageWriter.h>

/**Documentation
 *  test for the class "ToFImageWriter".
 */
int mitkToFImageWriterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("ToFImageWriter");

  //testing initialization of object
  mitk::ToFImageWriter::Pointer tofWriter = mitk::ToFImageWriter::New();
  MITK_TEST_CONDITION_REQUIRED(tofWriter.GetPointer(), "Testing initialization of test object!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetExtension()!= "", "Test initialization of member extension!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetDistanceImageFileName()== "", "Test initialization of member distanceImageFileName!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetAmplitudeImageFileName()== "", "Test initialization of member amplitudeImageFileName!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetIntensityImageFileName()== "", "Test initialization of member intnensityImageFileName!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetDistanceImageSelected(), "Test initialization of member distanceImageSelected!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetAmplitudeImageSelected(), "Test initialization of member amplitudeImageSelected!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetIntensityImageSelected(), "Test initialization of member intnensityImageSelected!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetCaptureWidth()== 200, "Test initialization of member captureWidth!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetCaptureHeight()== 200, "Test initialization of member captureHeight!");
  MITK_TEST_CONDITION_REQUIRED(tofWriter->GetToFImageType()== mitk::ToFImageWriter::ToFImageType3D, "Test initialization of member ToFImageType!");

  //set member parameter and test again
  unsigned int dimX = 255;
  unsigned int dimY = 188;
  std::string distanceImageFileName("distImg.pic");
  std::string amplitudeImageFileName("amplImg.pic");
  std::string intensityImageFileName("intImg.pic");
  std::string fileExtension(".test");
  bool distanceImageSelected = false;
  bool amplitudeImageSelected = false;
  bool intensityImageSelected = false;
  
  tofWriter->SetCaptureWidth(dimX);
  tofWriter->SetCaptureHeight(dimY);
  tofWriter->SetDistanceImageFileName(distanceImageFileName);
  tofWriter->SetAmplitudeImageFileName(amplitudeImageFileName);
  tofWriter->SetIntensityImageFileName(intensityImageFileName);
  tofWriter->SetExtension(fileExtension);
  tofWriter->SetDistanceImageSelected(distanceImageSelected);
  tofWriter->SetAmplitudeImageSelected(amplitudeImageSelected);
  tofWriter->SetIntensityImageSelected(intensityImageSelected);
  tofWriter->SetToFImageType(mitk::ToFImageWriter::ToFImageType2DPlusT);

  MITK_TEST_CONDITION_REQUIRED(distanceImageFileName==tofWriter->GetDistanceImageFileName(), "Testing set/get distance image file name");
  MITK_TEST_CONDITION_REQUIRED(amplitudeImageFileName==tofWriter->GetAmplitudeImageFileName(), "Testing set/get amplitude image file name");
  MITK_TEST_CONDITION_REQUIRED(intensityImageFileName==tofWriter->GetIntensityImageFileName(), "Testing set/get intensity image file name");
  MITK_TEST_CONDITION_REQUIRED(dimX==tofWriter->GetCaptureWidth(), "Testing set/get CaptureWidth");
  MITK_TEST_CONDITION_REQUIRED(dimY==tofWriter->GetCaptureHeight(), "Testing set/get CaptureHeight");
  MITK_TEST_CONDITION_REQUIRED(distanceImageSelected==tofWriter->GetDistanceImageSelected(), "Testing set/get distance image selection");
  MITK_TEST_CONDITION_REQUIRED(amplitudeImageSelected==tofWriter->GetAmplitudeImageSelected(), "Testing set/get amplitude image selection");
  MITK_TEST_CONDITION_REQUIRED(intensityImageSelected==tofWriter->GetIntensityImageSelected(), "Testing set/get intensity image selection");
  MITK_TEST_CONDITION_REQUIRED(fileExtension==tofWriter->GetExtension(), "Testing set/get file extension");
  MITK_TEST_CONDITION_REQUIRED(mitk::ToFImageWriter::ToFImageType2DPlusT==tofWriter->GetToFImageType(), "Testing set/get ToFImageType");

  MITK_TEST_END();  
}
