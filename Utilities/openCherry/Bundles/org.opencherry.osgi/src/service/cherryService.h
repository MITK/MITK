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

#ifndef CHERRYSERVICE_H_
#define CHERRYSERVICE_H_

#include "../cherryOSGiDll.h"
#include "../cherryMacros.h"
#include "../cherryObject.h"

#include <typeinfo>

namespace cherry {

class CHERRY_OSGI Service : public Object
{

public:

  cherryClassMacro(Service);

  virtual ~Service();

  virtual bool IsA(const std::type_info& type) const;
  virtual const std::type_info& GetType() const;
};

}  // namespace cherry

#endif /*CHERRYSERVICE_H_*/
