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

#ifndef BERRYOSGISTRING_H_
#define BERRYOSGISTRING_H_

#include "berryMacros.h"

#include <QString>

namespace berry {

class ObjectString : public QString, public Object
{

public:

  berryObjectMacro(berry::ObjectString)

  ObjectString() {}
  ObjectString(const QString& s) : QString(s) {}

  ~ObjectString() {}
};

}

#endif /*BERRYOSGISTRING_H_*/
