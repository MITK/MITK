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


#ifndef CHERRYOSGICORETESTSUITE_H_
#define CHERRYOSGICORETESTSUITE_H_

#include <CppUnit/TestSuite.h>

namespace cherry {

class OSGiCoreTestSuite : public CppUnit::TestSuite
{
public:

  OSGiCoreTestSuite();
};

}

#endif /* CHERRYOSGICORETESTSUITE_H_ */
