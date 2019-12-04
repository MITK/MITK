/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYHELPCONTENTSHANDLER_H
#define BERRYHELPCONTENTSHANDLER_H

#include <berryAbstractHandler.h>

namespace berry {

class HelpContentsHandler : public AbstractHandler
{
  Q_OBJECT

public:

  Object::Pointer Execute(const SmartPointer<const ExecutionEvent>& event) override;

  bool IsEnabled() const override;
};

}

#endif // BERRYHELPCONTENTSHANDLER_H
