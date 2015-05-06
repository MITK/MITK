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
            const QList<Object::Pointer> &args, Object::Pointer expectedValue);

};

}

#endif // BERRYOPENPERSPECTIVEPROPERTYTESTER_H
