/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYQUITHANDLER_H
#define BERRYQUITHANDLER_H

#include <berryAbstractHandler.h>

namespace berry {

/**
 * Quit (close the workbench).
 */
class QuitHandler : public AbstractHandler
{
  Q_OBJECT

public:

  Object::Pointer Execute(const SmartPointer<const ExecutionEvent>& event) override;

};

}

#endif // BERRYQUITHANDLER_H
