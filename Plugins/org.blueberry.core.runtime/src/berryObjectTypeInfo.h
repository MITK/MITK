/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYOBJECTTYPEINFO_H
#define BERRYOBJECTTYPEINFO_H

#include <berryObject.h>

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

class org_blueberry_core_runtime_EXPORT ObjectTypeInfo : public Reflection::TypeInfo, public Object
{
public:

  berryObjectMacro(ObjectTypeInfo, Reflection::TypeInfo, Object);

  ObjectTypeInfo();
  ObjectTypeInfo(const Reflection::TypeInfo& typeInfo);

  bool operator==(const Object* other) const override;
  bool operator==(const Reflection::TypeInfo& other) const;

};

}

#endif // BERRYOBJECTTYPEINFO_H
