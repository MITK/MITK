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


#ifndef CHERRYITESTHARNESS_H_
#define CHERRYITESTHARNESS_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "../cherryUiDll.h"

namespace cherry {

/**
 * Represents an arbitrary test harness.
 *
 * @since 3.0
 */
struct CHERRY_UI ITestHarness : public virtual Object {

  cherryInterfaceMacro(ITestHarness, cherry)

    /**
     * Runs the tests.
     */
  virtual void RunTests() = 0;

};

}

#endif /* CHERRYITESTHARNESS_H_ */
