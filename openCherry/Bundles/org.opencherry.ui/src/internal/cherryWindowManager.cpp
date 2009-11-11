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

#include "cherryWindowManager.h"

#include "../cherryWindow.h"

#include <algorithm>

namespace cherry
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
  poco_assert(parent != 0);
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
  std::vector<Window::Pointer> t = windows; // make iteration robust
  for (std::vector<Window::Pointer>::iterator iter = t.begin();
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
    for (std::list<WindowManager*>::iterator iter = subManagers.begin();
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

std::size_t WindowManager::GetWindowCount()
{
  return windows.size();
}

std::vector<Window::Pointer> WindowManager::GetWindows()
{
  return windows;
}

void WindowManager::Remove(Window::Pointer window)
{
  std::vector<Window::Pointer>::iterator iter = std::find(windows.begin(), windows.end(), window);
  if (iter != windows.end())
  {
    windows.erase(iter);
    window->SetWindowManager(0);
  }
}

}
