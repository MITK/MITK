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

#include "cherryIExecutionListenerWithChecks.h"

#include "cherryExecutionEvent.h"
#include "cherryIParameterValueConverter.h"
#include "cherryCommandCategory.h"
#include "cherryIHandler.h"

namespace cherry {

void
IExecutionListenerWithChecks::Events
::AddListener(IExecutionListener::Pointer l)
{
  if (l.IsNull()) return;

  IExecutionListener::Events::AddListener(l);

  if (IExecutionListenerWithChecks::Pointer cl = l.Cast<IExecutionListenerWithChecks>())
  {
    notDefined += NotDefinedDelegate(cl.GetPointer(), &IExecutionListenerWithChecks::NotDefined);
    notEnabled += NotEnabledDelegate(cl.GetPointer(), &IExecutionListenerWithChecks::NotEnabled);
  }
}

void
IExecutionListenerWithChecks::Events
::RemoveListener(IExecutionListener::Pointer l)
{
  if (l.IsNull()) return;

  IExecutionListener::Events::RemoveListener(l);

  if (IExecutionListenerWithChecks::Pointer cl = l.Cast<IExecutionListenerWithChecks>())
  {
    notDefined -= NotDefinedDelegate(cl.GetPointer(), &IExecutionListenerWithChecks::NotDefined);
    notEnabled -= NotEnabledDelegate(cl.GetPointer(), &IExecutionListenerWithChecks::NotEnabled);
  }
}

bool
IExecutionListenerWithChecks::Events
::HasListeners() const
{
  return IExecutionListener::Events::HasListeners() || notDefined.HasListeners() ||
         notEnabled.HasListeners();
}

bool
IExecutionListenerWithChecks::Events
::IsEmpty() const
{
  return !this->HasListeners();
}

}
