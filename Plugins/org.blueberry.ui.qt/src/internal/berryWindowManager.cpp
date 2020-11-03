/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryWindowManager.h"

#include "berryWindow.h"

#include <algorithm>

namespace berry
{

void WindowManager::AddWindowManager(WindowManager* wm)
{
  if (std::find(subManagers.begin(), subManagers.end(), wm)
      == subManagers.end())
  {
    subManagers.push_back(wm);
  }
}

WindowManager::WindowManager()
{
}

WindowManager::WindowManager(WindowManager* parent)
{
  poco_assert(parent != nullptr);
  parent->AddWindowManager(this);
}

void WindowManager::Add(Window::Pointer window)
{
  if (std::find(windows.begin(), windows.end(), window) == windows.end())
  {
    windows.push_back(window);
    window->SetWindowManager(this);
  }
}

bool WindowManager::Close()
{
  QList<Window::Pointer> t = windows; // make iteration robust
  for (QList<Window::Pointer>::iterator iter = t.begin();
      iter != t.end(); ++iter)
  {
    bool closed = (*iter)->Close();
    if (!closed)
    {
      return false;
    }
  }

  if (!subManagers.empty())
  {
    for (QList<WindowManager*>::iterator iter = subManagers.begin();
        iter != subManagers.end(); ++iter)
    {
      WindowManager* wm = *iter;
      bool closed = wm->Close();
      if (!closed)
      {
        return false;
      }
    }
  }
  return true;
}

std::size_t WindowManager::GetWindowCount() const
{
  return windows.size();
}

QList<Window::Pointer> WindowManager::GetWindows() const
{
  return windows;
}

void WindowManager::Remove(Window::Pointer window)
{
  QList<Window::Pointer>::iterator iter = std::find(windows.begin(), windows.end(), window);
  if (iter != windows.end())
  {
    windows.erase(iter);
    window->SetWindowManager(nullptr);
  }
}

}
