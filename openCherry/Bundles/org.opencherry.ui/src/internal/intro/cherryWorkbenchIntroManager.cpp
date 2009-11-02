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

#include "cherryWorkbenchIntroManager.h"

#include "cherryIntroConstants.h"

#include "../cherryWorkbench.h"
#include "../cherryWorkbenchPage.h"
#include "../cherryWorkbenchPlugin.h"
#include "../cherryPerspective.h"

namespace cherry
{

void WorkbenchIntroManager::CreateIntro(
    SmartPointer<IWorkbenchWindow> preferredWindow)
{
  if (!this->workbench->GetIntroDescriptor())
  {
    return;
  }

  IWorkbenchPage::Pointer workbenchPage = preferredWindow->GetActivePage();
  if (!workbenchPage)
  {
    return;
  }
  try
  {
    workbenchPage->ShowView(IntroConstants::INTRO_VIEW_ID);
  } catch (PartInitException& e)
  {
    //TODO IStatus
//    WorkbenchPlugin::Log(IntroMessages.Intro_could_not_create_part, new Status(
//        IStatus.ERROR, WorkbenchPlugin.PI_WORKBENCH, IStatus.ERROR,
//        IntroMessages.Intro_could_not_create_part, e));
    WorkbenchPlugin::Log("Could not create intro part.", e);
  }
}

WorkbenchIntroManager::WorkbenchIntroManager(Workbench* workbench)
: workbench(workbench)
{
  //        workbench.getExtensionTracker().registerHandler(new IExtensionChangeHandler(){
  //
  //            /* (non-Javadoc)
  //             * @see org.eclipse.core.runtime.dynamicHelpers.IExtensionChangeHandler#addExtension(org.eclipse.core.runtime.dynamicHelpers.IExtensionTracker, org.eclipse.core.runtime.IExtension)
  //             */
  //            public void addExtension(IExtensionTracker tracker,IExtension extension) {
  //                //Do nothing
  //            }
  //
  //      /* (non-Javadoc)
  //       * @see org.eclipse.core.runtime.dynamicHelpers.IExtensionChangeHandler#removeExtension(org.eclipse.core.runtime.IExtension, java.lang.Object[])
  //       */
  //      public void removeExtension(IExtension source, Object[] objects) {
  //                for (int i = 0; i < objects.length; i++) {
  //                    if (objects[i] instanceof IIntroPart) {
  //                        closeIntro((IIntroPart) objects[i]);
  //                    }
  //                }
  //
  //      }}, null);

}

bool WorkbenchIntroManager::CloseIntro(IIntroPart::Pointer part)
{
  if (!introPart || introPart != part)
  {
    return false;
  }

  IViewPart::Pointer introView = GetViewIntroAdapterPart();
  if (introView)
  {
    //assumption is that there is only ever one intro per workbench
    //if we ever support one per window then this will need revisiting
    IWorkbenchPage::Pointer page = introView->GetSite()->GetPage();
    IViewReference::Pointer reference = page->FindViewReference(
        IntroConstants::INTRO_VIEW_ID);
    page->HideView(introView);
    if (!reference || reference->GetPart(false) == 0)
    {
      introPart = 0;
      return true;
    }
    return false;
  }

  // if there is no part then null our reference
  introPart = 0;

  return true;
}

IIntroPart::Pointer WorkbenchIntroManager::ShowIntro(SmartPointer<
    IWorkbenchWindow> preferredWindow, bool standby)
{
  if (!preferredWindow)
  {
    preferredWindow = this->workbench->GetActiveWorkbenchWindow();
  }

  if (!preferredWindow)
  {
    return IIntroPart::Pointer(0);
  }

  ViewIntroAdapterPart::Pointer viewPart = GetViewIntroAdapterPart();
  if (!viewPart)
  {
    CreateIntro(preferredWindow);
  }
  else
  {
    try
    {
      IWorkbenchPage::Pointer page = viewPart->GetSite()->GetPage();
      IWorkbenchWindow::Pointer window = page->GetWorkbenchWindow();
      if (window != preferredWindow)
      {
        window->GetShell()->SetActive();
      }

      page->ShowView(IntroConstants::INTRO_VIEW_ID);
    } catch (PartInitException& e)
    {
      //TODO IStatus
//      WorkbenchPlugin::Log("Could not open intro", new Status(IStatus.ERROR,
//          WorkbenchPlugin.PI_WORKBENCH, IStatus.ERROR, "Could not open intro",
//          e)); //$NON-NLS-1$ //$NON-NLS-2$
      WorkbenchPlugin::Log("Could not open intro", e);
    }
  }
  SetIntroStandby(introPart, standby);
  return introPart;
}

bool WorkbenchIntroManager::IsIntroInWindow(
    SmartPointer<IWorkbenchWindow> testWindow) const
{
  ViewIntroAdapterPart::Pointer viewPart = GetViewIntroAdapterPart();
  if (!viewPart)
  {
    return false;
  }

  IWorkbenchWindow::Pointer window = viewPart->GetSite()->GetWorkbenchWindow();
  if (window == testWindow)
  {
    return true;
  }
  return false;
}

void WorkbenchIntroManager::SetIntroStandby(IIntroPart::Pointer part,
    bool standby)
{
  if (!introPart || introPart != part)
  {
    return;
  }

  ViewIntroAdapterPart::Pointer viewIntroAdapterPart = GetViewIntroAdapterPart();
  if (!viewIntroAdapterPart)
  {
    return;
  }

//  PartPane::Pointer pane = viewIntroAdapterPart->GetSite().Cast<PartSite>()->GetPane();
//  if (standby == !pane.isZoomed())
//  {
//    // the zoom state is already correct - just update the part's state.
//    viewIntroAdapterPart.setStandby(standby);
//    return;
//  }
//
//  viewIntroAdapterPart.getSite().getPage().toggleZoom(pane.getPartReference());

}

bool WorkbenchIntroManager::IsIntroStandby(IIntroPart::Pointer part) const
{
  if (!introPart || introPart != part)
  {
    return false;
  }

  ViewIntroAdapterPart::Pointer viewIntroAdapterPart = GetViewIntroAdapterPart();
  if (!viewIntroAdapterPart)
  {
    return false;
  }

  //return !((PartSite) viewIntroAdapterPart.getSite()).getPane().isZoomed();
  return false;
}

IIntroPart::Pointer WorkbenchIntroManager::GetIntro() const
{
  return introPart;
}

ViewIntroAdapterPart::Pointer WorkbenchIntroManager::GetViewIntroAdapterPart() const
{
  std::vector<IWorkbenchWindow::Pointer> windows(this->workbench->GetWorkbenchWindows());
  for (std::size_t i = 0; i < windows.size(); i++)
  {
    IWorkbenchWindow::Pointer window = windows[i];
    WorkbenchPage::Pointer page = window->GetActivePage().Cast<WorkbenchPage>();
    if (!page)
    {
      continue;
    }
    std::vector<IPerspectiveDescriptor::Pointer> perspDescs(page->GetOpenPerspectives());
    for (std::size_t j = 0; j < perspDescs.size(); j++)
    {
      IPerspectiveDescriptor::Pointer descriptor = perspDescs[j];
      IViewReference::Pointer reference = page->FindPerspective(descriptor)->FindView(
          IntroConstants::INTRO_VIEW_ID);
      if (reference)
      {
        IViewPart::Pointer part = reference->GetView(false);
        if (part && part.Cast<ViewIntroAdapterPart>())
        {
          return part.Cast<ViewIntroAdapterPart>();
        }
      }
    }
  }
  return ViewIntroAdapterPart::Pointer(0);
}

IIntroPart::Pointer WorkbenchIntroManager::CreateNewIntroPart()
    throw (CoreException)
{
  IntroDescriptor::Pointer introDescriptor(workbench->GetIntroDescriptor());
  introPart = (introDescriptor == 0 ? IIntroPart::Pointer(0) : introDescriptor->CreateIntro());
//  if (introPart)
//  {
//    workbench.getExtensionTracker().registerObject(
//        introDescriptor.getConfigurationElement() .getDeclaringExtension(),
//        introPart, IExtensionTracker.REF_WEAK);
//  }
  return introPart;
}

bool WorkbenchIntroManager::HasIntro() const
{
  return workbench->GetIntroDescriptor() != 0;
}

bool WorkbenchIntroManager::IsNewContentAvailable()
{
  IntroDescriptor::Pointer introDescriptor = workbench->GetIntroDescriptor();
  if (!introDescriptor)
  {
    return false;
  }
  try
  {
    IntroContentDetector::Pointer contentDetector =
        introDescriptor->GetIntroContentDetector();
    if (contentDetector)
    {
      return contentDetector->IsNewContentAvailable();
    }
  } catch (CoreException& ex)
  {
    //TODO IStatus
//    WorkbenchPlugin.log(new Status(IStatus.WARNING,
//        WorkbenchPlugin.PI_WORKBENCH, IStatus.WARNING,
//        "Could not load intro content detector", ex)); //$NON-NLS-1$
    WorkbenchPlugin::Log("Could not load intro content detector", ex);
  }
  return false;
}

}
