/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#ifndef CHERRYWWINPARTSERVICE_H_
#define CHERRYWWINPARTSERVICE_H_

#include "../cherryIPartService.h"

#include "cherryPartService.h"
#include "cherryWindowSelectionService.h"

namespace cherry
{

/**
 * A part service for a workbench window.
 */
class WWinPartService: public IPartService
{

private:

  friend class WWinListener;

  PartService partService; // (UIListenerLogging.WINDOW_PARTLISTENER_EVENTS, UIListenerLogging.WINDOW_PARTLISTENER2_EVENTS);

  WindowSelectionService selectionService;

  SmartPointer<IWorkbenchPage> activePage;

  IPartListener::Pointer partListener; // = new WWinListener();

public:

  /**
   * Creates a new part service for a workbench window.
   */
  WWinPartService(SmartPointer<IWorkbenchWindow> window);

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

#endif /* CHERRYWWINPARTSERVICE_H_ */
