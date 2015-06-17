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

  Object::Pointer Execute(const SmartPointer<const ExecutionEvent>& event);

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
