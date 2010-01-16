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

#ifndef BERRYSERVICE_H_
#define BERRYSERVICE_H_

#include "../berryOSGiDll.h"
#include "../berryMacros.h"
#include "../berryObject.h"

#include <typeinfo>

namespace berry {

class BERRY_OSGI Service : public Object
{

public:

  berryObjectMacro(Service);

  virtual ~Service();

  virtual bool IsA(const std::type_info& type) const;
  virtual const std::type_info& GetType() const;
};

}  // namespace berry

#endif /*BERRYSERVICE_H_*/
