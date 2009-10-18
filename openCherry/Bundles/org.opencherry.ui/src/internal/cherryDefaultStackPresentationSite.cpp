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

#include "cherryDefaultStackPresentationSite.h"

namespace cherry {

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
    * @see org.opencherry.ui.internal.skins.IStackPresentationSite#selectPart(org.opencherry.ui.internal.skins.IPresentablePart)
    */
   void DefaultStackPresentationSite::SelectPart(IPresentablePart::Pointer toSelect) {

       if (presentation != 0) {
           presentation->SelectPart(toSelect);
       }
   }


   /* (non-Javadoc)
    * @see org.opencherry.ui.internal.skins.IPresentationSite#setState(int)
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
    * @see org.opencherry.ui.internal.skins.IPresentablePart#isClosable()
    */
   bool DefaultStackPresentationSite::IsCloseable(IPresentablePart::Pointer part) {
       return part->IsCloseable();
   }

   /* (non-Javadoc)
    * @see org.opencherry.ui.internal.skins.IPresentationSite#dragStart(org.opencherry.ui.internal.skins.IPresentablePart, boolean)
    */
   void DefaultStackPresentationSite::DragStart(IPresentablePart::Pointer beingDragged, Point& initialPosition,
           bool  /*keyboard*/) {
   }

   /* (non-Javadoc)
    * @see org.opencherry.ui.internal.skins.IPresentationSite#close(org.opencherry.ui.internal.skins.IPresentablePart)
    */
   void DefaultStackPresentationSite::Close(IPresentablePart::Pointer  /*toClose*/) {
   }

   /* (non-Javadoc)
    * @see org.opencherry.ui.internal.skins.IPresentationSite#dragStart(boolean)
    */
   void DefaultStackPresentationSite::DragStart(Point&  /*initialPosition*/, bool  /*keyboard*/) {
   }

   /* (non-Javadoc)
    * @see org.opencherry.ui.presentations.IStackPresentationSite#supportsState(int)
    */
   bool DefaultStackPresentationSite::SupportsState(int  /*state*/) {
       return true;
   }

}
