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

#include "mitkUSImageVideoSource.h"
#include "mitkTestingMacros.h"


class mitkUSImageVideoSourceTestClass
{
public:

  // Anm: Implementierung der einzelnen Testmethoden

  static void TestInstantiation()
  {
    // let's create an object of our class
    mitk::USImageVideoSource::Pointer usSource = mitk::USImageVideoSource::New();
    MITK_TEST_CONDITION_REQUIRED(usSource.IsNotNull(), "USImageVideoSource should not be null after instantiation");
  }

    static void TestOpenVideoFile()
  {
    mitk::USImageVideoSource::Pointer usSource = mitk::USImageVideoSource::New();
    // "C:\\Users\\maerz\\Videos\\Debut\\us.avi"
    usSource->OpenVideoFile("C:\\Users\\maerz\\Videos\\Debut\\us.avi");
    MITK_TEST_CONDITION_REQUIRED(usSource->GetIsVideoReady(), "USImageVideoSource should have isVideoReady flag set after opening a Video File");
  }

 

};

/**
* This function is testing methods of the class USImage2D.
*/
int mitkUSImageVideoSourceTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkUSImageVideoSourceTest");

    mitkUSImageVideoSourceTestClass::TestInstantiation();
    mitkUSImageVideoSourceTestClass::TestOpenVideoFile();

  MITK_TEST_END();
}