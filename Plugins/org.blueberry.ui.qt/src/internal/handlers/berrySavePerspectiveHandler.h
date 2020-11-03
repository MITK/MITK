/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYSAVEPERSPECTIVEHANDLER_H
#define BERRYSAVEPERSPECTIVEHANDLER_H

#include <berryAbstractHandler.h>

namespace berry {

struct IWorkbenchPage;
class PerspectiveDescriptor;

class SavePerspectiveHandler : public AbstractHandler
{
  Q_OBJECT

public:

  Object::Pointer Execute(const SmartPointer<const ExecutionEvent>& event) override;

private:

  /**
   * Save a singleton over itself.
   */
  void SaveSingleton(const SmartPointer<IWorkbenchPage>& page);

  /**
   * Save a singleton over the user selection.
   */
  void SaveNonSingleton(const SmartPointer<IWorkbenchPage>& page, const SmartPointer<PerspectiveDescriptor>& oldDesc);

};

}

#endif // BERRYSAVEPERSPECTIVEHANDLER_H
