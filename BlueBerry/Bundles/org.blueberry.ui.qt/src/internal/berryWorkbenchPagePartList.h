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

#ifndef BERRYWORKBENCHPAGEPARTLIST_H_
#define BERRYWORKBENCHPAGEPARTLIST_H_

#include <berrySmartPointer.h>

#include "berryPartList.h"
#include "berryPartService.h"
#include "berryPageSelectionService.h"

namespace berry
{

/**
 * \ingroup org_blueberry_ui_internal
 *
 */
class WorkbenchPagePartList : public PartList
{
public:

  berryObjectMacro(WorkbenchPagePartList);

private:

  PageSelectionService* selectionService;

  PartService partService; // = new PartService(UIListenerLogging.PAGE_PARTLISTENER_EVENTS,
  //UIListenerLogging.PAGE_PARTLISTENER2_EVENTS);


protected:

  void FirePartOpened(IWorkbenchPartReference::Pointer part);

  void FirePartClosed(IWorkbenchPartReference::Pointer part);

  void FirePartAdded(IWorkbenchPartReference::Pointer part);

  void FirePartRemoved(IWorkbenchPartReference::Pointer part);

  void FireActiveEditorChanged(IWorkbenchPartReference::Pointer ref);

  void FireActivePartChanged(IWorkbenchPartReference::Pointer oldRef,
      IWorkbenchPartReference::Pointer newRef);

  void FirePartHidden(IWorkbenchPartReference::Pointer ref);

  void FirePartVisible(IWorkbenchPartReference::Pointer ref);

  void FirePartInputChanged(IWorkbenchPartReference::Pointer ref);

public:

  WorkbenchPagePartList(PageSelectionService* selectionService);

  IPartService* GetPartService();

  void FirePartBroughtToTop(IWorkbenchPartReference::Pointer ref);

};

}

#endif /*BERRYWORKBENCHPAGEPARTLIST_H_*/
