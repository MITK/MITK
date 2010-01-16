/*=========================================================================
 
 Program:   openCherry Platform
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

#include <Poco/ClassLibrary.h>

#include "src/cherryUiTestSuite.h"

#include "src/api/cherryMockViewPart.h"
#include "src/api/cherrySaveableMockViewPart.h"

POCO_BEGIN_NAMED_MANIFEST(CppUnitTest, CppUnit::Test)
  POCO_EXPORT_CLASS(cherry::UiTestSuite)
POCO_END_MANIFEST

POCO_BEGIN_NAMED_MANIFEST(cherryIViewPart, cherry::IViewPart)
  POCO_EXPORT_CLASS(cherry::MockViewPart)
  POCO_EXPORT_CLASS(cherry::SaveableMockViewPart)
POCO_END_MANIFEST
