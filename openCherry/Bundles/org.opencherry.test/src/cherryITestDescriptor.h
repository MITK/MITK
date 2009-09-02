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


#ifndef CHERRYITESTDESCRIPTOR_H_
#define CHERRYITESTDESCRIPTOR_H_

#include <cherryObject.h>
#include <cherryMacros.h>

#include "cherryITest.h"

namespace cherry {

struct ITestDescriptor : public Object
{
  cherryInterfaceMacro(ITestDescriptor, cherry)

  virtual ITest::Pointer CreateTest() = 0;
  virtual std::string GetId() const = 0;
  virtual std::string GetContributor() const = 0;
  virtual std::string GetDescription() const = 0;
};

}

#endif /* CHERRYITESTDESCRIPTOR_H_ */
