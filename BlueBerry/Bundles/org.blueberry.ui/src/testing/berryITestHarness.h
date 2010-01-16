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


#ifndef BERRYITESTHARNESS_H_
#define BERRYITESTHARNESS_H_

#include <berryObject.h>
#include <berryMacros.h>

#include "../berryUiDll.h"

namespace berry {

/**
 * Represents an arbitrary test harness.
 *
 * @since 3.0
 */
struct BERRY_UI ITestHarness : public virtual Object {

  berryInterfaceMacro(ITestHarness, berry)

    /**
     * Runs the tests.
     */
  virtual void RunTests() = 0;

};

}

#endif /* BERRYITESTHARNESS_H_ */
