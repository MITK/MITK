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

  Object::Pointer Execute(const SmartPointer<const ExecutionEvent>& event);

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
