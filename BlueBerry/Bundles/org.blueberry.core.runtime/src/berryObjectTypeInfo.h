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

#ifndef BERRYOBJECTTYPEINFO_H
#define BERRYOBJECTTYPEINFO_H

#include <berryObject.h>

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

class org_blueberry_core_runtime_EXPORT ObjectTypeInfo : public Reflection::TypeInfo, public Object
{
public:

  berryObjectMacro(ObjectTypeInfo, Reflection::TypeInfo, Object)

  ObjectTypeInfo();
  ObjectTypeInfo(const Reflection::TypeInfo& typeInfo);

  bool operator==(const Object* other) const;
  bool operator==(const Reflection::TypeInfo& other) const;

};

}

#endif // BERRYOBJECTTYPEINFO_H
