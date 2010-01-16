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

#include "berryViewPart.h"

#include "berryImageDescriptor.h"

#include <Poco/Exception.h>

namespace berry
{

ViewPart::ViewPart()
{

}

void ViewPart::Init(IViewSite::Pointer site, IMemento::Pointer /*memento*/)
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

void ViewPart::SaveState(IMemento::Pointer /*memento*/)
{
  // do nothing
}

void ViewPart::CheckSite(IWorkbenchPartSite::Pointer site)
{
  WorkbenchPart::CheckSite(site);
  if (site.Cast<IViewSite>().IsNull())
    throw Poco::AssertionViolationException("The site for a view must be an IViewSite"); //$NON-NLS-1$
}

IViewSite::Pointer ViewPart::GetViewSite()
{
  return this->GetSite().Cast<IViewSite>();
}

} // namespace berry

