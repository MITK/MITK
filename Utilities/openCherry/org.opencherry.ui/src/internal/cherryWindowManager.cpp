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

#include "../cherryWorkbenchWindow.h"

namespace cherry
{

WindowManager::WindowManager()
{
}

void WindowManager::Add(IWorkbenchWindow::Pointer window)
{
  if (std::find(windows.begin(), windows.end(), window) == windows.end())
  {
    windows.push_back(window);
    window.Cast<WorkbenchWindow>()->SetWindowManager(this);
  }
}

bool WindowManager::Close()
{
  std::vector<IWorkbenchWindow::Pointer> t = windows; // make iteration robust
  for (std::vector<IWorkbenchWindow::Pointer>::iterator iter = t.begin();
       iter != t.end(); ++iter)
  {
    bool closed = (*iter)->Close();
    if (!closed)
    {
      return false;
    }
  }
//  if (subManagers != null)
//  {
//    e = subManagers.iterator();
//    while (e.hasNext())
//    {
//      WindowManager wm = (WindowManager) e.next();
//      boolean closed = wm.close();
//      if (!closed)
//      {
//        return false;
//      }
//    }
//  }
  return true;
}

int WindowManager::GetWindowCount()
{
  return windows.size();
}

std::vector<SmartPointer<IWorkbenchWindow> > WindowManager::GetWindows()
{
  return windows;
}

void WindowManager::Remove(SmartPointer<IWorkbenchWindow> window)
{
  std::vector<IWorkbenchWindow::Pointer>::iterator iter = std::find(windows.begin(), windows.end(), window);
  if (iter != windows.end())
  {
    windows.erase(iter);
    window.Cast<WorkbenchWindow>()->SetWindowManager(0);
  }
}

}
