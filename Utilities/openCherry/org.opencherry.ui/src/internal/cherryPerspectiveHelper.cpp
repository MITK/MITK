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

#include "cherryPerspectiveHelper.h"

#include "../tweaklets/cherryGuiWidgetsTweaklet.h"
#include "cherryLayoutTree.h"
#include "cherryEditorSashContainer.h"

#include <Poco/RegularExpression.h>

namespace cherry
{

const int PerspectiveHelper::MIN_DETACH_WIDTH = 150;
const int PerspectiveHelper::MIN_DETACH_HEIGHT = 250;

PerspectiveHelper::MatchingPart::MatchingPart(const std::string& pid,
    const std::string& sid, StackablePart::Pointer part)
{
  this->pid = pid;
  this->sid = sid;
  this->part = part;
  this->len = pid.size() + sid.size();
  this->hasWildcard
      = (pid.find_first_of(PartPlaceholder::WILD_CARD) != std::string::npos) || (sid.find_first_of(PartPlaceholder::WILD_CARD) != std::string::npos);
}

bool PerspectiveHelper::CompareMatchingParts::operator()(const MatchingPart& m1, const MatchingPart& m2) const
{
  // specific ids always outweigh ids with wildcards
  if (m1.hasWildcard && !m2.hasWildcard)
  {
    return true;
  }
  if (!m1.hasWildcard && m2.hasWildcard)
  {
    return false;
  }
  // if both are specific or both have wildcards, simply compare based on length
  return (m2.len - m1.len) < 0;
}

PerspectiveHelper::PerspectiveHelper(WorkbenchPage::Pointer workbenchPage,
    ViewSashContainer::Pointer mainLayout, Perspective::Pointer persp)
 : page(workbenchPage), perspective(persp),
   mainLayout(mainLayout),
   detachable(false), active(false)
{
  //  this.page = workbenchPage;
  //  this.mainLayout = mainLayout;
  //  this.perspective = perspective;

  // Views can be detached if the feature is enabled (true by default,
  // use the plug-in customization file to disable), and if the platform
  // supports detaching.

  //TODO preference store
//  IPreferenceStore store = PlatformUI.getPreferenceStore();
//  this.detachable = store.getBoolean(
//      IWorkbenchPreferenceConstants.ENABLE_DETACHED_VIEWS);
  this->detachable = true;

  if (this->detachable)
  {
    // Check if some arbitrary Composite supports reparenting. If it
    // doesn't, views cannot be detached.

    void* client = workbenchPage->GetClientComposite();
    if (client == 0)
    {
      // The workbench page is not initialized. I don't think this can happen,
      // but if it does, silently set detachable to false.
      this->detachable = false;
    }
    else
    {
      this->detachable = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->IsReparentable(client);
    }
  }
}

void PerspectiveHelper::Activate(void* parent)
{

  if (active)
  {
    return;
  }

  parentWidget = parent;

  // Activate main layout
  // make sure all the views have been properly parented
  std::vector<PartPane::Pointer> children;
  this->CollectViewPanes(children, mainLayout->GetChildren());
  for (std::vector<PartPane::Pointer>::iterator iter = children.begin();
       iter != children.end(); ++iter)
  {
    PartPane::Pointer part = *iter;
    part->Reparent(parent);
  }
  mainLayout->CreateControl(parent);
  mainLayout->SetActive(true);

  std::string layoutDescr;
  mainLayout->DescribeLayout(layoutDescr);
  std::cout << "LAYOUT: " << layoutDescr << std::endl;

  // Open the detached windows.
  for (DetachedWindowsType::iterator iter = detachedWindowList.begin();
       iter != detachedWindowList.end(); ++iter)
  {
    (*iter)->Open();
  }

  this->EnableAllDrag();

  //  // Ensure that the maximized stack's presentation state is correct
  //  if (maximizedStackId != 0)
  //  {
  //    LayoutPart part = this->FindPart(maximizedStackId);
  //    if (part.Cast<PartStack>() != 0)
  //    {
  //      maximizedStack = (PartStack) part;
  //      maximizedStackId = 0;
  //    }
  //  }
  //
  //  // NOTE: we only handle ViewStacks here; Editor Stacks are handled by the
  //  // perspective
  //  if (maximizedStack instanceof ViewStack)
  //  {
  //    maximizedStack.setPresentationState(IStackPresentationSite.STATE_MAXIMIZED);
  //  }

  active = true;
}

void PerspectiveHelper::AddPart(StackablePart::Pointer part)
{

  // Look for a placeholder.
  PartPlaceholder::Pointer placeholder;
  StackablePart::Pointer testPart;
  std::string primaryId = part->GetId();
  std::string secondaryId;

  IViewReference::Pointer ref;
  if (part.Cast<PartPane> () != 0)
  {
    PartPane::Pointer pane = part.Cast<PartPane> ();
    ref = pane->GetPartReference().Cast<IViewReference> ();
    if (ref != 0)
      secondaryId = ref->GetSecondaryId();
  }
  if (secondaryId != "")
  {
    testPart = this->FindPart(primaryId, secondaryId);
  }
  else
  {
    testPart = this->FindPart(primaryId);
  }

  // validate the testPart
  if (testPart != 0 && testPart.Cast<PartPlaceholder>() != 0)
  {
    placeholder = testPart.Cast<PartPlaceholder> ();
  }

  // If there is no placeholder do a simple add. Otherwise, replace the
  // placeholder if its not a pattern matching placholder
  if (placeholder == 0)
  {
    part->Reparent(mainLayout->GetParent());
    LayoutPart::Pointer relative = mainLayout->FindBottomRight();
    if (relative != 0 && relative.Cast<IStackableContainer>() != 0)
    {
      IStackableContainer::Pointer stack =
          relative.Cast<IStackableContainer> ();
      if (stack->AllowsAdd(part))
      {
        mainLayout->Stack(part, stack);
      }
      else
      {
        mainLayout->AddPart(part);
      }
    }
    else
    {
      mainLayout->AddPart(part);
    }
  }
  else
  {
    IStackableContainer::Pointer container = placeholder->GetContainer();
    if (container != 0)
    {
      if (container.Cast<DetachedPlaceHolder> () != 0)
      {
        //Create a detached window add the part on it.
        DetachedPlaceHolder::Pointer holder = container.Cast<DetachedPlaceHolder>();
        detachedPlaceHolderList.remove(holder);
        container->Remove(testPart);
        DetachedWindow::Pointer window = new DetachedWindow(page);
        detachedWindowList.push_back(window);
        window->Create();
        part->CreateControl(window->GetShell());
        // Open window.
        window->GetShell()->SetBounds(holder->GetBounds());
        window->Open();
        // add part to detached window.
        PartPane::Pointer pane = part.Cast<PartPane>();
        window->Add(pane);
        std::list<StackablePart::Pointer> otherChildren = holder->GetChildren();
        for (std::list<StackablePart::Pointer>::iterator iter = otherChildren.begin();
             iter != otherChildren.end(); ++iter)
        {
          part->GetContainer()->Add(*iter);
        }
      }
      else
      {
        // show parent if necessary
        if (container.Cast<ContainerPlaceholder> () != 0)
        {
          ContainerPlaceholder::Pointer containerPlaceholder =
              container.Cast<ContainerPlaceholder>();
          ILayoutContainer::Pointer parentContainer =
              containerPlaceholder->GetContainer();
          container = containerPlaceholder->GetRealContainer();
          if (container.Cast<LayoutPart> () != 0)
          {
            parentContainer->Replace(containerPlaceholder,
                container.Cast<LayoutPart>());
          }
          containerPlaceholder->SetRealContainer(0);
        }

        // reparent part.
        //if (!(container instanceof ViewStack))
        //  {
        // We don't need to reparent children of PartTabFolders since they will automatically
        // reparent their children when they become visible. This if statement used to be
        // part of an else branch. Investigate if it is still necessary.
        //    part.reparent(mainLayout.getParent());
        //  }

        // see if we should replace the placeholder
        if (placeholder->HasWildCard())
        {
          //              if (container instanceof PartSashContainer)
          //              {
          //                ((PartSashContainer) container) .addChildForPlaceholder(part,
          //                    placeholder);
          //              }
          //              else
          //              {
          container->Add(part);
          //              }
        }
        else
        {
          container->Replace(placeholder, part);
        }
      }
    }
  }
}

void PerspectiveHelper::AttachPart(IViewReference::Pointer ref)
{
  PartPane::Pointer pane = ref.Cast<WorkbenchPartReference>()->GetPane();

  // Restore any maximized part before re-attaching.
  // Note that 'getMaximizedStack != 0' implies 'useNewMinMax'
//  if (getMaximizedStack() != 0)
//  {
//    getMaximizedStack().setState(IStackPresentationSite.STATE_RESTORED);
//  }

  this->DerefPart(pane);
  this->AddPart(pane);
  this->BringPartToTop(pane);
  pane->SetFocus();
}

bool PerspectiveHelper::CanDetach()
{
  return detachable;
}

bool PerspectiveHelper::BringPartToTop(StackablePart::Pointer part)
{
  IStackableContainer::Pointer container = part->GetContainer();
  if (container != 0 && container.Cast<PartStack> () != 0)
  {
    PartStack::Pointer folder = container.Cast<PartStack> ();
    if (folder->GetSelection() != part)
    {
      folder->SetSelection(part);
      return true;
    }
  }
  return false;
}

bool PerspectiveHelper::IsPartVisible(IWorkbenchPartReference::Pointer partRef)
{
  StackablePart::Pointer foundPart;
  if (partRef.Cast<IViewReference> () != 0)
  {
    foundPart = this->FindPart(partRef->GetId(),
        partRef.Cast<IViewReference>()->GetSecondaryId());
  }
  else
  {
    foundPart = this->FindPart(partRef->GetId());
  }
  if (foundPart == 0)
  {
    return false;
  }
  if (foundPart.Cast<PartPlaceholder> () != 0)
  {
    return false;
  }

  IStackableContainer::Pointer container = foundPart->GetContainer();

  if (container.Cast<ContainerPlaceholder> () != 0)
  {
    return false;
  }

  if (container.Cast<PartStack> () != 0)
  {
    PartStack::Pointer folder = container.Cast<PartStack>();
    StackablePart::Pointer visiblePart = folder->GetSelection();
    if (visiblePart == 0)
    {
      return false;
    }
    return partRef == visiblePart.Cast<PartPane>()->GetPartReference();
  }
  return true;
}

bool PerspectiveHelper::WillPartBeVisible(const std::string& partId)
{
  return this->WillPartBeVisible(partId, 0);
}

bool PerspectiveHelper::WillPartBeVisible(const std::string& partId,
    const std::string& secondaryId)
{
  StackablePart::Pointer part = this->FindPart(partId, secondaryId);
  if (part == 0)
  {
    return false;
  }
  IStackableContainer::Pointer container = part->GetContainer();
  if (container != 0 && container.Cast<ContainerPlaceholder> () != 0)
  {
    container
        = container.Cast<ContainerPlaceholder>()->GetRealContainer();
  }

  if (container != 0 && container.Cast<PartStack> () != 0)
  {
    PartStack::Pointer folder = container.Cast<PartStack>();
    if (folder->GetSelection() == 0)
    {
      return false;
    }
    return part->GetCompoundId() == folder->GetSelection().Cast<PartPane>()->GetCompoundId();
  }
  return true;
}

std::vector<PartPlaceholder::Pointer> PerspectiveHelper::CollectPlaceholders()
{
  // Scan the main window.
  std::vector<PartPlaceholder::Pointer> results = this->CollectPlaceholders(
      mainLayout->GetChildren());

  // Scan each detached window.
  if (detachable)
  {
    for (DetachedWindowsType::iterator winIter = detachedWindowList.begin();
         winIter != detachedWindowList.end(); ++winIter)
    {
      DetachedWindow::Pointer win = *winIter;
      std::list<StackablePart::Pointer> moreResults = win->GetChildren();
      if (moreResults.size() > 0)
      {
        for (std::list<StackablePart::Pointer>::iterator iter = moreResults.begin();
             iter != moreResults.end(); ++iter)
        {
          if (iter->Cast<PartPlaceholder>() != 0)
            results.push_back(iter->Cast<PartPlaceholder>());
        }
      }
    }
  }
  return results;
}

std::vector<PartPlaceholder::Pointer> PerspectiveHelper::CollectPlaceholders(
    const std::list<LayoutPart::Pointer>& parts)
{
  std::vector<PartPlaceholder::Pointer> result;

  for (std::list<LayoutPart::Pointer>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    if (part.Cast<ILayoutContainer> () != 0)
    {
      // iterate through sub containers to find sub-parts
      std::vector<PartPlaceholder::Pointer> newParts = this->CollectPlaceholders(
          part.Cast<ILayoutContainer>()->GetChildren());
      std::vector<PartPlaceholder::Pointer> newResult = result;
      std::copy(newParts.begin(), newParts.end(), newResult.end());
      result = newResult;
    }
    else if (part.Cast<IStackableContainer> () != 0)
    {
      std::list<StackablePart::Pointer> children = part.Cast<IStackableContainer>()->GetChildren();
      for (std::list<StackablePart::Pointer>::iterator partIter = children.begin();
           partIter != children.end(); ++partIter)
      {
        if (partIter->Cast<PartPlaceholder>() != 0)
          result.push_back(partIter->Cast<PartPlaceholder>());
      }
    }
  }

  return result;
}

void PerspectiveHelper::CollectViewPanes(std::vector<PartPane::Pointer>& result)
{
  // Scan the main window.
  this->CollectViewPanes(result, mainLayout->GetChildren());

  // Scan each detached window.
  if (detachable)
  {
    for (DetachedWindowsType::iterator winIter = detachedWindowList.begin();
         winIter != detachedWindowList.end(); ++winIter)
    {
      DetachedWindow::Pointer win = *winIter;
      std::list<StackablePart::Pointer> moreResults = win->GetChildren();
      for (std::list<StackablePart::Pointer>::iterator iter = moreResults.begin();
           iter != moreResults.end(); ++iter)
      {
        if (iter->Cast<PartPane>() != 0)
          result.push_back(iter->Cast<PartPane>());
      }
    }
  }
}

void PerspectiveHelper::CollectViewPanes(std::vector<PartPane::Pointer>& result,
    const std::list<LayoutPart::Pointer>& parts)
{
  for (std::list<LayoutPart::Pointer>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    if (part.Cast<IStackableContainer> () != 0)
    {
      std::list<StackablePart::Pointer> children = part.Cast<IStackableContainer>()->GetChildren();
      for (std::list<StackablePart::Pointer>::iterator partIter = children.begin();
           partIter != children.end(); ++partIter)
      {
        if (partIter->Cast<PartPane>() != 0)
          result.push_back(partIter->Cast<PartPane>());
      }
    }
    else if (part.Cast<ILayoutContainer> () != 0)
    {
      this->CollectViewPanes(result, part.Cast<ILayoutContainer>()->GetChildren());
    }
  }
}

void PerspectiveHelper::Deactivate()
{
  if (!active)
  {
    return;
  }

  this->DisableAllDrag();

  // Reparent all views to the main window
  void* parent = mainLayout->GetParent();
  std::vector<PartPane::Pointer> children;
  this->CollectViewPanes(children, mainLayout->GetChildren());

  for (DetachedWindowsType::iterator winIter = detachedWindowList.begin();
       winIter != detachedWindowList.end(); ++winIter)
  {
    DetachedWindow::Pointer window = *winIter;
    std::list<StackablePart::Pointer> moreResults = window->GetChildren();
    for (std::list<StackablePart::Pointer>::iterator iter = moreResults.begin();
         iter != moreResults.end(); ++iter)
    {
      if (iter->Cast<PartPane>() != 0)
        children.push_back(iter->Cast<PartPane>());
    }
  }

  // *** Do we even need to do this if detached windows not supported?
  for (std::vector<PartPane::Pointer>::iterator itr = children.begin();
       itr != children.end(); ++itr)
  {
    PartPane::Pointer part = *itr;
    part->Reparent(parent);
  }

  // Dispose main layout.

  mainLayout->SetActive(false);

  // Dispose the detached windows
  for (DetachedWindowsType::iterator iter = detachedWindowList.begin();
       iter != detachedWindowList.end(); ++iter)
  {
    (*iter)->Close();
  }

  active = false;
}

PerspectiveHelper::~PerspectiveHelper()
{
  mainLayout->DisposeSashes();
}

void PerspectiveHelper::DescribeLayout(std::string& buf) const
{

  if (detachable)
  {
    if (detachedWindowList.size() != 0)
    {
      buf.append("detachedWindows ("); //$NON-NLS-1$

      for (DetachedWindowsType::const_iterator winIter = detachedWindowList.begin();
           winIter != detachedWindowList.end(); ++winIter)
      {
        DetachedWindow::ConstPointer window = *winIter;
        std::list<StackablePart::Pointer> children = window->GetChildren();
        unsigned int j = 0;
        if (children.size() != 0)
        {
          buf.append("dWindow ("); //$NON-NLS-1$
          for (std::list<StackablePart::Pointer>::iterator partIter = children.begin();
               partIter != children.end(); ++partIter, ++j)
          {
            if (partIter->Cast<PartPlaceholder>() != 0)
              buf.append(partIter->Cast<PartPlaceholder>()->GetPlaceHolderId());
            else if (partIter->Cast<PartPane>() != 0)
              buf.append(
                partIter->Cast<PartPane>()->GetPartReference()->GetPartName());

            if (j < (children.size() - 1))
            {
              buf.append(", "); //$NON-NLS-1$
            }
          }
          buf.append(")"); //$NON-NLS-1$
        }

      }
      buf.append("), "); //$NON-NLS-1$
    }
  }

  this->GetLayout()->DescribeLayout(buf);
}

void PerspectiveHelper::DerefPart(StackablePart::Pointer part)
{
//  if (part.Cast<PartPane> () != 0)
//  {
//    IViewReference::Pointer ref = ((ViewPane) part).getViewReference();
//    if (perspective.isFastView(ref))
//    {
//      // Special check: if it's a fast view then it's actual ViewStack
//      // may only contain placeholders and the stack is represented in
//      // the presentation by a container placeholder...make sure the
//      // PartPlaceHolder for 'ref' is removed from the ViewStack
//      String id = perspective.getFastViewManager().getIdForRef(ref);
//      LayoutPart parentPart = findPart(id, 0);
//      if (parentPart.Cast<ContainerPlaceholder> () != 0)
//      {
//        ViewStack vs =
//            (ViewStack) ((ContainerPlaceholder) parentPart).getRealContainer();
//        std::vector<LayoutPart::Pointer> kids = vs.getChildren();
//        for (int i = 0; i < kids.length; i++)
//        {
//          if (kids[i].Cast<PartPlaceholder> () != 0)
//          {
//            if (ref.getId().equals(kids[i].id))
//              vs.remove(kids[i]);
//          }
//        }
//      }
//      perspective.getFastViewManager().removeViewReference(ref, true, true);
//    }
//  }

  // Get vital part stats before reparenting.
  IStackableContainer::Pointer oldContainer = part->GetContainer();
  bool wasDocked = part->IsDocked();
  Shell::Pointer oldShell = part->GetShell();

  // Reparent the part back to the main window
  part->Reparent(mainLayout->GetParent());

  // Update container.
  if (oldContainer == 0)
  {
    return;
  }

  oldContainer->Remove(part);

  IStackableContainer::ChildrenType children = oldContainer->GetChildren();
  if (wasDocked)
  {
    bool hasChildren = (children.size() > 0);
    if (hasChildren)
    {
      // make sure one is at least visible
      int childVisible = 0;
      for (IStackableContainer::ChildrenType::iterator iter = children.begin();
           iter != children.end(); ++iter)
      {
        if ((*iter)->GetControl() != 0)
        {
          childVisible++;
        }
      }

      // none visible, then reprarent and remove container
      if (oldContainer.Cast<PartStack> () != 0)
      {
        PartStack::Pointer folder = oldContainer.Cast<PartStack>();

        // Is the part in the trim?
        bool inTrim = false;
//        // Safety check...there may be no FastViewManager
//        if (perspective.getFastViewManager() != 0)
//          inTrim
//              = perspective.getFastViewManager().getFastViews(folder.getID()).size()
//                  > 0;

        if (childVisible == 0 && !inTrim)
        {
          ILayoutContainer::Pointer parentContainer = folder->GetContainer();
          hasChildren = folder->GetChildren().size() > 0;

          // We maintain the stack as a place-holder if it has children
          // (which at this point would represent view place-holders)
          if (hasChildren)
          {
            //folder->Dispose();

            // replace the real container with a ContainerPlaceholder
            ContainerPlaceholder::Pointer placeholder =
                new ContainerPlaceholder(folder->GetID());
            placeholder->SetRealContainer(folder);
            parentContainer->Replace(folder, placeholder);
          }
        }
        else if (childVisible == 1)
        {
          LayoutTree::Pointer layout = mainLayout->GetLayoutTree();
          layout = layout->Find(folder);
          layout->SetBounds(layout->GetBounds());
        }
      }
    }

    if (!hasChildren)
    {
      // There are no more children in this container, so get rid of
      // it
      if (oldContainer.Cast<LayoutPart> () != 0)
      {
        LayoutPart::Pointer parent = oldContainer.Cast<LayoutPart>();
        ILayoutContainer::Pointer parentContainer = parent->GetContainer();
        if (parentContainer != 0)
        {
          parentContainer->Remove(parent);
          //parent->Dispose();
        }
      }
    }
  }
  else if (!wasDocked)
  {
    if (children.empty())
    {
      // There are no more children in this container, so get rid of
      // it
      // Turn on redraw again just in case it was off.
      //oldShell.setRedraw(true);
      DetachedWindow::Pointer w = oldShell->GetData().Cast<DetachedWindow>();
      oldShell->Close();
      detachedWindowList.remove(w);
    }
    else
    {
      // There are children. If none are visible hide detached
      // window.
      bool allInvisible = true;
      for (IStackableContainer::ChildrenType::iterator iter = children.begin();
           iter != children.end(); ++iter)
      {
        if (iter->Cast<PartPlaceholder> () == 0)
        {
          allInvisible = false;
          break;
        }
      }
      if (allInvisible)
      {
        DetachedPlaceHolder::Pointer placeholder = new DetachedPlaceHolder("",
           oldShell->GetBounds());
        for (IStackableContainer::ChildrenType::iterator iter = children.begin();
             iter != children.end(); ++iter)
        {
          oldContainer->Remove(*iter);
          (*iter)->SetContainer(placeholder);
          placeholder->Add(*iter);
        }
        detachedPlaceHolderList.push_back(placeholder);
        DetachedWindow::Pointer w = oldShell->GetData().Cast<DetachedWindow>();
        oldShell->Close();
        detachedWindowList.remove(w);
      }
    }
  }

}

void PerspectiveHelper::Detach(LayoutPart::Pointer part, int x, int y)
{

  // Detaching is disabled on some platforms ..
  if (!detachable)
  {
    return;
  }

  // Calculate detached window size.
  Point size = part->GetSize();
  if (size.x == 0 || size.y == 0)
  {
    ILayoutContainer::Pointer container = part->GetContainer();
    if (container.Cast<LayoutPart> () != 0)
    {
      size = container.Cast<LayoutPart>()->GetSize();
    }
  }
  int width = std::max<int>(size.x, MIN_DETACH_WIDTH);
  int height = std::max<int>(size.y, MIN_DETACH_HEIGHT);

  // Create detached window.
  DetachedWindow::Pointer window = new DetachedWindow(page);
  detachedWindowList.push_back(window);

  // Open window.
  window->Create();
  window->GetShell()->SetBounds(x, y, width, height);
  window->Open();

  if (part.Cast<PartStack> () != 0)
  {
    //window.getShell().setRedraw(false);
    //parentWidget.setRedraw(false);
    PartStack::Pointer stack = part.Cast<PartStack>();
    StackablePart::Pointer visiblePart = stack->GetSelection();
    IStackableContainer::ChildrenType children = stack->GetChildren();
    for (IStackableContainer::ChildrenType::iterator iter = children.begin();
         iter != children.end(); ++iter)
    {
      if (!(*iter)->IsPlaceHolder())
      {
        // remove the part from its current container
        this->DerefPart(*iter);
        // add part to detached window.
        window->Add(*iter);
      }
    }
    if (visiblePart != 0)
    {
      this->BringPartToTop(visiblePart);
      visiblePart->SetFocus();
    }
    //window.getShell().setRedraw(true);
    //parentWidget.setRedraw(true);
  }

}

void PerspectiveHelper::DetachPart(StackablePart::Pointer part, int x, int y)
{

  // Detaching is disabled on some platforms ..
  if (!detachable)
  {
    return;
  }

  // Calculate detached window size.
  Point size = part->GetSize();
  if (size.x == 0 || size.y == 0)
  {
    IStackableContainer::Pointer container = part->GetContainer();
    if (container.Cast<LayoutPart> () != 0)
    {
      size = container.Cast<LayoutPart>()->GetSize();
    }
  }
  int width = std::max<int>(size.x, MIN_DETACH_WIDTH);
  int height = std::max<int>(size.y, MIN_DETACH_HEIGHT);

  // Create detached window.
  DetachedWindow::Pointer window = new DetachedWindow(page);
  detachedWindowList.push_back(window);

  // Open window.
  window->Create();
  window->GetShell()->SetBounds(x, y, width, height);
  window->Open();

  // remove the part from its current container
  this->DerefPart(part);
  // add part to detached window.
  window->Add(part);
  part->SetFocus();

}

void PerspectiveHelper::DetachPart(IViewReference::Pointer ref)
{
  PartPane::Pointer pane = ref.Cast<WorkbenchPartReference>()->GetPane();
  if (this->CanDetach() && pane != 0)
  {
//    if (getMaximizedStack() != 0)
//      getMaximizedStack().setState(IStackPresentationSite.STATE_RESTORED);

    Rectangle bounds = pane->GetParentBounds();
    this->DetachPart(pane, bounds.x, bounds.y);
  }
}

void PerspectiveHelper::AddDetachedPart(StackablePart::Pointer part)
{
  // Calculate detached window size.
  Rectangle bounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShell(parentWidget)->GetBounds();
  bounds.x = bounds.x + (bounds.width - 300) / 2;
  bounds.y = bounds.y + (bounds.height - 300) / 2;

  this->AddDetachedPart(part, bounds);
}

void PerspectiveHelper::AddDetachedPart(StackablePart::Pointer part,
    const Rectangle& bounds)
{
  // Detaching is disabled on some platforms ..
  if (!detachable)
  {
    this->AddPart(part);
    return;
  }

  // Create detached window.
  DetachedWindow::Pointer window = new DetachedWindow(page);
  detachedWindowList.push_back(window);
  window->Create();

  // add part to detached window.
  part->CreateControl(window->GetShell()->GetControl());
  window->Add(part);

  // Open window.
  window->GetShell()->SetBounds(bounds.x, bounds.y, bounds.width, bounds.height);
  window->Open();

  part->SetFocus();

}

void PerspectiveHelper::DisableAllDrag()
{
  //TODO DND
  //DragUtil.removeDragTarget(0, dragTarget);
}

void PerspectiveHelper::EnableAllDrag()
{
  //TODO DND
  //DragUtil.addDragTarget(0, dragTarget);
}

StackablePart::Pointer PerspectiveHelper::FindPart(const std::string& id)
{
  return this->FindPart(id, "");
}

StackablePart::Pointer PerspectiveHelper::FindPart(const std::string& primaryId,
    const std::string& secondaryId)
{
  // check main window.
  std::vector<MatchingPart> matchingParts;
  StackablePart::Pointer part = (secondaryId != "") ? this->FindPart(primaryId, secondaryId,
      mainLayout->GetChildren(), matchingParts) : this->FindPart(primaryId,
      mainLayout->GetChildren(), matchingParts);
  if (part != 0)
  {
    return part;
  }

  // check each detached windows.
  for (DetachedWindowsType::iterator iter = detachedWindowList.begin();
       iter != detachedWindowList.end(); ++iter)
  {
    DetachedWindow::Pointer window = *iter;
    part = (secondaryId != "") ? this->FindPart(primaryId, secondaryId,
        window->GetChildren(), matchingParts) : this->FindPart(primaryId,
        window->GetChildren(), matchingParts);
    if (part != 0)
    {
      return part;
    }
  }
  for (DetachedPlaceHoldersType::iterator iter = detachedPlaceHolderList.begin();
       iter != detachedPlaceHolderList.end(); ++iter)
  {
    DetachedPlaceHolder::Pointer holder = *iter;
    part = (secondaryId != "") ? this->FindPart(primaryId, secondaryId,
        holder->GetChildren(), matchingParts) : this->FindPart(primaryId,
        holder->GetChildren(), matchingParts);
    if (part != 0)
    {
      return part;
    }
  }

  // sort the matching parts
  if (matchingParts.size() > 0)
  {
    std::partial_sort(matchingParts.begin(), (matchingParts.begin()), matchingParts.end(), CompareMatchingParts());
    const MatchingPart& mostSignificantPart = matchingParts.front();
    return mostSignificantPart.part;
  }

  // Not found.
  return 0;
}

StackablePart::Pointer PerspectiveHelper::FindPart(const std::string& id,
    const std::list<LayoutPart::Pointer>& parts,
    std::vector<MatchingPart>& matchingParts)
{
  for (std::list<LayoutPart::Pointer>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    if (part.Cast<ILayoutContainer> () != 0)
    {
      StackablePart::Pointer result = this->FindPart(id, part.Cast<ILayoutContainer>()->GetChildren(),
          matchingParts);
      return result;
    }
    else if (part.Cast<IStackableContainer>() != 0)
    {
      StackablePart::Pointer result = this->FindPart(id, part.Cast<IStackableContainer>()->GetChildren(),
          matchingParts);
      return result;
    }
  }
  return 0;
}

LayoutPart::Pointer PerspectiveHelper::FindLayoutPart(const std::string& id,
    const std::list<LayoutPart::Pointer>& parts,
    std::vector<MatchingPart>& matchingParts)
{
  for (std::list<LayoutPart::Pointer>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    // check for part equality, parts with secondary ids fail
    if (part->GetID() == id)
    {
      return part;
    }
    else if (part.Cast<EditorSashContainer>() != 0)
    {
      // Skip.
    }
    else if (part.Cast<ILayoutContainer> () != 0)
    {
      part = this->FindLayoutPart(id, part.Cast<ILayoutContainer>()->GetChildren(),
          matchingParts);
      return part;
    }
  }
  return 0;
}

StackablePart::Pointer PerspectiveHelper::FindPart(const std::string& id,
    const std::list<StackablePart::Pointer>& parts,
    std::vector<MatchingPart>& matchingParts)
{
  for (std::list<StackablePart::Pointer>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter)
  {
    StackablePart::Pointer part = *iter;
    // check for part equality, parts with secondary ids fail
    if (part->GetId() == id)
    {
      if (part.Cast<PartPane> () != 0)
      {
        PartPane::Pointer pane = part.Cast<PartPane>();
        IViewReference::Pointer ref = pane->GetPartReference().Cast<IViewReference>();
        if (ref->GetSecondaryId() != "")
        {
          continue;
        }
      }
      return part;
    }
    // check pattern matching placeholders

    else if (part.Cast<PartPlaceholder>() != 0
        && part.Cast<PartPlaceholder>()->HasWildCard())
    {
      Poco::RegularExpression re(id, Poco::RegularExpression::RE_CASELESS);
      if (re.match(part->GetId()))
      {
        matchingParts.push_back(MatchingPart(part->GetId(), "", part));
      }
//      StringMatcher sm = new StringMatcher(part.getID(), true, false);
//      if (sm.match(id))
//      {
//        matchingParts .add(new MatchingPart(part.getID(), 0, part));
//      }
    }
  }
  return 0;
}

StackablePart::Pointer PerspectiveHelper::FindPart(const std::string& primaryId,
    const std::string& secondaryId,
    const std::list<LayoutPart::Pointer>& parts,
    std::vector<MatchingPart>& matchingParts)
{
  for (std::list<LayoutPart::Pointer>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    // check containers first
    if (part.Cast<EditorSashContainer>() != 0)
    {
      // skip
    }
    else if (part.Cast<ILayoutContainer> () != 0)
    {
      StackablePart::Pointer testPart = this->FindPart(primaryId, secondaryId,
          part.Cast<ILayoutContainer>()->GetChildren(), matchingParts);
      if (testPart != 0)
      {
        return testPart;
      }
    }
  }
  return 0;
}

StackablePart::Pointer PerspectiveHelper::FindPart(const std::string& primaryId,
    const std::string& secondaryId,
    const std::list<StackablePart::Pointer>& parts,
    std::vector<MatchingPart>& matchingParts)
{
  for (std::list<StackablePart::Pointer>::const_iterator iter = parts.begin();
       iter != parts.end(); ++iter)
  {
    StackablePart::Pointer part = *iter;
    // check for view part equality
    if (part.Cast<PartPane> () != 0)
    {
      PartPane::Pointer pane = part.Cast<PartPane>();
      IViewReference::Pointer ref = pane->GetPartReference().Cast<IViewReference>();
      if (ref->GetId() == primaryId && ref->GetSecondaryId() == secondaryId)
      {
        return part;
      }
    }
    // check placeholders
    else if (part.Cast<PartPlaceholder> () != 0)
    {
      std::string id = part->GetId();

      // optimization: don't bother parsing id if it has no separator -- it can't match
      std::string phSecondaryId = ViewFactory::ExtractSecondaryId(id);
      if (phSecondaryId == "")
      {
        // but still need to check for wildcard case
        if (id == PartPlaceholder::WILD_CARD)
        {
          matchingParts.push_back(MatchingPart(id, "", part));
        }
        continue;
      }

      std::string phPrimaryId = ViewFactory::ExtractPrimaryId(id);
      // perfect matching pair
      if (phPrimaryId == primaryId && phSecondaryId == secondaryId)
      {
        return part;
      }
      // check for partial matching pair
      Poco::RegularExpression pre(phPrimaryId, Poco::RegularExpression::RE_CASELESS);
      if (pre.match(primaryId))
      {
        Poco::RegularExpression sre(phSecondaryId, Poco::RegularExpression::RE_CASELESS);
        if (sre.match(secondaryId))
        {
          matchingParts.push_back(MatchingPart(phPrimaryId, phSecondaryId, part));
        }
      }
    }
  }
  return 0;
}

bool PerspectiveHelper::HasPlaceholder(const std::string& id)
{
  return this->HasPlaceholder(id, 0);
}

bool PerspectiveHelper::HasPlaceholder(const std::string& primaryId,
    const std::string& secondaryId)
{
  StackablePart::Pointer testPart;
  if (secondaryId == "")
  {
    testPart = this->FindPart(primaryId);
  }
  else
  {
    testPart = this->FindPart(primaryId, secondaryId);
  }
  return (testPart != 0 && testPart.Cast<PartPlaceholder> () != 0);
}

PartSashContainer::Pointer PerspectiveHelper::GetLayout() const
{
  return mainLayout;
}

bool PerspectiveHelper::IsActive()
{
  return active;
}

float PerspectiveHelper::GetDockingRatio(StackablePart::Pointer source,
    LayoutPart::Pointer target)
{
  if ((source.Cast<PartPane> () != 0 || source.Cast<PartStack> () != 0)
      && target.Cast<EditorSashContainer> () != 0)
  {
    return 0.25f;
  }
  return 0.5f;
}

void PerspectiveHelper::RemovePart(StackablePart::Pointer part)
{

  // Reparent the part back to the main window
  void* parent = mainLayout->GetParent();
  part->Reparent(parent);

  // Replace part with a placeholder
  IStackableContainer::Pointer container = part->GetContainer();
  if (container != 0)
  {
    std::string placeHolderId = part->GetPlaceHolderId();
    container->Replace(part, new PartPlaceholder(placeHolderId));

//    // If the parent is root we're done. Do not try to replace
//    // it with placeholder.
//    if (container == mainLayout)
//    {
//      return;
//    }

    // If the parent is empty replace it with a placeholder.
    std::list<StackablePart::Pointer> children = container->GetChildren();

    bool allInvisible = true;
    for (std::list<StackablePart::Pointer>::iterator childIter = children.begin();
         childIter != children.end(); ++childIter)
    {
      if (childIter->Cast<PartPlaceholder> () == 0)
      {
        allInvisible = false;
        break;
      }
    }
    if (allInvisible && (container.Cast<LayoutPart> () != 0))
    {
      // what type of window are we in?
      LayoutPart::Pointer cPart = container.Cast<LayoutPart>();
      //Window oldWindow = cPart.getWindow();
      bool wasDocked = cPart->IsDocked();
      Shell::Pointer oldShell = cPart->GetShell();
      if (wasDocked)
      {

//        // PR 1GDFVBY: ViewStack not disposed when page
//        // closed.
//        if (container.Cast<PartStack> () != 0)
//        {
//          container.Cast<PartStack>()->Dispose();
//        }

        // replace the real container with a
        // ContainerPlaceholder
        ILayoutContainer::Pointer parentContainer = cPart->GetContainer();
        ContainerPlaceholder::Pointer placeholder =
            new ContainerPlaceholder(cPart->GetID());
        placeholder->SetRealContainer(container);
        parentContainer->Replace(cPart, placeholder);

      }
      else
      {
        DetachedPlaceHolder::Pointer placeholder =
            new DetachedPlaceHolder("", oldShell->GetBounds()); //$NON-NLS-1$
        for (std::list<StackablePart::Pointer>::iterator childIter2 = children.begin();
             childIter2 != children.end(); ++childIter2)
        {
          (*childIter2)->GetContainer()->Remove(*childIter2);
          (*childIter2)->SetContainer(placeholder);
          placeholder->Add(*childIter2);
        }
        detachedPlaceHolderList.push_back(placeholder);
        DetachedWindow::Pointer w = oldShell->GetData().Cast<DetachedWindow>();
        oldShell->Close();
        detachedWindowList.remove(w);
      }

    }
  }
}

void PerspectiveHelper::ReplacePlaceholderWithPart(StackablePart::Pointer part)
{

  // Look for a PartPlaceholder that will tell us how to position this
  // object
  std::vector<PartPlaceholder::Pointer> placeholders = this->CollectPlaceholders();
  for (unsigned int i = 0; i < placeholders.size(); i++)
  {
    if (placeholders[i]->GetCompoundId() == part->GetCompoundId())
    {
      // found a matching placeholder which we can replace with the
      // new View
      IStackableContainer::Pointer container = placeholders[i]->GetContainer();
      if (container != 0)
      {
        if (container.Cast<ContainerPlaceholder> () != 0)
        {
          // One of the children is now visible so replace the
          // ContainerPlaceholder with the real container
          ContainerPlaceholder::Pointer containerPlaceholder =
              container.Cast<ContainerPlaceholder>();
          ILayoutContainer::Pointer parentContainer =
              containerPlaceholder->GetContainer();
          container
              = containerPlaceholder->GetRealContainer();
          if (container.Cast<LayoutPart> () != 0)
          {
            parentContainer->Replace(containerPlaceholder,
                container.Cast<LayoutPart>());
          }
          containerPlaceholder->SetRealContainer(0);
        }
        container->Replace(placeholders[i], part);
        return;
      }
    }
  }

}

bool PerspectiveHelper::RestoreState(IMemento::Pointer memento)
{
  //TODO PerspectiveHelper restore state
//  // Restore main window.
//  IMemento::Pointer childMem = memento .getChild(IWorkbenchConstants.TAG_MAIN_WINDOW);
//  IStatus r = mainLayout.restoreState(childMem);
//
//  // Restore each floating window.
//  if (detachable)
//  {
//    IMemento detachedWindows[] = memento .getChildren(
//        IWorkbenchConstants.TAG_DETACHED_WINDOW);
//    for (int nX = 0; nX < detachedWindows.length; nX++)
//    {
//      DetachedWindow win = new DetachedWindow(page);
//      detachedWindowList.add(win);
//      win.restoreState(detachedWindows[nX]);
//    }
//    IMemento childrenMem[] = memento .getChildren(
//        IWorkbenchConstants.TAG_HIDDEN_WINDOW);
//    for (int i = 0, length = childrenMem.length; i < length; i++)
//    {
//      DetachedPlaceHolder holder =
//          new DetachedPlaceHolder("", new Rectangle(0, 0, 0, 0)); //$NON-NLS-1$
//      holder.restoreState(childrenMem[i]);
//      detachedPlaceHolderList.add(holder);
//    }
//  }
//
//  // Get the cached id of the currently maximized stack
//  maximizedStackId = childMem.getString(IWorkbenchConstants.TAG_MAXIMIZED);
//
//  return r;
  return true;
}

bool PerspectiveHelper::SaveState(IMemento::Pointer memento)
{
  //TODO PerspectiveHelper save state
//  // Persist main window.
//  IMemento childMem = memento .createChild(IWorkbenchConstants.TAG_MAIN_WINDOW);
//  IStatus r = mainLayout.saveState(childMem);
//
//  if (detachable)
//  {
//    // Persist each detached window.
//    for (int i = 0, length = detachedWindowList.size(); i < length; i++)
//    {
//      DetachedWindow window = (DetachedWindow) detachedWindowList .get(i);
//      childMem = memento .createChild(IWorkbenchConstants.TAG_DETACHED_WINDOW);
//      window.saveState(childMem);
//    }
//    for (int i = 0, length = detachedPlaceHolderList.size(); i < length; i++)
//    {
//      DetachedPlaceHolder holder =
//          (DetachedPlaceHolder) detachedPlaceHolderList .get(i);
//      childMem = memento .createChild(IWorkbenchConstants.TAG_HIDDEN_WINDOW);
//      holder.saveState(childMem);
//    }
//  }
//
//  // Write out the id of the maximized (View) stack (if any)
//  // NOTE: we only write this out if it's a ViewStack since the
//  // Editor Area is handled by the perspective
//  if (maximizedStack.Cast<PartStack> () != 0)
//  {
//    childMem.putString(IWorkbenchConstants.TAG_MAXIMIZED,
//        maximizedStack.getID());
//  }
//  else if (maximizedStackId != 0)
//  {
//    // Maintain the cache if the perspective has never been activated
//    childMem.putString(IWorkbenchConstants.TAG_MAXIMIZED, maximizedStackId);
//  }
//
//  return r;
  return true;
}

void PerspectiveHelper::UpdateBoundsMap()
{
  boundsMap.clear();

  // Walk the layout gathering the current bounds of each stack
  // and the editor area
  std::list<LayoutPart::Pointer> kids = mainLayout->GetChildren();
  for (std::list<LayoutPart::Pointer>::iterator iter = kids.begin();
       iter != kids.end(); ++iter)
  {
    if (iter->Cast<PartStack> () != 0)
    {
      PartStack::Pointer vs = iter->Cast<PartStack>();
      boundsMap.insert(std::make_pair(vs->GetID(), vs->GetBounds()));
    }
    else if (iter->Cast<EditorSashContainer> () != 0)
    {
      EditorSashContainer::Pointer esc = iter->Cast<EditorSashContainer>();
      boundsMap.insert(std::make_pair(esc->GetID(), esc->GetBounds()));
    }
  }
}

void PerspectiveHelper::ResetBoundsMap()
{
  boundsMap.clear();
}

Rectangle PerspectiveHelper::GetCachedBoundsFor(const std::string& id)
{
  return boundsMap[id];
}

}
