/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYOBJECTSTRING_H_
#define BERRYOBJECTSTRING_H_

#include "berryMacros.h"
#include "berryObject.h"

#include <QString>

#include <org_blueberry_core_runtime_Export.h>

namespace berry {

class org_blueberry_core_runtime_EXPORT ObjectString : public QString, public Object
{

public:

  berryObjectMacro(berry::ObjectString);

  ObjectString();
  ObjectString(const QString& s);

  ~ObjectString() override;

  bool operator==(const Object* other) const override;

  bool operator==(const QString& other) const;

  ObjectString& operator=(const QString& other);

  QString ToString() const override;

};

}

#endif /*BERRYOBJECTSTRING_H_*/
