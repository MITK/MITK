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


#ifndef BERRYICOMMANDLINECALLBACK_H_
#define BERRYICOMMANDLINECALLBACK_H_

#include <osgi/framework/Object.h>
#include <osgi/framework/Macros.h>

namespace berry {

struct ICommandLineCallback : public virtual Object
{
  osgiInterfaceMacro(berry::ICommandLineCallback)

  virtual void Option(const std::string& name, const std::string& value) = 0;
};

}


#endif /* BERRYICOMMANDLINECALLBACK_H_ */
