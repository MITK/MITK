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


#ifndef BERRYITESTDESCRIPTOR_H_
#define BERRYITESTDESCRIPTOR_H_

#include <berryObject.h>
#include <berryMacros.h>

#include <CppUnit/Test.h>

namespace berry {

struct ITestDescriptor : public Object
{
  berryInterfaceMacro(ITestDescriptor, berry)

  virtual CppUnit::Test* CreateTest() = 0;
  virtual std::string GetId() const = 0;
  virtual std::string GetContributor() const = 0;
  virtual std::string GetDescription() const = 0;

  virtual bool IsUITest() const = 0;
};

}

#endif /* BERRYITESTDESCRIPTOR_H_ */
