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


#include "cherryViewPart.h"

namespace cherry {

ViewPart::ViewPart()
{

}
  
void ViewPart::Init(IWorkbenchPartSite::Pointer site, IMemento* /*memento*/) 
{
  /*
   * Initializes this view with the given view site.  A memento is passed to
   * the view which contains a snapshot of the views state from a previous
   * session.  Where possible, the view should try to recreate that state
   * within the part controls.
   * <p>
   * This implementation will ignore the memento and initialize the view in
   * a fresh state.  Subclasses may override the implementation to perform any
   * state restoration as needed.
   */
  this->SetSite(site);
}

void ViewPart::SaveState(IMemento* /*memento*/) 
{
  // do nothing
}

}  // namespace cherry

