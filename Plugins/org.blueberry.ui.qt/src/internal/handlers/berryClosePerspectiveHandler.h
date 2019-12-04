/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYCLOSEPERSPECTIVEHANDLER_H
#define BERRYCLOSEPERSPECTIVEHANDLER_H

#include <berryAbstractHandler.h>

namespace berry {

class WorkbenchPage;
class Perspective;

class ClosePerspectiveHandler : public AbstractHandler
{
  Q_OBJECT

public:

  Object::Pointer Execute(const SmartPointer<const ExecutionEvent>& event) override;

  /**
   * Closes the specified perspective. Nothing will happen if the given page
   * or perspective are <code>null</null>.
   * @param page
   *    a reference to the page
   * @param persp
   *    the perspective to close
   */
  static void ClosePerspective(const SmartPointer<WorkbenchPage>& page,
                               const SmartPointer<Perspective>& persp);
};

}
#endif // BERRYCLOSEPERSPECTIVEHANDLER_H
