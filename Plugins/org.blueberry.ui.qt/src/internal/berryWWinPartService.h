/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYWWINPARTSERVICE_H_
#define BERRYWWINPARTSERVICE_H_

#include "berryIPartService.h"

#include "berryPartService.h"
#include "berryWindowSelectionService.h"

namespace berry
{

/**
 * A part service for a workbench window.
 */
class WWinPartService: public IPartService
{

private:

  friend struct WWinListener;

  PartService partService; // (UIListenerLogging.WINDOW_PARTLISTENER_EVENTS, UIListenerLogging.WINDOW_PARTLISTENER2_EVENTS);

  WindowSelectionService selectionService;

  IWorkbenchPage* activePage;

  QScopedPointer<IPartListener> partListener; // = new WWinListener();

public:

  /**
   * Creates a new part service for a workbench window.
   */
  WWinPartService(IWorkbenchWindow* window);

  /*
   * (non-Javadoc)
   * Method declared on IPartService
   */
  void AddPartListener(IPartListener* l) override;

  /*
   * (non-Javadoc)
   * Method declared on IPartService
   */
  void RemovePartListener(IPartListener* l) override;

  /*
   * (non-Javadoc)
   * Method declared on IPartService
   */
  IWorkbenchPart::Pointer GetActivePart() override;

  /*
   * (non-Javadoc)
   * Method declared on IPartService
   */
  IWorkbenchPartReference::Pointer GetActivePartReference() override;

  /*
   * Returns the selection service.
   */
  ISelectionService* GetSelectionService() const;

  /*
   * Notifies that a page has been activated.
   */
  void PageActivated(SmartPointer<IWorkbenchPage> newPage);

  /*
   * Notifies that a page has been closed
   */
  void PageClosed(SmartPointer<IWorkbenchPage> page);

  /*
   * Notifies that a page has been opened.
   */
  void PageOpened(SmartPointer<IWorkbenchPage> page);

private:

  void UpdateActivePart();

  /*
   * Resets the part service. The active page, part and selection are
   * dereferenced.
   */
  void Reset();

};

}

#endif /* BERRYWWINPARTSERVICE_H_ */
