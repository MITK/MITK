/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "berryUiApiTestSuite.h"

#include "berryXMLMementoTest.h"
#include "berryIPageLayoutTest.h"
#include "berryIViewPartTest.h"

namespace berry {

UiApiTestSuite::UiApiTestSuite()
: CppUnit::TestSuite("UiApiTestSuite")
{
  addTest(IPageLayoutTest::Suite());
  addTest(IViewPartTest::Suite());
  addTest(XMLMementoTest::Suite());
}

}
