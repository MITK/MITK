/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYSERVICE_H_
#define BERRYSERVICE_H_

#include <org_blueberry_osgi_Export.h>
#include "berryMacros.h"
#include "berryObject.h"

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
