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

#ifndef BERRYKEYEDELEMENT_H
#define BERRYKEYEDELEMENT_H

#include <berryObject.h>

namespace berry {

class KeyedElement : public Object
{
public:

  berryObjectMacro(berry::KeyedElement)

  ~KeyedElement() override;

  bool operator==(const KeyedElement& other) const;

  bool operator==(const Object* other) const override;

  virtual QString GetKey() const = 0;

  uint HashCode() const override;

private:

  virtual bool IsEqual(const KeyedElement& other) const = 0;
};

}

#endif // BERRYKEYEDELEMENT_H
