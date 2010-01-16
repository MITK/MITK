/*=========================================================================

 Program:   BlueBerry Platform
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

#ifndef BERRYDEFAULTSTACKPRESENTATIONSITE_H_
#define BERRYDEFAULTSTACKPRESENTATIONSITE_H_

#include "../presentations/berryIStackPresentationSite.h"
#include "../presentations/berryStackPresentation.h"

namespace berry
{

class DefaultStackPresentationSite: public IStackPresentationSite
{

private:

  StackPresentation::Pointer presentation;

  int state;

  int activeState;

public:

  osgiObjectMacro(DefaultStackPresentationSite);

  DefaultStackPresentationSite();

  void SetPresentation(StackPresentation::Pointer newPresentation);

  StackPresentation::Pointer GetPresentation();

  int GetState();

  void SetActive(int activeState);

  int GetActive();

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IStackPresentationSite#selectPart(org.blueberry.ui.internal.skins.IPresentablePart)
   */
  void SelectPart(IPresentablePart::Pointer toSelect);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentationSite#setState(int)
   */
  void SetState(int newState);

  void SetPresentationState(int newState);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentablePart#isClosable()
   */
  bool IsCloseable(IPresentablePart::Pointer part);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentationSite#dragStart(org.blueberry.ui.internal.skins.IPresentablePart, boolean)
   */
  void DragStart(IPresentablePart::Pointer beingDragged,
      Point& initialPosition, bool keyboard);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentationSite#close(org.blueberry.ui.internal.skins.IPresentablePart)
   */
  void Close(IPresentablePart::Pointer toClose);

  /* (non-Javadoc)
   * @see org.blueberry.ui.internal.skins.IPresentationSite#dragStart(boolean)
   */
  void DragStart(Point& initialPosition, bool keyboard);

  /* (non-Javadoc)
   * @see org.blueberry.ui.presentations.IStackPresentationSite#supportsState(int)
   */
  bool SupportsState(int state);

  //    void AddSystemActions(IMenuManager menuManager) {
  //
  //    }

};

}

#endif /* BERRYDEFAULTSTACKPRESENTATIONSITE_H_ */
