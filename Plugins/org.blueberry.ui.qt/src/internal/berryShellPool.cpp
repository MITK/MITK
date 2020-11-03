/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "tweaklets/berryGuiWidgetsTweaklet.h"

#include "berryShellPool.h"

namespace berry
{

const QString ShellPool::CLOSE_LISTENER = "close listener"; //$NON-NLS-1$

void ShellPool::ShellClosed(const ShellEvent::Pointer& e)
{

  if (e->doit)
  {
    Shell::Pointer s = e->GetSource();
    IShellListener* l =  s->GetExtraShellListener();

    if (l != nullptr)
    {
      s->SetExtraShellListener(nullptr);
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
        s->SetExtraShellListener(l);
      }
    }
  }
  e->doit = false;
}

ShellPool::ShellPool(Shell::Pointer parentShell, int childFlags) :
  flags(childFlags), parentShell(parentShell)
{

}

Shell::Pointer ShellPool::AllocateShell(IShellListener* closeListener)
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
    result->AddShellListener(this);
    //result.addDisposeListener(disposeListener);
  }

  result->SetExtraShellListener(closeListener);
  return result;
}

}
