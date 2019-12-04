/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKABOUTHANDLER_H
#define QMITKABOUTHANDLER_H

#include <berryAbstractHandler.h>

class QmitkAboutHandler : public berry::AbstractHandler
{
  Q_OBJECT

public:

  Object::Pointer Execute(const berry::SmartPointer<const berry::ExecutionEvent>& event) override;
};

#endif // QMITKABOUTHANDLER_H
