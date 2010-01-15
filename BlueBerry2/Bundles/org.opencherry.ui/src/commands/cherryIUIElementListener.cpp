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

#include "cherryIUIElementListener.h"
#include "cherryUIElement.h"

namespace cherry {

void
IUIElementListener::Events
::AddListener(IUIElementListener::Pointer l)
{
  if (l.IsNull()) return;

  elementDisposed += DisposedDelegate(l.GetPointer(), &IUIElementListener::UIElementDisposed);
  elementSelected += SelectedDelegate(l.GetPointer(), &IUIElementListener::UIElementSelected);
}

void
IUIElementListener::Events
::RemoveListener(IUIElementListener::Pointer l)
{
  if (l.IsNull()) return;

  elementDisposed -= DisposedDelegate(l.GetPointer(), &IUIElementListener::UIElementDisposed);
  elementSelected -= SelectedDelegate(l.GetPointer(), &IUIElementListener::UIElementSelected);
}

}

