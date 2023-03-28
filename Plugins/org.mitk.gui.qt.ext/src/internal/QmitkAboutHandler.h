/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QmitkAboutHandler_h
#define QmitkAboutHandler_h

#include <berryAbstractHandler.h>

class QmitkAboutHandler : public berry::AbstractHandler
{
  Q_OBJECT

public:

  Object::Pointer Execute(const berry::SmartPointer<const berry::ExecutionEvent>& event) override;
};

#endif
