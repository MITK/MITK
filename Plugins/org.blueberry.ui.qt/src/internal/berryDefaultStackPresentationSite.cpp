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

#include "berryDefaultStackPresentationSite.h"

namespace berry {

DefaultStackPresentationSite::DefaultStackPresentationSite()
  : state(IStackPresentationSite::STATE_RESTORED),
    activeState(StackPresentation::AS_INACTIVE)
    {}

   void DefaultStackPresentationSite::SetPresentation(StackPresentation::Pointer newPresentation) {
       presentation = newPresentation;
       if (presentation != 0) {
           presentation->SetState(state);
           presentation->SetActive(activeState);
       }
   }

   StackPresentation::Pointer DefaultStackPresentationSite::GetPresentation() {
       return presentation;
   }

   int DefaultStackPresentationSite::GetState() {
       return state;
   }

   void DefaultStackPresentationSite::SetActive(int activeState) {
       if (activeState != this->activeState) {
           this->activeState = activeState;
           if (presentation != 0) {
               presentation->SetActive(activeState);
           }
       }
   }

   int DefaultStackPresentationSite::GetActive() {
       return activeState;
   }

   /* (non-Javadoc)
    * @see org.blueberry.ui.internal.skins.IStackPresentationSite#selectPart(org.blueberry.ui.internal.skins.IPresentablePart)
    */
   void DefaultStackPresentationSite::SelectPart(IPresentablePart::Pointer toSelect) {

       if (presentation != 0) {
           presentation->SelectPart(toSelect);
       }
   }


   /* (non-Javadoc)
    * @see org.blueberry.ui.internal.skins.IPresentationSite#setState(int)
    */
   void DefaultStackPresentationSite::SetState(int newState) {
       this->SetPresentationState(newState);
   }

    void DefaultStackPresentationSite::SetPresentationState(int newState) {
       state = newState;
       if (presentation != 0) {
           presentation->SetState(newState);
       }
   }

   /* (non-Javadoc)
    * @see org.blueberry.ui.internal.skins.IPresentablePart#isClosable()
    */
   bool DefaultStackPresentationSite::IsCloseable(IPresentablePart::Pointer part) {
       return part->IsCloseable();
   }

   /* (non-Javadoc)
    * @see org.blueberry.ui.internal.skins.IPresentationSite#dragStart(org.blueberry.ui.internal.skins.IPresentablePart, boolean)
    */
   void DefaultStackPresentationSite::DragStart(IPresentablePart::Pointer /*beingDragged*/, QPoint& /*initialPosition*/,
           bool  /*keyboard*/) {
   }

   /* (non-Javadoc)
    * @see org.blueberry.ui.internal.skins.IPresentationSite#close(org.blueberry.ui.internal.skins.IPresentablePart)
    */
   void DefaultStackPresentationSite::Close(IPresentablePart::Pointer  /*toClose*/) {
   }

   /* (non-Javadoc)
    * @see org.blueberry.ui.internal.skins.IPresentationSite#dragStart(boolean)
    */
   void DefaultStackPresentationSite::DragStart(QPoint&  /*initialPosition*/, bool  /*keyboard*/) {
   }

   /* (non-Javadoc)
    * @see org.blueberry.ui.presentations.IStackPresentationSite#supportsState(int)
    */
   bool DefaultStackPresentationSite::SupportsState(int  /*state*/) {
       return true;
   }

}
