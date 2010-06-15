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

#include "mitkNavigationToolStorageSerializer.h"
#include "mitkCommon.h"
#include "mitkTestingMacros.h"

class NavigationToolStorageSerializerAndDeserializerTestClass
  {
  public:

    static void TestInstantiation()
    {
    // let's create an object of our class
    }
  };

/** This function is testing the TrackingVolume class. */
int mitkNavigationToolStorageSerializerAndDeserializerTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationToolStorageSerializerAndDeserializer")

 NavigationToolStorageSerializerAndDeserializerTestClass::TestInstantiation();

  MITK_TEST_END()
}

