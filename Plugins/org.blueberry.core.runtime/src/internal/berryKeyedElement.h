/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYKEYEDELEMENT_H
#define BERRYKEYEDELEMENT_H

#include <berryObject.h>

namespace berry {

class KeyedElement : public Object
{
public:

  berryObjectMacro(berry::KeyedElement);

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
