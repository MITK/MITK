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


#ifndef CHERRYUIAPITESTSUITE_H_
#define CHERRYUIAPITESTSUITE_H_

#include <cherryITest.h>

namespace cherry {

struct UiApiTestSuite : public ITest
{
  CppUnit::Test* GetTest() const;
};

}

#endif /* CHERRYUIAPITESTSUITE_H_ */
