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

#ifndef BERRYRESETPERSPECTIVEHANDLER_H
#define BERRYRESETPERSPECTIVEHANDLER_H

#include <berryAbstractHandler.h>

namespace berry {

class ResetPerspectiveHandler : public AbstractHandler
{
  Q_OBJECT

public:

  Object::Pointer Execute(const SmartPointer<const ExecutionEvent>& event);
};

}

#endif // BERRYRESETPERSPECTIVEHANDLER_H
