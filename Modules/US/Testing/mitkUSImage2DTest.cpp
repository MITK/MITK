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

  // Anm: Implementierung der einzelnen Testmethoden

  static void TestInstantiation()
  {
    // let's create an object of our class
    mitk::USImage2D::Pointer us2d = mitk::USImage2D::New();
    MITK_TEST_CONDITION_REQUIRED(us2d.IsNotNull(), "USImage2D should not be null after instantiation");
  }

 

};

/**
* This function is testing methods of the class USImage2D.
*/
int mitkUSImage2DTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkUSImage2DTest");

    mitkUSImage2DTestClass::TestInstantiation();

  MITK_TEST_END();
}