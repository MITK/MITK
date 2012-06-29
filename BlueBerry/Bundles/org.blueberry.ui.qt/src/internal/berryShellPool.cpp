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

#include "berryShellPool.h"

#include "tweaklets/berryGuiWidgetsTweaklet.h"

namespace berry
{

const std::string ShellPool::CLOSE_LISTENER = "close listener"; //$NON-NLS-1$

void ShellPool::ShellClosed(ShellEvent::Pointer e)
{

  if (e->doit)
  {
    Shell::Pointer s = e->GetSource();
    IShellListener::Pointer l =
        s->GetData(CLOSE_LISTENER).Cast<IShellListener> ();

    if (l != 0)
    {
      s->SetData(Object::Pointer(0), CLOSE_LISTENER);
      l->ShellClosed(e);

      // The shell can 'cancel' the close by setting
      // the 'doit' to false...if so, do nothing
      if (e->doit)
      {
        availableShells.push_back(s);
        s->SetVisible(false);
      }
      else
      {
        // Restore the listener
        s->SetData(l, CLOSE_LISTENER);
      }
    }
  }
  e->doit = false;
}

ShellPool::ShellPool(Shell::Pointer parentShell, int childFlags) :
  flags(childFlags), parentShell(parentShell)
{

}

Shell::Pointer ShellPool::AllocateShell(IShellListener::Pointer closeListener)
{
  Shell::Pointer result;
  if (!availableShells.empty())
  {
    result = availableShells.front();
    availableShells.pop_front();
  }
  else
  {
    result = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->CreateShell(parentShell.Lock(),
        flags);
    result->AddShellListener(IShellListener::Pointer(this));
    //result.addDisposeListener(disposeListener);
  }

  result->SetData(closeListener, CLOSE_LISTENER);
  return result;
}

}
