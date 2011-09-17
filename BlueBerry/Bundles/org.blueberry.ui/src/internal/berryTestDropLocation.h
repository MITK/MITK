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


#ifndef BERRYTESTDROPLOCATION_H_
#define BERRYTESTDROPLOCATION_H_

#include <berryObject.h>
#include "../berryPoint.h"
#include "../berryShell.h"

#include <vector>

namespace berry {

/**
 * This is an interface intended for use in test suites. Objects can implement
 * this interface to force any dragged object to be dropped at a particular
 * location.
 *
 * @since 3.0
 */
struct TestDropLocation : public Object {

  berryObjectMacro(TestDropLocation);

    /**
     * Location where the object should be dropped, in display coordinates
     *
     * @return a location in display coordinates
     */
    virtual Point GetLocation() = 0;

    /**
     * The drop code will pretend that only the given shells are open,
     * and that they have the specified Z-order.
     *
     * @return the shells to check for drop targets, from bottom to top.
     */
    virtual std::vector<Shell::Pointer> GetShells() = 0;
};

}

#endif /* BERRYTESTDROPLOCATION_H_ */
