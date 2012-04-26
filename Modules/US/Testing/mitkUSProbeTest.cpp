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

#include "mitkUSProbe.h"
#include "mitkTestingMacros.h"


class mitkUSProbeTestClass
{
public:

  // Anm: Implementierung der einzelnen Testmethoden

  static void TestInstantiation()
  {
    // let's create an object of our class
    mitk::USProbe::Pointer probe = mitk::USProbe::New();
    MITK_TEST_CONDITION_REQUIRED(probe.IsNotNull(), "USProbe should not be null after instantiation");
  }

    static void TestIsEqualToProbe()
  {
    mitk::USProbe::Pointer usSource = mitk::USProbe::New();
    mitk::USProbe::Pointer probeA = mitk::USProbe::New();
    mitk::USProbe::Pointer probeB = mitk::USProbe::New();
    mitk::USProbe::Pointer probea = mitk::USProbe::New();
    mitk::USProbe::Pointer identicalProbe = mitk::USProbe::New();
    probeA->SetName("ProbeA");
    probeB->SetName("ProbeB");
    probea->SetName("Probea");
    identicalProbe->SetName("ProbeA");
    MITK_TEST_CONDITION_REQUIRED(! probeA->IsEqualToProbe(probeB), "ProbeA and probeB should not be equal");
    MITK_TEST_CONDITION_REQUIRED(! probeA->IsEqualToProbe(probea), "ProbeA and probea should not be equal");
    MITK_TEST_CONDITION_REQUIRED(probeA->IsEqualToProbe(identicalProbe), "ProbeA and probeA should be equal");
  }

 

};

/**
* This function is testing methods of the class USProbe.
*/
int mitkUSProbeTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkUSProbeTest");

    mitkUSProbeTestClass::TestInstantiation();
    mitkUSProbeTestClass::TestIsEqualToProbe();

  MITK_TEST_END();
}