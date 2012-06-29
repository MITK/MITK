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

  IPartListener::Pointer partListener; // = new WWinListener();

public:

  /**
   * Creates a new part service for a workbench window.
   */
  WWinPartService(IWorkbenchWindow* window);

  /*
   * (non-Javadoc)
   * Method declared on IPartService
   */
  void AddPartListener(IPartListener::Pointer l);

  /*
   * (non-Javadoc)
   * Method declared on IPartService
   */
  void RemovePartListener(IPartListener::Pointer l);

  /*
   * (non-Javadoc)
   * Method declared on IPartService
   */
  IWorkbenchPart::Pointer GetActivePart();

  /*
   * (non-Javadoc)
   * Method declared on IPartService
   */
  IWorkbenchPartReference::Pointer GetActivePartReference();

  /*
   * Returns the selection service.
   */
  ISelectionService* GetSelectionService();

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
