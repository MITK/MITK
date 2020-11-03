/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef BERRYPARTSERVICE_H_
#define BERRYPARTSERVICE_H_

#include "berryIPartService.h"

#include <string>

namespace berry
{

class PartService : public IPartService
{

private:

  //PartListenerList listeners = new PartListenerList();
  IPartListener::Events partEvents;

  IWorkbenchPartReference::WeakPtr activePart;

  QString debugListenersKey;
  QString debugListeners2Key;

  /**
   * @param ref
   */
  void FirePartActivated(IWorkbenchPartReference::Pointer ref);

  /**
   * @param ref
   */
  void FirePartDeactivated(IWorkbenchPartReference::Pointer ref);

public:

  PartService(const QString& debugListenersKey,
      const QString& debugListeners2Key);

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

  /**
   * @param ref
   */
  void FirePartBroughtToTop(IWorkbenchPartReference::Pointer ref);

  /**
   * @param ref
   */
  void FirePartClosed(IWorkbenchPartReference::Pointer ref);

  void FirePartVisible(IWorkbenchPartReference::Pointer ref);

  void FirePartHidden(IWorkbenchPartReference::Pointer ref);

  void FirePartInputChanged(IWorkbenchPartReference::Pointer ref);

  /**
   * @param ref
   */
  void FirePartOpened(IWorkbenchPartReference::Pointer ref);

  IWorkbenchPart::Pointer GetActivePart() override;

  IWorkbenchPartReference::Pointer GetActivePartReference() override;

  void SetActivePart(IWorkbenchPartReference::Pointer ref);

};

}

#endif /*BERRYPARTSERVICE_H_*/
