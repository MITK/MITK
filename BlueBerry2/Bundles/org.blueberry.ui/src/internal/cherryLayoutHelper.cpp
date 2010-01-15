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

#include "cherryLayoutHelper.h"

#include "cherryPageLayout.h"
#include "cherryWorkbenchPartReference.h"

namespace cherry
{

LayoutHelper::LayoutHelper()
{

}

void LayoutHelper::AddViewActivator(PageLayout::Pointer pageLayout,
    const std::string&  /*viewId*/)
{
  // TODO View Activator
//  if (viewId == null)
//  {
//    return;
//  }
//
//  ViewFactory viewFactory = pageLayout.getViewFactory();
//
//  final IWorkbenchPage partPage = viewFactory.getWorkbenchPage();
//  if (partPage == null)
//  {
//    return;
//  }
//
//  final IPerspectiveDescriptor partPerspective = pageLayout
//  .getDescriptor();
//
//  IWorkbenchActivitySupport support =
//      PlatformUI.getWorkbench() .getActivitySupport();
//
//  IViewDescriptor descriptor = viewFactory.getViewRegistry().find(viewId);
//  if (!(descriptor instanceof IPluginContribution)
//    )
//    {
//      return;
//    }
//
//    IIdentifier identifier = support.getActivityManager().getIdentifier(
//        WorkbenchActivityHelper .createUnifiedId(
//            (IPluginContribution) descriptor));
//
//  identifier.addIdentifierListener(new IIdentifierListener()
//      {
//
//        /* (non-Javadoc)
//         * @see org.opencherry.ui.activities.IIdentifierListener#identifierChanged(org.opencherry.ui.activities.IdentifierEvent)
//         */
//      public void identifierChanged(IdentifierEvent identifierEvent)
//        {
//          if (identifierEvent.hasEnabledChanged())
//          {
//            IIdentifier thisIdentifier = identifierEvent
//            .getIdentifier();
//            if (thisIdentifier.isEnabled())
//            {
//              // show view
//              thisIdentifier.removeIdentifierListener(this);
//              IWorkbenchPage activePage = partPage
//              .getWorkbenchWindow().getActivePage();
//              if (partPage == activePage
//                  && partPerspective == activePage
//                  .getPerspective())
//              {
//                // show immediately.
//                try
//                {
//                  partPage.showView(viewId);
//                }
//                catch (PartInitException e)
//                {
//                  WorkbenchPlugin.log(getClass(), "identifierChanged", e); //$NON-NLS-1$
//                }
//              }
//              else
//              { // show when the perspective becomes active
//                partPage.getWorkbenchWindow()
//                .addPerspectiveListener(
//                    new IPerspectiveListener()
//                    {
//
//                      /* (non-Javadoc)
//                       * @see org.opencherry.ui.IPerspectiveListener#perspectiveActivated(org.opencherry.ui.IWorkbenchPage, org.opencherry.ui.IPerspectiveDescriptor)
//                       */
//                    public void perspectiveActivated(
//                          IWorkbenchPage page,
//                          IPerspectiveDescriptor newPerspective)
//                      {
//                        if (partPerspective == newPerspective)
//                        {
//                          partPage
//                          .getWorkbenchWindow()
//                          .removePerspectiveListener(
//                              this);
//                          try
//                          {
//                            page
//                            .showView(viewId);
//                          }
//                          catch (PartInitException e)
//                          {
//                            WorkbenchPlugin.log(getClass(), "perspectiveActivated", e); //$NON-NLS-1$
//                          }
//                        }
//                      }
//
//                      /* (non-Javadoc)
//                       * @see org.opencherry.ui.IPerspectiveListener#perspectiveChanged(org.opencherry.ui.IWorkbenchPage, org.opencherry.ui.IPerspectiveDescriptor, java.lang.String)
//                       */
//                    public void perspectiveChanged(
//                          IWorkbenchPage page,
//                          IPerspectiveDescriptor perspective,
//                          String changeId)
//                      {
//                        // no-op
//                      }
//                    });
//              }
//            }
//          }
//        }
//      }
//      );
    }

PartPane::Pointer LayoutHelper::CreateView(ViewFactory* factory,
    const std::string& viewId)
{
  WorkbenchPartReference::Pointer ref = factory->CreateView(
      ViewFactory::ExtractPrimaryId(viewId), ViewFactory::ExtractSecondaryId(
          viewId)).Cast<WorkbenchPartReference>();

  PartPane::Pointer newPart = ref->GetPane();
  return newPart;
}

}
