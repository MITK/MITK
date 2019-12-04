/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYOPENPERSPECTIVEPROPERTYTESTER_H
#define BERRYOPENPERSPECTIVEPROPERTYTESTER_H

#include <berryPropertyTester.h>

namespace berry {

/**
 * Tests if any Perspective is open or not.
 */
class OpenPerspectivePropertyTester : public PropertyTester
{
  Q_OBJECT

private:

  static QString PROPERTY_IS_PERSPECTIVE_OPEN; // = "isPerspectiveOpen";

public:

  bool Test(Object::ConstPointer receiver, const QString& property,
            const QList<Object::Pointer> &args, Object::Pointer expectedValue) override;

};

}

#endif // BERRYOPENPERSPECTIVEPROPERTYTESTER_H
