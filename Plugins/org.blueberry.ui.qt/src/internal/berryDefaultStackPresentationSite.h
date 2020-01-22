/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BERRYDEFAULTSTACKPRESENTATIONSITE_H_
#define BERRYDEFAULTSTACKPRESENTATIONSITE_H_

#include "presentations/berryIStackPresentationSite.h"
#include "presentations/berryStackPresentation.h"

namespace berry
{

class DefaultStackPresentationSite: public IStackPresentationSite
{

private:

  StackPresentation::Pointer presentation;

  int state;

  int activeState;

public:

  berryObjectMacro(DefaultStackPresentationSite);

  DefaultStackPresentationSite();

  void SetPresentation(StackPresentation::Pointer newPresentation);

  StackPresentation::Pointer GetPresentation();

  int GetState() override;

  void SetActive(int activeState);

  int GetActive();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IStackPresentationSite#selectPart(org.blueberry.ui.internal.skins.IPresentablePart)
   */
  void SelectPart(IPresentablePart::Pointer toSelect) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentationSite#setState(int)
   */
  void SetState(int newState) override;

  void SetPresentationState(int newState);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentablePart#isClosable()
   */
  bool IsCloseable(IPresentablePart::Pointer part) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentationSite#dragStart(org.blueberry.ui.internal.skins.IPresentablePart, boolean)
   */
  void DragStart(IPresentablePart::Pointer beingDragged,
      QPoint& initialPosition, bool keyboard) override;

  using IStackPresentationSite::Close;

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentationSite#close(org.blueberry.ui.internal.skins.IPresentablePart)
   */
  void Close(IPresentablePart::Pointer toClose);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentationSite#dragStart(boolean)
   */
  void DragStart(QPoint& initialPosition, bool keyboard) override;

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IStackPresentationSite#supportsState(int)
   */
  bool SupportsState(int state) override;

  //    void AddSystemActions(IMenuManager menuManager) {
  //
  //    }

};

}

#endif /* BERRYDEFAULTSTACKPRESENTATIONSITE_H_ */
