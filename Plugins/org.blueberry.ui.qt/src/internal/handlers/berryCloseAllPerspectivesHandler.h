/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCLOSEALLPERSPECTIVESHANDLER_H
#define BERRYCLOSEALLPERSPECTIVESHANDLER_H

#include <berryAbstractHandler.h>

namespace berry {

/**
 * Closes all the perspectives.
 */
class CloseAllPerspectivesHandler : public AbstractHandler
{
  Q_OBJECT

public:

  Object::Pointer Execute(const SmartPointer<const ExecutionEvent>& event) override;

};

}

#endif // BERRYCLOSEALLPERSPECTIVESHANDLER_H
