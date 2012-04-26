/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkUSImage2D.h"
#include "mitkTestingMacros.h"


class mitkUSImage2DTestClass
{
public:

  // Anm: Implementierung der einzelben Testmethoden

  static void TestInstantiation()
  {
    // let's create an object of our class
    //mitk::ClaronTrackingDevice::Pointer testInstance;
    //testInstance = mitk::ClaronTrackingDevice::New();
    MITK_TEST_CONDITION_REQUIRED(true, "Rigid Test")
      mitk::USImage2D::Pointer us2d = mitk::USImage2D::New();
    MITK_TEST_CONDITION_REQUIRED(us2d.IsNotNull(), "UsImage2D not null after instantiation");
  }

 

};

/**
* This function is testing methods of the class ClaronTrackingDevice which are independent from the hardware. For more tests we would need 
* the MicronTracker hardware, so only a few simple tests, which can run without the hardware are tested here. More tests can be found in the 
* class ClaronTrackingDeviceHardwareTests which tests the interaction with the hardware. 
*/
int mitkUSImage2DTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkUSImage2DTest");

    mitkUSImage2DTestClass::TestInstantiation();

  MITK_TEST_END();
}