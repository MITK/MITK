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

#include "mitkUSImageVideoSource.h"
#include "mitkTestingMacros.h"


class mitkUSImageVideoSourceTestClass
{
public:

  static void TestInstantiation()
  {
    // let's create an object of our class
    mitk::USImageVideoSource::Pointer usSource = mitk::USImageVideoSource::New();
    MITK_TEST_CONDITION_REQUIRED(usSource.IsNotNull(), "USImageVideoSource should not be null after instantiation");
  }

    static void TestOpenVideoFile(std::string videoFilePath)
  {
    mitk::USImageVideoSource::Pointer usSource = mitk::USImageVideoSource::New();

    usSource->SetVideoFileInput(videoFilePath);
    MITK_TEST_CONDITION_REQUIRED(usSource->GetIsVideoReady(), "USImageVideoSource should have isVideoReady flag set after opening a Video File");
    mitk::USImage::Pointer frame;
    frame = usSource->GetNextImage();
    MITK_TEST_CONDITION_REQUIRED(frame.IsNotNull(), "First frame should not be null.");
    frame = usSource->GetNextImage();
    MITK_TEST_CONDITION_REQUIRED(frame.IsNotNull(), "Second frame should not be null.");
    frame = usSource->GetNextImage();
    MITK_TEST_CONDITION_REQUIRED(frame.IsNotNull(), "Third frame should not be null.");
    frame = usSource->GetNextImage();
    MITK_TEST_CONDITION_REQUIRED(frame.IsNotNull(), "Fourth frame should not be null.");
    frame = usSource->GetNextImage();
    MITK_TEST_CONDITION_REQUIRED(frame.IsNotNull(), "Fifth frame should not be null.");
  }
/** This Test will fail if no device is attached. Since it basically covers the same non-OpenCV Functionality as TestOpenVideoFile, it is ommited
  static void TestOpenDevice()
  {
    mitk::USImageVideoSource::Pointer usSource = mitk::USImageVideoSource::New();
    usSource->SetCameraInput(-1);
    MITK_TEST_CONDITION_REQUIRED(usSource->GetIsVideoReady(), "USImageVideoSource should have isVideoReady flag set after opening a Camera device");
  }
*/



};

/**
* This function is testing methods of the class USImageVideoSource.
*/
int mitkUSImageVideoSourceTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkUSImageVideoSourceTest");

  mitkUSImageVideoSourceTestClass::TestInstantiation();

  #ifdef WIN32 // Video file compression is currently only supported under windows.
  mitkUSImageVideoSourceTestClass::TestOpenVideoFile(argv[1]);
  #endif

  // This test is commented out since no videodevcie ist steadily connected to the dart clients.
  // Functionality should sufficiently be tested through TestOpenVideoFile anyway
  //mitkUSImageVideoSourceTestClass::TestOpenDevice();

  MITK_TEST_END();
}
