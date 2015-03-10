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

#include "tweaklets/berryGuiWidgetsTweaklet.h"

#include "berryPerspectiveHelper.h"

#include "berryLayoutTree.h"
#include "berryEditorSashContainer.h"
#include "berryDragUtil.h"
#include "berryPresentationFactoryUtil.h"
#include "berryWorkbenchConstants.h"

#include <berryDebugUtil.h>

namespace berry
{

const int PerspectiveHelper::MIN_DETACH_WIDTH = 150;
const int PerspectiveHelper::MIN_DETACH_HEIGHT = 250;

PerspectiveHelper::DragOverListener::DragOverListener(PerspectiveHelper* perspHelper) :
  perspHelper(perspHelper)
{

}

IDropTarget::Pointer PerspectiveHelper::DragOverListener::Drag(
    QWidget* /*currentControl*/, const Object::Pointer& draggedObject, const QPoint& /*position*/,
    const QRect& dragRectangle)
{

  if (draggedObject.Cast<PartPane>() != 0)
  {
    PartPane::Pointer part = draggedObject.Cast<PartPane>();
    if (part->GetContainer().Cast<PartStack>()->GetAppearance() == PresentationFactoryUtil::ROLE_EDITOR)
      return IDropTarget::Pointer(0);

    // Views that haven't been shown yet have no 'control' which causes
    // 'GetWorkbenchWindow' to return 'null' so check explicitly
    if (part->GetPage() != perspHelper->page)
      return IDropTarget::Pointer(0);
    else if (part->GetWorkbenchWindow() != perspHelper->page->GetWorkbenchWindow())
      return IDropTarget::Pointer(0);

    if (perspHelper->dropTarget == 0)
      perspHelper->dropTarget = new ActualDropTarget(perspHelper, part, dragRectangle);
    else
      perspHelper->dropTarget->SetTarget(part, dragRectangle);
  }
  else if (draggedObject.Cast<PartStack>() != 0)
  {
    PartStack::Pointer stack = draggedObject.Cast<PartStack>();
    if (stack->GetAppearance() == PresentationFactoryUtil::ROLE_EDITOR)
      return IDropTarget::Pointer(0);

    if (stack->GetWorkbenchWindow() != perspHelper->page->GetWorkbenchWindow())
      return IDropTarget::Pointer(0);

    if (perspHelper->dropTarget == 0)
      perspHelper->dropTarget = new ActualDropTarget(perspHelper, stack, dragRectangle);
    else
      perspHelper->dropTarget->SetTarget(stack, dragRectangle);
  }

  return perspHelper->dropTarget;
}

void PerspectiveHelper::ActualDropTarget::SetTarget(PartPane::Pointer part,
    const QRect& dragRectangle)
{
  this->stack = 0;
  this->part = part;
  this->dragRectangle = dragRectangle;
}

void PerspectiveHelper::ActualDropTarget::SetTarget(PartStack::Pointer stack,
    const QRect& dragRectangle)
{
  this->stack = stack;
  this->part = 0;
  this->dragRectangle = dragRectangle;
}

PerspectiveHelper::ActualDropTarget::ActualDropTarget(PerspectiveHelper* perspHelper, PartPane::Pointer part,
    const QRect& dragRectangle)
: AbstractDropTarget(), perspHelper(perspHelper)
{
  this->SetTarget(part, dragRectangle);
}

PerspectiveHelper::ActualDropTarget::ActualDropTarget(PerspectiveHelper* perspHelper, PartStack::Pointer stack,
    const QRect& dragRectangle)
: AbstractDropTarget(), perspHelper(perspHelper)
{
  this->SetTarget(stack, dragRectangle);
}

void PerspectiveHelper::ActualDropTarget::Drop()
{

  if (part != 0)
  {
  Shell::Pointer shell = part->GetShell();
  if (shell->GetData().Cast<DetachedWindow> () != 0)
  {
    // if only one view in tab folder then do a window move
    ILayoutContainer::Pointer container = part->GetContainer();
    if (container.Cast<PartStack> () != 0)
    {
      if (container.Cast<PartStack>()->GetItemCount() == 1)
      {
        shell->SetLocation(dragRectangle.x(), dragRectangle.y());
        return;
      }
    }
  }

//  // If layout is modified always zoom out.
//  if (isZoomed())
//  {
//    zoomOut();
//  }
  // do a normal part detach
  perspHelper->DetachPart(part, dragRectangle.x(), dragRectangle.y());
  }
  else if (stack != 0)
  {
    Shell::Pointer shell = stack->GetShell();
  if (shell->GetData().Cast<DetachedWindow> () != 0)
  {
    // only one tab folder in a detach window, so do window
    // move
     shell->SetLocation(dragRectangle.x(), dragRectangle.y());
      return;
  }

//  // If layout is modified always zoom out.
//  if (isZoomed())
//  {
//    zoomOut();
//  }
  // do a normal part detach
  perspHelper->Detach(stack, dragRectangle.x(), dragRectangle.y());
  }
}

CursorType PerspectiveHelper::ActualDropTarget::GetCursor()
{
  return CURSOR_OFFSCREEN;
}

PerspectiveHelper::MatchingPart::MatchingPart(const QString& pid,
    const QString& sid, LayoutPart::Pointer part)
{
  this->pid = pid;
  this->sid = sid;
  this->part = part;
  this->len = pid.size() + sid.size();
  this->hasWildcard = (pid.indexOf(PartPlaceholder::WILD_CARD) != -1) ||
                      (sid.indexOf(PartPlaceholder::WILD_CARD) != -1);
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
  return m1.len > m2.len;
}

PerspectiveHelper::PerspectiveHelper(WorkbenchPage* workbenchPage,
    ViewSashContainer::Pointer mainLayout, Perspective* persp)
: page(workbenchPage), perspective(persp),
mainLayout(mainLayout),
detachable(false), active(false)
{

  // Views can be detached if the feature is enabled (true by default,
  // use the plug-in customization file to disable), and if the platform
  // supports detaching.

  this->dragTarget.reset(new DragOverListener(this));

  //TODO preference store
  //  IPreferenceStore store = PlatformUI.getPreferenceStore();
  //  this.detachable = store.getBoolean(
  //      IWorkbenchPreferenceConstants.ENABLE_DETACHED_VIEWS);
  this->detachable = true;

  if (this->detachable)
  {
    // Check if some arbitrary Composite supports reparenting. If it
    // doesn't, views cannot be detached.

    QWidget* client = workbenchPage->GetClientComposite();
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

void PerspectiveHelper::Activate(QWidget* parent)
{

  if (active)
  {
    return;
  }

  parentWidget = parent;

  // Activate main layout
  // make sure all the views have been properly parented
  QList<PartPane::Pointer> children;
  this->CollectViewPanes(children, mainLayout->GetChildren());
  for (QList<PartPane::Pointer>::iterator iter = children.begin();
      iter != children.end(); ++iter)
  {
    PartPane::Pointer part = *iter;
    part->Reparent(parent);
  }
  mainLayout->CreateControl(parent);
  mainLayout->SetActive(true);

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

void PerspectiveHelper::AddPart(LayoutPart::Pointer part)
{

  // Look for a placeholder.
  PartPlaceholder::Pointer placeholder;
  LayoutPart::Pointer testPart;
  QString primaryId = part->GetID();
  QString secondaryId;

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
    if (relative != 0 && relative.Cast<ILayoutContainer>() != 0)
    {
      ILayoutContainer::Pointer stack =
      relative.Cast<ILayoutContainer> ();
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
    ILayoutContainer::Pointer container = placeholder->GetContainer();
    if (container != 0)
    {
      if (container.Cast<DetachedPlaceHolder> () != 0)
      {
        //Create a detached window add the part on it.
        DetachedPlaceHolder::Pointer holder = container.Cast<DetachedPlaceHolder>();
        detachedPlaceHolderList.removeAll(holder);
        container->Remove(testPart);
        DetachedWindow::Pointer window(new DetachedWindow(page));
        detachedWindowList.push_back(window);
        window->Create();
        part->CreateControl(window->GetShell()->GetControl());
        // Open window.
        window->GetShell()->SetBounds(holder->GetBounds());
        window->Open();
        // add part to detached window.
        PartPane::Pointer pane = part.Cast<PartPane>();
        window->Add(pane);
        QList<LayoutPart::Pointer> otherChildren = holder->GetChildren();
        for (QList<LayoutPart::Pointer>::iterator iter = otherChildren.begin();
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
          if (parentContainer == 0) return;

          container = containerPlaceholder->GetRealContainer().Cast<ILayoutContainer>();
          if (container.Cast<LayoutPart> () != 0)
          {
            parentContainer->Replace(containerPlaceholder,
                container.Cast<LayoutPart>());
          }
          containerPlaceholder->SetRealContainer(ILayoutContainer::Pointer(0));
        }

//        // reparent part.
//        if (container.Cast<PartStack>() == 0)
//        {
//          // We don't need to reparent children of PartTabFolders since they will automatically
//          // reparent their children when they become visible. This if statement used to be
//          // part of an else branch. Investigate if it is still necessary.
//          part->Reparent(mainLayout->GetParent());
//        }

        // see if we should replace the placeholder
        if (placeholder->HasWildCard())
        {
          if (PartSashContainer::Pointer sashContainer = container.Cast<PartSashContainer>())
          {
            sashContainer->AddChildForPlaceholder(part, placeholder);
          }
          else
          {
            container->Add(part);
          }
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

bool PerspectiveHelper::BringPartToTop(LayoutPart::Pointer part)
{
  ILayoutContainer::Pointer container = part->GetContainer();
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
  LayoutPart::Pointer foundPart;
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

  ILayoutContainer::Pointer container = foundPart->GetContainer();

  if (container.Cast<ContainerPlaceholder> () != 0)
  {
    return false;
  }

  if (container.Cast<PartStack> () != 0)
  {
    PartStack::Pointer folder = container.Cast<PartStack>();
    LayoutPart::Pointer visiblePart = folder->GetSelection();
    if (visiblePart == 0)
    {
      return false;
    }
    return partRef == visiblePart.Cast<PartPane>()->GetPartReference();
  }
  return true;
}

bool PerspectiveHelper::WillPartBeVisible(const QString& partId)
{
  return this->WillPartBeVisible(partId, 0);
}

bool PerspectiveHelper::WillPartBeVisible(const QString& partId,
    const QString& secondaryId)
{
  LayoutPart::Pointer part = this->FindPart(partId, secondaryId);
  if (part == 0)
  {
    return false;
  }
  ILayoutContainer::Pointer container = part->GetContainer();
  if (container != 0 && container.Cast<ContainerPlaceholder> () != 0)
  {
    container = container.Cast<ContainerPlaceholder>()->GetRealContainer().Cast<ILayoutContainer>();
  }

  if (container != 0 && container.Cast<PartStack> () != 0)
  {
    PartStack::Pointer folder = container.Cast<PartStack>();
    if (folder->GetSelection() == 0)
    {
      return false;
    }
    return part->GetID() == folder->GetSelection()->GetID();
  }
  return true;
}

QList<PartPlaceholder::Pointer> PerspectiveHelper::CollectPlaceholders()
{
  // Scan the main window.
  QList<PartPlaceholder::Pointer> results = this->CollectPlaceholders(
      mainLayout->GetChildren());

  // Scan each detached window.
  if (detachable)
  {
    for (DetachedWindowsType::iterator winIter = detachedWindowList.begin();
        winIter != detachedWindowList.end(); ++winIter)
    {
      DetachedWindow::Pointer win = *winIter;
      QList<LayoutPart::Pointer> moreResults = win->GetChildren();
      if (moreResults.size()> 0)
      {
        for (QList<LayoutPart::Pointer>::iterator iter = moreResults.begin();
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

QList<PartPlaceholder::Pointer> PerspectiveHelper::CollectPlaceholders(
    const QList<LayoutPart::Pointer>& parts)
{
  QList<PartPlaceholder::Pointer> result;

  for (QList<LayoutPart::Pointer>::const_iterator iter = parts.begin();
      iter != parts.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    if (ILayoutContainer::Pointer container = part.Cast<ILayoutContainer>())
    {
      // iterate through sub containers to find sub-parts
      QList<PartPlaceholder::Pointer> newParts = this->CollectPlaceholders(
            container->GetChildren());
      result.append(newParts);
    }
    else if (PartPlaceholder::Pointer placeholder = part.Cast<PartPlaceholder>())
    {
      result.push_back(placeholder);
    }
  }

  return result;
}

void PerspectiveHelper::CollectViewPanes(QList<PartPane::Pointer>& result)
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
      CollectViewPanes(result, win->GetChildren());
    }
  }
}

void PerspectiveHelper::CollectViewPanes(QList<PartPane::Pointer>& result,
    const QList<LayoutPart::Pointer>& parts)
{
  for (QList<LayoutPart::Pointer>::const_iterator iter = parts.begin();
      iter != parts.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    if (PartPane::Pointer partPane = part.Cast<PartPane>())
    {
      if(partPane->GetPartReference().Cast<IViewReference>())
      {
        result.push_back(partPane);
      }
    }
    else if (ILayoutContainer::Pointer container = part.Cast<ILayoutContainer> ())
    {
      this->CollectViewPanes(result, container->GetChildren());
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
  QWidget* parent = mainLayout->GetParent();
  QList<PartPane::Pointer> children;
  this->CollectViewPanes(children, mainLayout->GetChildren());

  for (DetachedWindowsType::iterator winIter = detachedWindowList.begin();
      winIter != detachedWindowList.end(); ++winIter)
  {
    DetachedWindow::Pointer window = *winIter;
    CollectViewPanes(children, window->GetChildren());
  }

  // *** Do we even need to do this if detached windows not supported?
  for (QList<PartPane::Pointer>::iterator itr = children.begin();
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
  mainLayout->Dispose();
  mainLayout->DisposeSashes();
}

void PerspectiveHelper::DescribeLayout(QString& buf) const
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
        QList<LayoutPart::Pointer> children = window->GetChildren();
        int j = 0;
        if (children.size() != 0)
        {
          buf.append("dWindow ("); //$NON-NLS-1$
          for (QList<LayoutPart::Pointer>::iterator partIter = children.begin();
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

void PerspectiveHelper::DerefPart(LayoutPart::Pointer part)
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
  //        QList<LayoutPart::Pointer> kids = vs.getChildren();
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
  ILayoutContainer::Pointer oldContainer = part->GetContainer();
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

  ILayoutContainer::ChildrenType children = oldContainer->GetChildren();
  if (wasDocked)
  {
    bool hasChildren = (children.size()> 0);
    if (hasChildren)
    {
      // make sure one is at least visible
      int childVisible = 0;
      for (ILayoutContainer::ChildrenType::iterator iter = children.begin();
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
          hasChildren = folder->GetChildren().size()> 0;

          // We maintain the stack as a place-holder if it has children
          // (which at this point would represent view place-holders)
          if (hasChildren)
          {
            folder->Dispose();

            // replace the real container with a ContainerPlaceholder
            ContainerPlaceholder::Pointer placeholder(
            new ContainerPlaceholder(folder->GetID()));
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
        //BERRY_INFO << "No children left, removing container\n";

        LayoutPart::Pointer parent = oldContainer.Cast<LayoutPart>();
        ILayoutContainer::Pointer parentContainer = parent->GetContainer();
        if (parentContainer != 0)
        {
          parentContainer->Remove(parent);
          parent->Print(qDebug());
          parent->Dispose();
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
      detachedWindowList.removeAll(w);
    }
    else
    {
      // There are children. If none are visible hide detached
      // window.
      bool allInvisible = true;
      for (ILayoutContainer::ChildrenType::iterator iter = children.begin();
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
        DetachedPlaceHolder::Pointer placeholder(new DetachedPlaceHolder("",
            oldShell->GetBounds()));
        for (ILayoutContainer::ChildrenType::iterator iter = children.begin();
            iter != children.end(); ++iter)
        {
          oldContainer->Remove(*iter);
          (*iter)->SetContainer(placeholder);
          placeholder->Add(*iter);
        }
        detachedPlaceHolderList.push_back(placeholder);
        DetachedWindow::Pointer w = oldShell->GetData().Cast<DetachedWindow>();
        oldShell->Close();
        detachedWindowList.removeAll(w);
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
  QSize size = part->GetSize();
  if (size.width() == 0 || size.height() == 0)
  {
    ILayoutContainer::Pointer container = part->GetContainer();
    if (container.Cast<LayoutPart> () != 0)
    {
      size = container.Cast<LayoutPart>()->GetSize();
    }
  }
  int width = std::max<int>(size.width(), MIN_DETACH_WIDTH);
  int height = std::max<int>(size.height(), MIN_DETACH_HEIGHT);

  // Create detached window.
  DetachedWindow::Pointer window(new DetachedWindow(page));
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
    LayoutPart::Pointer visiblePart = stack->GetSelection();
    ILayoutContainer::ChildrenType children = stack->GetChildren();
    for (ILayoutContainer::ChildrenType::iterator iter = children.begin();
        iter != children.end(); ++iter)
    {
      if (PartPane::Pointer partPane = iter->Cast<PartPane>()
          // && check it is a view?
          )
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

void PerspectiveHelper::DetachPart(LayoutPart::Pointer part, int x, int y)
{

  // Detaching is disabled on some platforms ..
  if (!detachable)
  {
    return;
  }

  // Calculate detached window size.
  QSize size = part->GetSize();
  if (size.width() == 0 || size.height() == 0)
  {
    ILayoutContainer::Pointer container = part->GetContainer();
    if (container.Cast<LayoutPart> () != 0)
    {
      size = container.Cast<LayoutPart>()->GetSize();
    }
  }
  int width = std::max<int>(size.width(), MIN_DETACH_WIDTH);
  int height = std::max<int>(size.height(), MIN_DETACH_HEIGHT);

  // Create detached window.
  DetachedWindow::Pointer window(new DetachedWindow(page));
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

    QRect bounds = pane->GetParentBounds();
    this->DetachPart(pane, bounds.x(), bounds.y());
  }
}

void PerspectiveHelper::AddDetachedPart(LayoutPart::Pointer part)
{
  // Calculate detached window size.
  QRect bounds = Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetShell(parentWidget)->GetBounds();
  bounds.setX(bounds.x() + (bounds.width() - 300) / 2);
  bounds.setY(bounds.y() + (bounds.height() - 300) / 2);

  this->AddDetachedPart(part, bounds);
}

void PerspectiveHelper::AddDetachedPart(LayoutPart::Pointer part,
    const QRect& bounds)
{
  // Detaching is disabled on some platforms ..
  if (!detachable)
  {
    this->AddPart(part);
    return;
  }

  // Create detached window.
  DetachedWindow::Pointer window(new DetachedWindow(page));
  detachedWindowList.push_back(window);
  window->Create();

  // add part to detached window.
  part->CreateControl(window->GetShell()->GetControl());
  window->Add(part);

  // Open window.
  window->GetShell()->SetBounds(bounds.x(), bounds.y(), bounds.width(), bounds.height());
  window->Open();

  part->SetFocus();

}

void PerspectiveHelper::DisableAllDrag()
{
  DragUtil::RemoveDragTarget(0, dragTarget.data());
}

void PerspectiveHelper::EnableAllDrag()
{
  DragUtil::AddDragTarget(0, dragTarget.data());
}

LayoutPart::Pointer PerspectiveHelper::FindPart(const QString& id)
{
  return this->FindPart(id, "");
}

LayoutPart::Pointer PerspectiveHelper::FindPart(const QString& primaryId,
    const QString& secondaryId)
{

  //BERRY_INFO << "Looking for part: " << primaryId << ":" << secondaryId << std::endl;

  // check main window.
  QList<MatchingPart> matchingParts;
  LayoutPart::Pointer part = (secondaryId != "") ? this->FindPart(primaryId, secondaryId,
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

  //BERRY_INFO << "Looking through the matched parts (count: " << matchingParts.size() << ")\n";

  // sort the matching parts
  if (matchingParts.size()> 0)
  {
    std::partial_sort(matchingParts.begin(), (matchingParts.begin()), matchingParts.end(), CompareMatchingParts());
    const MatchingPart& mostSignificantPart = matchingParts.front();
    return mostSignificantPart.part;
  }

  // Not found.
  return LayoutPart::Pointer(0);
}

LayoutPart::Pointer PerspectiveHelper::FindPart(const QString& id,
    const QList<LayoutPart::Pointer>& parts,
    QList<MatchingPart>& matchingParts)
{
  for (QList<LayoutPart::Pointer>::const_iterator iter = parts.begin();
      iter != parts.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    // check for part equality, parts with secondary ids fail
    if (part->GetID() == id)
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

    else if (part->IsPlaceHolder()
             && part.Cast<PartPlaceholder>()->HasWildCard())
    {
      QRegExp re(id, Qt::CaseInsensitive);
      if (re.exactMatch(part->GetID()))
      {
        matchingParts.push_back(MatchingPart(part->GetID(), "", part));
      }
      //      StringMatcher sm = new StringMatcher(part.getID(), true, false);
      //      if (sm.match(id))
      //      {
      //        matchingParts .add(new MatchingPart(part.getID(), 0, part));
      //      }
    }
    else if (ILayoutContainer::Pointer layoutContainer = part.Cast<ILayoutContainer>())
    {
      part = FindPart(id, layoutContainer->GetChildren(),
                      matchingParts);
      if (part)
      {
        return part;
      }
    }
  }

  //BERRY_INFO << "Returning 0\n";
  return LayoutPart::Pointer(0);
}

LayoutPart::Pointer PerspectiveHelper::FindPart(const QString& primaryId,
    const QString& secondaryId,
    const QList<LayoutPart::Pointer>& parts,
    QList<MatchingPart>& matchingParts)
{
  for (QList<LayoutPart::Pointer>::const_iterator iter = parts.begin();
      iter != parts.end(); ++iter)
  {
    LayoutPart::Pointer part = *iter;
    // check containers first
    if (ILayoutContainer::Pointer layoutContainer = part.Cast<ILayoutContainer>())
    {
      LayoutPart::Pointer testPart = FindPart(primaryId, secondaryId,
                                              layoutContainer->GetChildren(), matchingParts);
      if (testPart)
      {
        return testPart;
      }
    }
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
      QString id = part->GetID();

      // optimization: don't bother parsing id if it has no separator -- it can't match
      QString phSecondaryId = ViewFactory::ExtractSecondaryId(id);
      if (phSecondaryId == "")
      {
        // but still need to check for wildcard case
        if (id == PartPlaceholder::WILD_CARD)
        {
          matchingParts.push_back(MatchingPart(id, "", part));
        }
        continue;
      }

      QString phPrimaryId = ViewFactory::ExtractPrimaryId(id);
      // perfect matching pair
      if (phPrimaryId == primaryId && phSecondaryId == secondaryId)
      {
        return part;
      }
      // check for partial matching pair
      QRegExp pre(phPrimaryId, Qt::CaseInsensitive);
      if (pre.exactMatch(primaryId))
      {
        QRegExp sre(phSecondaryId, Qt::CaseInsensitive);
        if (sre.exactMatch(secondaryId))
        {
          matchingParts.push_back(MatchingPart(phPrimaryId, phSecondaryId, part));
        }
      }
    }
  }
  return LayoutPart::Pointer(0);
}

bool PerspectiveHelper::HasPlaceholder(const QString& id)
{
  return this->HasPlaceholder(id, 0);
}

bool PerspectiveHelper::HasPlaceholder(const QString& primaryId,
    const QString& secondaryId)
{
  LayoutPart::Pointer testPart;
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

float PerspectiveHelper::GetDockingRatio(LayoutPart::Pointer source,
    LayoutPart::Pointer target)
{
  if ((source.Cast<PartPane> () != 0 || source.Cast<PartStack> () != 0)
      && target.Cast<EditorSashContainer> () != 0)
  {
    return 0.25f;
  }
  return 0.5f;
}

void PerspectiveHelper::RemovePart(LayoutPart::Pointer part)
{

  // Reparent the part back to the main window
  QWidget* parent = mainLayout->GetParent();
  part->Reparent(parent);

  // Replace part with a placeholder
  ILayoutContainer::Pointer container = part->GetContainer();
  if (container != 0)
  {
    QString placeHolderId = part->GetPlaceHolderId();
    container->Replace(part, LayoutPart::Pointer(new PartPlaceholder(placeHolderId)));

//    // If the parent is root we're done. Do not try to replace
//    // it with placeholder.
//    if (container == mainLayout)
//    {
//      return;
//    }

    // If the parent is empty replace it with a placeholder.
    QList<LayoutPart::Pointer> children = container->GetChildren();

    bool allInvisible = true;
    for (QList<LayoutPart::Pointer>::iterator childIter = children.begin();
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

        // PR 1GDFVBY: ViewStack not disposed when page
        // closed.
        if (container.Cast<PartStack> () != 0)
        {
          container.Cast<PartStack>()->Dispose();
        }

        // replace the real container with a
        // ContainerPlaceholder
        ILayoutContainer::Pointer parentContainer = cPart->GetContainer();
        ContainerPlaceholder::Pointer placeholder(
        new ContainerPlaceholder(cPart->GetID()));
        placeholder->SetRealContainer(container);
        parentContainer->Replace(cPart, placeholder);

      }
      else
      {
        DetachedPlaceHolder::Pointer placeholder(
        new DetachedPlaceHolder("", oldShell->GetBounds())); //$NON-NLS-1$
        for (QList<LayoutPart::Pointer>::iterator childIter2 = children.begin();
            childIter2 != children.end(); ++childIter2)
        {
          (*childIter2)->GetContainer()->Remove(*childIter2);
          (*childIter2)->SetContainer(placeholder);
          placeholder->Add(*childIter2);
        }
        detachedPlaceHolderList.push_back(placeholder);
        DetachedWindow::Pointer w = oldShell->GetData().Cast<DetachedWindow>();
        oldShell->Close();
        detachedWindowList.removeAll(w);
      }

    }
  }
}

void PerspectiveHelper::ReplacePlaceholderWithPart(LayoutPart::Pointer part)
{

  // Look for a PartPlaceholder that will tell us how to position this
  // object
  QList<PartPlaceholder::Pointer> placeholders = this->CollectPlaceholders();
  for (int i = 0; i < placeholders.size(); i++)
  {
    if (placeholders[i]->GetID() == part->GetID())
    {
      // found a matching placeholder which we can replace with the
      // new View
      ILayoutContainer::Pointer container = placeholders[i]->GetContainer();
      if (container != 0)
      {
        if (ContainerPlaceholder::Pointer containerPlaceholder = container.Cast<ContainerPlaceholder> ())
        {
          // One of the children is now visible so replace the
          // ContainerPlaceholder with the real container
          ILayoutContainer::Pointer parentContainer =
              containerPlaceholder->GetContainer();
          container = containerPlaceholder->GetRealContainer().Cast<ILayoutContainer>();
          if (LayoutPart::Pointer layoutPart = container.Cast<LayoutPart> ())
          {
            parentContainer->Replace(containerPlaceholder, layoutPart);
          }
          containerPlaceholder->SetRealContainer(ILayoutContainer::Pointer(0));
        }
        container->Replace(placeholders[i], part);
        return;
      }
    }
  }
}

bool PerspectiveHelper::RestoreState(IMemento::Pointer memento)
{
  // Restore main window.
  IMemento::Pointer childMem = memento->GetChild(WorkbenchConstants::TAG_MAIN_WINDOW);
  //IStatus r = mainLayout->RestoreState(childMem);
  bool r = mainLayout->RestoreState(childMem);

  // Restore each floating window.
  if (detachable)
  {
    QList<IMemento::Pointer> detachedWindows(memento->GetChildren(
        WorkbenchConstants::TAG_DETACHED_WINDOW));
    for (QList<IMemento::Pointer>::iterator iter = detachedWindows.begin();
        iter != detachedWindows.end(); ++iter)
    {
      DetachedWindow::Pointer win(new DetachedWindow(page));
      detachedWindowList.push_back(win);
      win->RestoreState(*iter);
    }

    QList<IMemento::Pointer> childrenMem(memento->GetChildren(
        WorkbenchConstants::TAG_HIDDEN_WINDOW));
    for (QList<IMemento::Pointer>::iterator iter = childrenMem.begin();
        iter != childrenMem.end(); ++iter)
    {
      DetachedPlaceHolder::Pointer holder(
          new DetachedPlaceHolder("", QRect(0, 0, 0, 0)));
      holder->RestoreState(*iter);
      detachedPlaceHolderList.push_back(holder);
    }
  }

  // Get the cached id of the currently maximized stack
  //maximizedStackId = childMem.getString(IWorkbenchConstants.TAG_MAXIMIZED);

  return r;
}

bool PerspectiveHelper::SaveState(IMemento::Pointer memento)
{
  // Persist main window.
  IMemento::Pointer childMem = memento->CreateChild(WorkbenchConstants::TAG_MAIN_WINDOW);
  //IStatus r = mainLayout->SaveState(childMem);
  bool r = mainLayout->SaveState(childMem);

  if (detachable)
  {
    // Persist each detached window.
    for (DetachedWindowsType::iterator iter = detachedWindowList.begin();
        iter != detachedWindowList.end(); ++iter)
    {
      childMem = memento->CreateChild(WorkbenchConstants::TAG_DETACHED_WINDOW);
      (*iter)->SaveState(childMem);
    }
    for (DetachedPlaceHoldersType::iterator iter = detachedPlaceHolderList.begin();
        iter != detachedPlaceHolderList.end(); ++iter)
    {
      childMem = memento->CreateChild(WorkbenchConstants::TAG_HIDDEN_WINDOW);
      (*iter)->SaveState(childMem);
    }
  }

  // Write out the id of the maximized (View) stack (if any)
  // NOTE: we only write this out if it's a ViewStack since the
  // Editor Area is handled by the perspective
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

  return r;
}

void PerspectiveHelper::UpdateBoundsMap()
{
  boundsMap.clear();

  // Walk the layout gathering the current bounds of each stack
  // and the editor area
  QList<LayoutPart::Pointer> kids = mainLayout->GetChildren();
  for (QList<LayoutPart::Pointer>::iterator iter = kids.begin();
      iter != kids.end(); ++iter)
  {
    if (iter->Cast<PartStack> () != 0)
    {
      PartStack::Pointer vs = iter->Cast<PartStack>();
      boundsMap.insert(vs->GetID(), vs->GetBounds());
    }
    else if (iter->Cast<EditorSashContainer> () != 0)
    {
      EditorSashContainer::Pointer esc = iter->Cast<EditorSashContainer>();
      boundsMap.insert(esc->GetID(), esc->GetBounds());
    }
  }
}

void PerspectiveHelper::ResetBoundsMap()
{
  boundsMap.clear();
}

QRect PerspectiveHelper::GetCachedBoundsFor(const QString& id)
{
  return boundsMap[id];
}

}
