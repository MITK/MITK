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

  berryObjectMacro(berry::ObjectString)

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
