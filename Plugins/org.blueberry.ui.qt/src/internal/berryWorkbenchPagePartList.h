/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
private:

  PageSelectionService* selectionService;

  PartService partService; // = new PartService(UIListenerLogging.PAGE_PARTLISTENER_EVENTS,
  //UIListenerLogging.PAGE_PARTLISTENER2_EVENTS);


protected:

  void FirePartOpened(IWorkbenchPartReference::Pointer part) override;

  void FirePartClosed(IWorkbenchPartReference::Pointer part) override;

  void FirePartAdded(IWorkbenchPartReference::Pointer part) override;

  void FirePartRemoved(IWorkbenchPartReference::Pointer part) override;

  void FireActiveEditorChanged(IWorkbenchPartReference::Pointer ref) override;

  void FireActivePartChanged(IWorkbenchPartReference::Pointer oldRef,
      IWorkbenchPartReference::Pointer newRef) override;

  void FirePartHidden(IWorkbenchPartReference::Pointer ref) override;

  void FirePartVisible(IWorkbenchPartReference::Pointer ref) override;

  void FirePartInputChanged(IWorkbenchPartReference::Pointer ref) override;

public:

  WorkbenchPagePartList(PageSelectionService* selectionService);

  IPartService* GetPartService();

  void FirePartBroughtToTop(IWorkbenchPartReference::Pointer ref) override;

};

}

#endif /*BERRYWORKBENCHPAGEPARTLIST_H_*/
