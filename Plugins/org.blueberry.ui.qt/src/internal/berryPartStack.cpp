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

#include "berryPartStack.h"

#include "berryPerspective.h"
#include "berryPresentationFactoryUtil.h"
#include "berryWorkbenchPlugin.h"
#include "berryPresentationSerializer.h"
#include "berryDragUtil.h"
#include "berryEditorAreaHelper.h"
#include "berryPerspectiveHelper.h"
#include "berryWorkbenchConstants.h"

#include "berryXMLMemento.h"

#include "berryIWorkbenchPartConstants.h"
#include "berryGeometry.h"

#include <berryObjects.h>


namespace berry
{

const int PartStack::PROP_SELECTION = 0x42;
PartStack::PartStackDropResult::Pointer PartStack::dropResult(
    new PartStack::PartStackDropResult());

void PartStack::PartStackDropResult::SetTarget(PartStack::Pointer stack,
    PartPane::Pointer pane, StackDropResult::Pointer result)
{
  this->pane = pane;
  this->dropResult = result;
  this->stack = stack;
}

void PartStack::PartStackDropResult::Drop()
{
  // If we're dragging a pane over itself do nothing
  //if (dropResult.getInsertionPoint() == pane.getPresentablePart()) { return; };

  Object::Pointer cookie;
  if (dropResult != 0)
  {
    cookie = dropResult->GetCookie();
  }

  PartPane::Pointer pane(this->pane);
  PartStack::Pointer stack(this->stack);
  // Handle cross window drops by opening a new editor
  if (pane->GetPartReference().Cast<IEditorReference> () != 0)
  {
    IEditorReference::Pointer editorRef = pane->GetPartReference().Cast<
        IEditorReference> ();
    if (pane->GetWorkbenchWindow() != stack->GetWorkbenchWindow())
    {
      try
      {
        IEditorInput::Pointer input = editorRef->GetEditorInput();

        // Close the old editor and capture the actual closed state incase of a 'cancel'
        bool editorClosed = pane->GetPage()->CloseEditor(editorRef, true);

        // Only open open the new editor if the old one closed
        if (editorClosed)
          stack->GetPage()->OpenEditor(input, editorRef->GetId());
        return;
      }
      catch (const PartInitException& e)
      {
        //e.printStackTrace();
        BERRY_ERROR << e.what();
      }

    }
  }

  if (pane->GetContainer() != stack)
  {
    // Moving from another stack
    stack->DerefPart(pane);
    pane->Reparent(stack->GetParent());
    stack->Add(pane, cookie);
    stack->SetSelection(pane);
    pane->SetFocus();
  }
  else if (cookie != 0)
  {
    // Rearranging within this stack
    stack->GetPresentation()->MovePart(stack->GetPresentablePart(pane), cookie);
  }
}

CursorType PartStack::PartStackDropResult::GetCursor()
{
  return CURSOR_CENTER;
}

QRect PartStack::PartStackDropResult::GetSnapRectangle()
{
  if (dropResult == 0)
  {
    return DragUtil::GetDisplayBounds(stack.Lock()->GetControl());
  }
  return dropResult->GetSnapRectangle();
}

PartStack::MyStackPresentationSite::MyStackPresentationSite(PartStack* stack) :
  partStack(stack)
{

}

void PartStack::MyStackPresentationSite::Close(IPresentablePart::Pointer part)
{
  partStack->Close(part);
}

void PartStack::MyStackPresentationSite::Close(const QList<
    IPresentablePart::Pointer>& parts)
{
  partStack->Close(parts);
}

void PartStack::MyStackPresentationSite::DragStart(
    IPresentablePart::Pointer beingDragged, QPoint& initialLocation,
    bool keyboard)
{
  partStack->DragStart(beingDragged, initialLocation, keyboard);
}

void PartStack::MyStackPresentationSite::DragStart(QPoint& initialLocation,
    bool keyboard)
{
  partStack->DragStart(IPresentablePart::Pointer(nullptr), initialLocation, keyboard);
}

bool PartStack::MyStackPresentationSite::IsPartMoveable(
    IPresentablePart::Pointer part)
{
  return partStack->IsMoveable(part);
}

void PartStack::MyStackPresentationSite::SelectPart(
    IPresentablePart::Pointer toSelect)
{
  partStack->PresentationSelectionChanged(toSelect);
}

bool PartStack::MyStackPresentationSite::SupportsState(int state)
{
  return partStack->SupportsState(state);
}

void PartStack::MyStackPresentationSite::SetState(int newState)
{
  partStack->SetState(newState);
}

IPresentablePart::Pointer PartStack::MyStackPresentationSite::GetSelectedPart()
{
  return partStack->GetSelectedPart();
}

//         void AddSystemActions(IMenuManager menuManager) {
//            PartStack.this.addSystemActions(menuManager);
//        }

bool PartStack::MyStackPresentationSite::IsStackMoveable()
{
  return partStack->CanMoveFolder();
}

void PartStack::MyStackPresentationSite::FlushLayout()
{
  partStack->FlushLayout();
}

PartStack::PresentableVector PartStack::MyStackPresentationSite::GetPartList()
{
  return partStack->GetPresentableParts();
}

QString PartStack::MyStackPresentationSite::GetProperty(
    const QString& id)
{
  return partStack->GetProperty(id);
}

PartStack::PartStack(WorkbenchPage* p, bool allowsStateChanges,
    int appear, IPresentationFactory* fac) :
  LayoutPart("PartStack"), page(p), isActive(true), allowStateChanges(
      allowsStateChanges), appearance(appear), ignoreSelectionChanges(false),
      factory(fac)
{
  QString str;
  QTextStream buf(&str);
  buf << "PartStack@" << this;
  this->SetID(str);

  presentationSite = new MyStackPresentationSite(this);
}

bool PartStack::IsMoveable(IPresentablePart::Pointer part)
{
  PartPane::Pointer pane = this->GetPaneFor(part);
  Perspective::Pointer perspective = this->GetPage()->GetActivePerspective();
  if (perspective == 0)
  {
    // Shouldn't happen -- can't have a ViewStack without a
    // perspective
    return true;
  }
  IWorkbenchPartReference::Pointer partRef = pane->GetPartReference();
  if (partRef.Cast<IViewReference> () != 0)
    return perspective->IsMoveable(partRef.Cast<IViewReference> ());

  return true;
}

bool PartStack::SupportsState(int  /*newState*/)
{
  if (page->IsFixedLayout())
  {
    return false;
  }

  return allowStateChanges;
}

bool PartStack::CanMoveFolder()
{
  if (appearance == PresentationFactoryUtil::ROLE_EDITOR)
    return true;

  Perspective::Pointer perspective = this->GetPage()->GetActivePerspective();

  if (perspective == 0)
  {
    // Shouldn't happen -- can't have a ViewStack without a
    // perspective
    return false;
  }

  // We need to search if one of the presentations is not moveable
  // if that's the case the whole folder should not be moveable
  IStackPresentationSite::Pointer presenationSite;

  if ((presenationSite = this->GetPresentationSite()) != 0)
  {
    QList<IPresentablePart::Pointer> parts = presenationSite->GetPartList();
    for (QList<IPresentablePart::Pointer>::iterator iter = parts.begin(); iter
        != parts.end(); ++iter)
    {
      if (!presenationSite->IsPartMoveable(*iter))
      {
        return false;
      }
    }
  }

  return !perspective->IsFixedLayout();
}

void PartStack::DerefPart(LayoutPart::Pointer toDeref)
{
  if (appearance == PresentationFactoryUtil::ROLE_EDITOR)
    EditorAreaHelper::DerefPart(toDeref);
  else
    this->GetPage()->GetActivePerspective()->GetPresentation()->DerefPart(
        toDeref);
}

bool PartStack::AllowsDrop(PartPane::Pointer part)
{
  PartStack::Pointer stack = part->GetContainer().Cast<PartStack> ();
  if (stack != 0)
  {
    if (stack->appearance == this->appearance)
      return true;
  }

  return false;
}

void PartStack::AddListener(IPropertyChangeListener *listener)
{
  propEvents.AddListener(listener);
}

void PartStack::RemoveListener(IPropertyChangeListener *listener)
{
  propEvents.RemoveListener(listener);
}

int PartStack::GetAppearance() const
{
  return appearance;
}

QString PartStack::GetID() const
{
  return LayoutPart::GetID();
}

bool PartStack::IsStandalone()
{
  return (appearance == PresentationFactoryUtil::ROLE_STANDALONE || appearance
      == PresentationFactoryUtil::ROLE_STANDALONE_NOTITLE);
}

IPresentablePart::Pointer PartStack::GetSelectedPart()
{
  return presentationCurrent.Cast<IPresentablePart> ();
}

IStackPresentationSite::Pointer PartStack::GetPresentationSite()
{
  return presentationSite;
}

void PartStack::TestInvariants()
{
  QWidget* focusControl =
      Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetFocusControl();

  bool currentFound = false;

  ChildVector children = this->GetChildren();

  for (ChildVector::iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    LayoutPart::Pointer child = *iter;

    // No 0 children allowed
    poco_assert(child != 0)
;   // "0 children are not allowed in PartStack"

    // Ensure that all the PartPanes have an associated presentable part
    IPresentablePart::Pointer part = this->GetPresentablePart(child);
    if (!child->IsPlaceHolder())
    {
      poco_assert(part != 0); // "All PartPanes must have a non-0 IPresentablePart"
    }

    // Ensure that the child's backpointer points to this stack
    ILayoutContainer::Pointer childContainer = child->GetContainer();

    // Disable tests for placeholders -- PartPlaceholder backpointers don't
    // obey the usual rules -- they sometimes point to a container placeholder
    // for this stack instead of the real stack.
    if (!child->IsPlaceHolder())
    {

      // If the widgetry exists, the child's backpointer must point to us
      poco_assert(childContainer.GetPointer() == this); // "PartStack has a child that thinks it has a different parent"

      // If this child has focus, then ensure that it is selected and that we have
      // the active appearance.

      if (focusControl && Tweaklets::Get(GuiWidgetsTweaklet::KEY)->IsChild(child->GetControl(), focusControl))
      {
        poco_assert(child == current); // "The part with focus is not the selected part"
        //  focus check commented out since it fails when focus workaround in LayoutPart.setVisible is not present
        //              Assert.isTrue(getActive() == StackPresentation.AS_ACTIVE_FOCUS);
      }

    }

    // Ensure that "current" points to a valid child
    if (child == current)
    {
      currentFound = true;
    }

    // Test the child's internal state
    child->TestInvariants();
  }

  // If we have at least one child, ensure that the "current" pointer points to one of them
  if (this->GetPresentableParts().size()> 0)
  {
    poco_assert(currentFound);

    StackPresentation::Pointer presentation = this->GetPresentation();

    // If the presentation controls have focus, ensure that we have the active appearance
    if (focusControl && Tweaklets::Get(GuiWidgetsTweaklet::KEY)->IsChild(presentation->GetControl(), focusControl))
    {
      poco_assert(this->GetActive() == StackPresentation::AS_ACTIVE_FOCUS);
      // "The presentation has focus but does not have the active appearance"
    }

  }

  // Check to that we're displaying the zoomed icon iff we're actually maximized
  //poco_assert((this->GetState() == IStackPresentationSite::STATE_MAXIMIZED)
  //        == (this->GetContainer() != 0 && this->GetContainer()->ChildIsZoomed(this)));

}

void PartStack::DescribeLayout(QString& buf) const
{
  int activeState = this->GetActive();
  if (activeState == StackPresentation::AS_ACTIVE_FOCUS)
  {
    buf.append("active "); //$NON-NLS-1$
  }
  else if (activeState == StackPresentation::AS_ACTIVE_NOFOCUS)
  {
    buf.append("active_nofocus "); //$NON-NLS-1$
  }

  buf.append("("); //$NON-NLS-1$

  ChildVector children = this->GetChildren();

  int visibleChildren = 0;

  for (ChildVector::iterator iter = children.begin(); iter != children.end(); ++iter)
  {

    LayoutPart::Pointer next = *iter;
    if (!next->IsPlaceHolder())
    {
      if (iter != children.begin())
      {
        buf.append(", "); //$NON-NLS-1$
      }

      if (next == requestedCurrent)
      {
        buf.append("*"); //$NON-NLS-1$
      }

      next->DescribeLayout(buf);

      visibleChildren++;
    }
  }

  buf.append(")"); //$NON-NLS-1$
}

void PartStack::Add(LayoutPart::Pointer child)
{
  this->Add(child, Object::Pointer(nullptr));
}

void PartStack::Add(LayoutPart::Pointer newChild, Object::Pointer cookie)
{
  children.push_back(newChild);

  // Fix for bug 78470:
  if(newChild->GetContainer().Cast<ContainerPlaceholder>() == 0)
  {
    newChild->SetContainer(ILayoutContainer::Pointer(this));
  }

  this->ShowPart(newChild, cookie);
}

bool PartStack::AllowsAdd(LayoutPart::Pointer  /*toAdd*/)
{
  return !this->IsStandalone();
}

bool PartStack::AllowsAutoFocus()
{
  if (presentationSite->GetState() == IStackPresentationSite::STATE_MINIMIZED)
  {
    return false;
  }

  return LayoutPart::AllowsAutoFocus();
}

void PartStack::Close(const QList<IPresentablePart::Pointer>& parts)
{
  for (int idx = 0; idx < parts.size(); idx++)
  {
    IPresentablePart::Pointer part = parts[idx];

    this->Close(part);
  }
}

void PartStack::Close(IPresentablePart::Pointer part)
{
  if (!presentationSite->IsCloseable(part))
  {
    return;
  }

  PartPane::Pointer pane = this->GetPaneFor(part);

  if (pane != 0)
  {
    pane->DoHide();
  }
}

IPresentationFactory* PartStack::GetFactory()
{

  if (factory != nullptr)
  {
    return factory;
  }

  return WorkbenchPlugin::GetDefault()->GetPresentationFactory();
}

void PartStack::CreateControl(QWidget* parent)
{
  if (this->GetPresentation() != 0)
  {
    return;
  }

  IPresentationFactory* factory = this->GetFactory();

  PresentableVector partList = this->GetPresentableParts();
  PresentationSerializer serializer(partList);

  StackPresentation::Pointer presentation = PresentationFactoryUtil
  ::CreatePresentation(factory, appearance, parent,
      presentationSite, &serializer, savedPresentationState);

  this->CreateControl(parent, presentation);
  Tweaklets::Get(GuiWidgetsTweaklet::KEY)->MoveBelow(this->GetControl(), nullptr);
}

IDropTarget::Pointer PartStack::GetDropTarget(Object::Pointer draggedObject, const QPoint& position)
{

  if (draggedObject.Cast<PartPane>() == 0)
  {
    return IDropTarget::Pointer(nullptr);
  }

  PartPane::Pointer pane = draggedObject.Cast<PartPane>();
  if (this->IsStandalone()
      || !this->AllowsDrop(pane))
  {
    return IDropTarget::Pointer(nullptr);
  }

  // Don't allow views to be dragged between windows
  bool differentWindows = pane->GetWorkbenchWindow() != this->GetWorkbenchWindow();
  bool editorDropOK = ((pane->GetPartReference().Cast<IEditorReference>() != 0) &&
      pane->GetWorkbenchWindow()->GetWorkbench() ==
      this->GetWorkbenchWindow()->GetWorkbench());
  if (differentWindows && !editorDropOK)
  {
    return IDropTarget::Pointer(nullptr);
  }

  StackDropResult::Pointer dropResult = this->GetPresentation()->DragOver(
      this->GetControl(), position);

  if (dropResult == 0)
  {
    return IDropTarget::Pointer(nullptr);
  }

  return this->CreateDropTarget(pane, dropResult);
}

void PartStack::SetBounds(const QRect& r)
{
  if (this->GetPresentation() != 0)
  {
    this->GetPresentation()->SetBounds(r);
  }
}

IDropTarget::Pointer PartStack::CreateDropTarget(PartPane::Pointer pane, StackDropResult::Pointer result)
{
  dropResult->SetTarget(PartStack::Pointer(this), pane, result);
  return dropResult;
}

void PartStack::SetActive(bool isActive)
{

  this->isActive = isActive;
  // Add all visible children to the presentation
  for(ChildVector::iterator iter = children.begin(); iter != children.end(); ++iter)
  {
    (*iter)->SetContainer(isActive ? ILayoutContainer::Pointer(this) : ILayoutContainer::Pointer(nullptr));
  }

  for (PresentableVector::iterator iter = presentableParts.begin();
      iter != presentableParts.end(); ++iter)
  {
    PresentablePart::Pointer next = iter->Cast<PresentablePart>();

    next->EnableInputs(isActive);
    next->EnableOutputs(isActive);
  }
}

void PartStack::CreateControl(QWidget*  /*parent*/, StackPresentation::Pointer presentation)
{

  poco_assert(this->GetPresentation() == 0);

  if (presentationSite->GetPresentation() != 0)
  {
    return;
  }

  presentationSite->SetPresentation(presentation);

  // Add all visible children to the presentation
  // Use a copy of the current set of children to avoid a ConcurrentModificationException
  // if a part is added to the same stack while iterating over the children (bug 78470)
  ChildVector childParts(children);
  for (ChildVector::iterator iter = childParts.begin(); iter != childParts.end(); ++iter)
  {
    this->ShowPart(*iter, Object::Pointer(nullptr));
  }

  if (savedPresentationState != 0)
  {
    PresentableVector partList = this->GetPresentableParts();
    PresentationSerializer serializer(partList);
    presentation->RestoreState(&serializer, savedPresentationState);
  }

  //QWidget* ctrl = this->GetPresentation()->GetControl();

  //TODO control setData ?
  //ctrl.setData(this);

  // We should not have a placeholder selected once we've created the widgetry
  if (requestedCurrent != 0 && requestedCurrent->IsPlaceHolder())
  {
    requestedCurrent = nullptr;
    this->UpdateContainerVisibleTab();
  }

  this->RefreshPresentationSelection();
}

void PartStack::SavePresentationState()
{
  if (this->GetPresentation() == 0)
  {
    return;
  }

  {// Save the presentation's state before disposing it
    XMLMemento::Pointer memento = XMLMemento
    ::CreateWriteRoot(WorkbenchConstants::TAG_PRESENTATION);
    memento->PutString(WorkbenchConstants::TAG_ID, this->GetFactory()->GetId());

    QList<IPresentablePart::Pointer> parts(this->GetPresentableParts());
    PresentationSerializer serializer(parts);

    this->GetPresentation()->SaveState(&serializer, memento);

    // Store the memento in savedPresentationState
    savedPresentationState = memento;
  }
}

PartStack::~PartStack()
{
  //BERRY_INFO << "DELETING PARTSTACK";
}

void PartStack::Dispose()
{
  if (this->GetPresentation() == 0)
  {
    return;
  }

  this->SavePresentationState();

  //  for (PresentableVector::iterator iter = presentableParts.begin();
  //       iter != presentableParts.end(); ++iter)
  //  {
  //    iter->Cast<PresentablePart>()->Dispose();
  //  }
  presentableParts.clear();

  presentationCurrent = nullptr;
  current = nullptr;

  this->FireInternalPropertyChange(PROP_SELECTION);
}

void PartStack::FindSashes(LayoutPart::Pointer /*toFind*/, PartPane::Sashes& sashes)
{
  ILayoutContainer::Pointer container = this->GetContainer();

  if (container != 0)
  {
    container->FindSashes(LayoutPart::Pointer(this), sashes);
  }
}

QRect PartStack::GetBounds()
{
  if (this->GetPresentation() == 0)
  {
    return QRect(0, 0, 0, 0);
  }
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->GetPresentation()->GetControl());
}

QList<LayoutPart::Pointer> PartStack::GetChildren() const
{
  return children;
}

QWidget* PartStack::GetControl()
{
  StackPresentation::Pointer presentation = this->GetPresentation();

  if (presentation == 0)
  {
    return nullptr;
  }

  return presentation->GetControl();
}

/**
 * Answer the number of children.
 */
PartStack::ChildVector::size_type PartStack::GetItemCount()
{
  if (this->GetPresentation() == 0)
  {
    return children.size();
  }
  return this->GetPresentableParts().size();
}

PartPane::Pointer PartStack::GetPaneFor(IPresentablePart::Pointer part)
{
  if (part == 0 || part.Cast<PresentablePart>() == 0)
  {
    return PartPane::Pointer(nullptr);
  }

  return part.Cast<PresentablePart>()->GetPane();
}

QWidget* PartStack::GetParent()
{
  return Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetParent(this->GetControl());
}

PartStack::PresentableVector PartStack::GetPresentableParts()
{
  return presentableParts;
}

PresentablePart::Pointer PartStack::GetPresentablePart(LayoutPart::Pointer pane)
{
  for (PresentableVector::iterator iter = presentableParts.begin(); iter != presentableParts.end(); ++iter)
  {
    PresentablePart::Pointer part = iter->Cast<PresentablePart>();

    if (part->GetPane() == pane)
    {
      return part;
    }
  }

  return PresentablePart::Pointer(nullptr);
}

StackPresentation::Pointer PartStack::GetPresentation()
{
  return presentationSite->GetPresentation();
}

PartPane::Pointer PartStack::GetSelection()
{
  if (PartPane::Pointer partPane = current.Cast<PartPane>())
  {
    return partPane;
  }
  return PartPane::Pointer(nullptr);
}

void PartStack::PresentationSelectionChanged(IPresentablePart::Pointer newSelection)
{
  // Ignore selection changes that occur as a result of removing a part
  if (ignoreSelectionChanges)
  {
    return;
  }
  PartPane::Pointer newPart = this->GetPaneFor(newSelection);

  // This method should only be called on objects that are already in the layout
  poco_assert(newPart != 0);

  if (newPart == requestedCurrent)
  {
    return;
  }

  this->SetSelection(newPart);

  if (newPart != 0)
  {
    newPart->SetFocus();
  }

}

void PartStack::Remove(LayoutPart::Pointer child)
{
  IPresentablePart::Pointer presentablePart = this->GetPresentablePart(child);

  // Need to remove it from the list of children before notifying the presentation
  // since it may setVisible(false) on the part, leading to a partHidden notification,
  // during which findView must not find the view being removed.  See bug 60039.
  children.removeAll(child);

  StackPresentation::Pointer presentation = this->GetPresentation();

  if (presentablePart != 0 && presentation != 0)
  {
    ignoreSelectionChanges = true;
    presentableParts.removeAll(presentablePart);
    presentation->RemovePart(presentablePart);
    presentablePart = nullptr;
    ignoreSelectionChanges = false;
  }

  if (this->GetPresentation() != 0)
  {
    child->SetContainer(ILayoutContainer::Pointer(nullptr));
  }

  if (child == requestedCurrent)
  {
    this->UpdateContainerVisibleTab();
  }
}

void PartStack::Reparent(QWidget* newParent)
{

  QWidget* control = this->GetControl();
  GuiWidgetsTweaklet* tweaklet = Tweaklets::Get(GuiWidgetsTweaklet::KEY);
  if ((control == nullptr) || (tweaklet->GetParent(control) == newParent)
      || !tweaklet->IsReparentable(control))
  {
    return;
  }

  LayoutPart::Reparent(newParent);

  for(ChildVector::iterator iter = children.begin();
      iter != children.end(); ++iter)
  {
    (*iter)->Reparent(newParent);
  }
}

void PartStack::Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild)
{
  ChildVector::iterator loc = std::find(children.begin(), children.end(), oldChild);
  int idx = 0;
  int numPlaceholders = 0;
  //subtract the number of placeholders still existing in the list
  //before this one - they wont have parts.
  for (ChildVector::iterator iter = children.begin(); iter != loc; ++iter, ++idx)
  {
    if ((*iter)->IsPlaceHolder())
    {
      numPlaceholders++;
    }
  }
  ObjectInt::Pointer cookie(new ObjectInt(idx - numPlaceholders));
  children.insert(loc, newChild);

  this->ShowPart(newChild, cookie);

  if (oldChild == requestedCurrent && newChild.Cast<PartPane>() != 0)
  {
    this->SetSelection(newChild.Cast<PartPane>());
  }

  this->Remove(oldChild);
}

int PartStack::ComputePreferredSize(bool width, int availableParallel,
    int availablePerpendicular, int preferredParallel)
{
  return this->GetPresentation()->ComputePreferredSize(width, availableParallel,
      availablePerpendicular, preferredParallel);
}

int PartStack::GetSizeFlags(bool horizontal)
{
  StackPresentation::Pointer presentation = this->GetPresentation();

  if (presentation != 0)
  {
    return presentation->GetSizeFlags(horizontal);
  }

  return 0;
}

bool PartStack::RestoreState(IMemento::Pointer memento)
{
  // Read the active tab.
  QString activeTabID; memento->GetString(WorkbenchConstants::TAG_ACTIVE_PAGE_ID, activeTabID);

  // Read the page elements.
  QList<IMemento::Pointer> children = memento->GetChildren(WorkbenchConstants::TAG_PAGE);

  // Loop through the page elements.
  for (int i = 0; i < children.size(); i++)
  {
    // Get the info details.
    IMemento::Pointer childMem = children[i];
    QString partID; childMem->GetString(WorkbenchConstants::TAG_CONTENT, partID);

    // Create the part.
    LayoutPart::Pointer part(new PartPlaceholder(partID));
    part->SetContainer(ILayoutContainer::Pointer(this));
    this->Add(part);
    //1FUN70C: ITPUI:WIN - Shouldn't set Container when not active
    //part.setContainer(this);
    if (partID == activeTabID)
    {
      this->SetSelection(part);
      // Mark this as the active part.
      //current = part;
    }
  }

  //IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
  //boolean useNewMinMax = preferenceStore.getBoolean(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
  int expanded;
  if (memento->GetInteger(WorkbenchConstants::TAG_EXPANDED, expanded))
  {
    //StartupThreading.runWithoutExceptions(new StartupRunnable()
    //    {
    //      void runWithException() throws Throwable
    //      {
    SetState(expanded != IStackPresentationSite::STATE_MINIMIZED ? IStackPresentationSite::STATE_RESTORED
        : IStackPresentationSite::STATE_MINIMIZED);
    //      }
    //    });
  }
  else
  {
    SetState(IStackPresentationSite::STATE_RESTORED);
  }

  int appearance;
  if (memento->GetInteger(WorkbenchConstants::TAG_APPEARANCE, appearance))
  {
    this->appearance = appearance;
  }

  // Determine if the presentation has saved any info here
  savedPresentationState = nullptr;
  QList<IMemento::Pointer> presentationMementos(memento
      ->GetChildren(WorkbenchConstants::TAG_PRESENTATION));

  for (int idx = 0; idx < presentationMementos.size(); idx++)
  {
    IMemento::Pointer child = presentationMementos[idx];

    QString id; child->GetString(WorkbenchConstants::TAG_ID, id);

    if (id == GetFactory()->GetId())
    {
      savedPresentationState = child;
      break;
    }
  }

  IMemento::Pointer propertiesState = memento->GetChild(WorkbenchConstants::TAG_PROPERTIES);
  if (propertiesState)
  {
    QList<IMemento::Pointer> props(propertiesState->GetChildren(WorkbenchConstants::TAG_PROPERTY));
    for (int i = 0; i < props.size(); i++)
    {
      QString id = props[i]->GetID();
      properties.insert(id, props[i]->GetTextData());
    }
  }

  //return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", 0); //$NON-NLS-1$
  return true;
}

void PartStack::SetVisible(bool makeVisible)
{
  QWidget* ctrl = this->GetControl();

  bool useShortcut = makeVisible || !isActive;

  if (ctrl != nullptr && useShortcut)
  {
    if (makeVisible == Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetVisible(ctrl))
    {
      return;
    }
  }

  if (makeVisible)
  {
    for (PresentableVector::iterator iter = presentableParts.begin();
        iter != presentableParts.end(); ++iter)
    {
      PresentablePart::Pointer next = iter->Cast<PresentablePart>();

      next->EnableInputs(isActive);
      next->EnableOutputs(isActive);
    }
  }

  LayoutPart::SetVisible(makeVisible);

  StackPresentation::Pointer presentation = this->GetPresentation();

  if (presentation != 0)
  {
    presentation->SetVisible(makeVisible);
  }

  if (!makeVisible)
  {
    for (PresentableVector::iterator iter = presentableParts.begin();
        iter != presentableParts.end(); ++iter)
    {
      PresentablePart::Pointer next = iter->Cast<PresentablePart>();

      next->EnableInputs(false);
    }
  }
}

bool PartStack::SaveState(IMemento::Pointer memento)
{

  if (GetAppearance() != PresentationFactoryUtil::ROLE_EDITOR)
  {
    // Save the active tab.
    if (requestedCurrent)
    {
      memento->PutString(WorkbenchConstants::TAG_ACTIVE_PAGE_ID, requestedCurrent->GetID());
    }

    // Write out the presentable parts (in order)
    QSet<QString> cachedIds;
    PartStack::PresentableVector pparts(GetPresentableParts());
    for (PartStack::PresentableVector::iterator ppIter = pparts.begin();
        ppIter != pparts.end(); ++ppIter)
    {
      PresentablePart::Pointer presPart = ppIter->Cast<PresentablePart>();

      IMemento::Pointer childMem = memento->CreateChild(WorkbenchConstants::TAG_PAGE);
      PartPane::Pointer part = presPart->GetPane();
      QString tabText = part->GetPartReference()->GetPartName();

      childMem->PutString(WorkbenchConstants::TAG_LABEL, tabText);
      childMem->PutString(WorkbenchConstants::TAG_CONTENT, presPart->GetPane()->GetPlaceHolderId());

      // Cache the id so we don't write it out later
      cachedIds.insert(presPart->GetPane()->GetPlaceHolderId());
    }

    for (ChildVector::iterator iter = children.begin();
        iter != children.end(); ++iter)
    {
      LayoutPart::Pointer next = *iter;

      PartPane::Pointer part;
      if ((part = next.Cast<PartPane>()))
      {
        // Have we already written it out?
        if (cachedIds.find(part->GetPlaceHolderId()) != cachedIds.end())
        continue;
      }

      IMemento::Pointer childMem = memento
      ->CreateChild(WorkbenchConstants::TAG_PAGE);

      QString tabText = "LabelNotFound";
      if (part)
      {
        tabText = part->GetPartReference()->GetPartName();
      }
      childMem->PutString(WorkbenchConstants::TAG_LABEL, tabText);
      childMem->PutString(WorkbenchConstants::TAG_CONTENT, next->GetID());
    }
  }

  //  IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
  //  boolean useNewMinMax = preferenceStore.getBoolean(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
  //  if (useNewMinMax)
  //  {
  memento->PutInteger(WorkbenchConstants::TAG_EXPANDED, presentationSite->GetState());
  //  }
  //  else
  //  {
  //    memento
  //    .putInteger(
  //        IWorkbenchConstants.TAG_EXPANDED,
  //        (presentationSite.getState() == IStackPresentationSite.STATE_MINIMIZED) ? IStackPresentationSite.STATE_MINIMIZED
  //        : IStackPresentationSite.STATE_RESTORED);
  //  }

  memento->PutInteger(WorkbenchConstants::TAG_APPEARANCE, appearance);

  this->SavePresentationState();

  if (savedPresentationState)
  {
    IMemento::Pointer presentationState = memento
    ->CreateChild(WorkbenchConstants::TAG_PRESENTATION);
    presentationState->PutMemento(savedPresentationState);
  }

  if (!properties.empty())
  {
    IMemento::Pointer propertiesState = memento->CreateChild(WorkbenchConstants::TAG_PROPERTIES);
    for (QHash<QString, QString>::iterator iterator = properties.begin();
        iterator != properties.end(); ++iterator)
    {
      if (iterator.value().isEmpty()) continue;

      IMemento::Pointer prop = propertiesState->CreateChild(WorkbenchConstants::TAG_PROPERTY, iterator.key());
      prop->PutTextData(iterator.value());
    }
  }

  //return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", 0);
  return true;
}

WorkbenchPage::Pointer PartStack::GetPage()
{
  //  WorkbenchWindow::Pointer window = this->GetWorkbenchWindow().Cast<WorkbenchWindow>();
  //
  //  if (window == 0)
  //  {
  //    return 0;
  //  }
  //
  //  return window->GetActivePage().Cast<WorkbenchPage>();
  return WorkbenchPage::Pointer(page);
}

void PartStack::SetActive(int activeState)
{

  //  if (activeState == StackPresentation::AS_ACTIVE_FOCUS && isMinimized)
  //  {
  //    setMinimized(false);
  //  }

  presentationSite->SetActive(activeState);
}

int PartStack::GetActive() const
{
  return presentationSite->GetActive();
}

void PartStack::SetSelection(LayoutPart::Pointer part)
{
  if (part == requestedCurrent)
  {
    return;
  }

  requestedCurrent = part;

  this->RefreshPresentationSelection();
}

void PartStack::UpdateActions(PresentablePart::Pointer  /*current*/)
{

}

void PartStack::HandleDeferredEvents()
{
  LayoutPart::HandleDeferredEvents();

  this->RefreshPresentationSelection();
}

void PartStack::RefreshPresentationSelection()
{
  // If deferring UI updates, exit.
  if (this->IsDeferred())
  {
    return;
  }

  // If the presentation is already displaying the desired part, then there's nothing
  // to do.
  if (current == requestedCurrent)
  {
    return;
  }

  StackPresentation::Pointer presentation = this->GetPresentation();
  if (presentation != 0)
  {

    presentationCurrent = this->GetPresentablePart(requestedCurrent);

    // this->UupdateActions(presentationCurrent);


    if (presentationCurrent != 0 && presentation != 0)
    {
      requestedCurrent->CreateControl(this->GetParent());
      GuiWidgetsTweaklet* tweaklet = Tweaklets::Get(GuiWidgetsTweaklet::KEY);
      if (tweaklet->GetParent(requestedCurrent->GetControl()) !=
          tweaklet->GetParent(this->GetControl()))
      {
        requestedCurrent->Reparent(tweaklet->GetParent(this->GetControl()));
      }

      presentation->SelectPart(presentationCurrent);

    }

    // Update the return value of getVisiblePart
    current = requestedCurrent;
    this->FireInternalPropertyChange(PROP_SELECTION);
  }
}

int PartStack::GetState()
{
  return presentationSite->GetState();
}

void PartStack::SetState(const int newState)
{
  int oldState = presentationSite->GetState();
  if (!this->SupportsState(newState) || newState == oldState)
  {
    return;
  }

  //  WorkbenchWindow::Pointer wbw = this->GetPage()->GetWorkbenchWindow().Cast<WorkbenchWindow>();
  //  if (wbw == 0 || wbw->GetShell() == 0 || wbw->GetActivePage() == 0)
  //    return;
  //
  //  WorkbenchPage::Pointer page = wbw->GetActivePage();
  //
  //  bool useNewMinMax = Perspective::UseNewMinMax(page->GetActivePerspective());
  //
  //  // we have to fiddle with the zoom behavior to satisfy Intro req's
  //  // by usning the old zoom behavior for its stack
  //  if (newState == IStackPresentationSite::STATE_MAXIMIZED)
  //    useNewMinMax = useNewMinMax; // && !this->IsIntroInStack();
  //  else if (newState == IStackPresentationSite::STATE_RESTORED)
  //  {
  //    PartStack::Pointer maxStack = page->GetActivePerspective()->GetPresentation()->GetMaximizedStack();
  //    useNewMinMax = useNewMinMax && maxStack == this;
  //  }
  //
  //  if (useNewMinMax)
  //  {
  //    //StartupThreading.runWithoutExceptions(new StartupRunnable()
  //    //    {
  //    //      void runWithException() throws Throwable
  //    //      {
  //    wbw->GetPageComposite()->SetRedraw(false);
  //    try
  //    {
  //      if (newState == IStackPresentationSite::STATE_MAXIMIZED)
  //      {
  //        smartZoom();
  //      }
  //      else if (oldState == IStackPresentationSite::STATE_MAXIMIZED)
  //      {
  //        smartUnzoom();
  //      }
  //
  //      if (newState == IStackPresentationSite::STATE_MINIMIZED)
  //      {
  //        setMinimized(true);
  //      }
  //
  //      wbw.getPageComposite().setRedraw(true);
  //
  //      // Force a redraw (fixes Mac refresh)
  //      wbw.getShell().redraw();
  //
  //    }
  //    catch (...)
  //    {
  //      wbw.getPageComposite().setRedraw(true);
  //
  //      // Force a redraw (fixes Mac refresh)
  //      wbw.getShell().redraw();
  //    }
  //
  //    this->SetPresentationState(newState);
  //    //     }
  //    //   });
  //  }
  //  else
  //  {
  ////    bool minimized = (newState == IStackPresentationSite::STATE_MINIMIZED);
  ////    this->SetMinimized(minimized);
  ////
  ////    if (newState == IStackPresentationSite::STATE_MAXIMIZED)
  ////    {
  ////      requestZoomIn();
  ////    }
  ////    else if (oldState == IStackPresentationSite::STATE_MAXIMIZED)
  ////    {
  ////      requestZoomOut();
  ////
  ////      if (newState == IStackPresentationSite::STATE_MINIMIZED)
  ////      setMinimized(true);
  ////    }
  //  }
}

void PartStack::ShowPart(LayoutPart::Pointer part, Object::Pointer cookie)
{

  if (this->GetPresentation() == 0)
  {
    return;
  }

  if (part->IsPlaceHolder())
  {
    part->SetContainer(ILayoutContainer::Pointer(this));
    return;
  }

  if (part.Cast<PartPane>() == 0)
  {
    WorkbenchPlugin::Log("Incorrect part " + part->GetID() + "contained in a part stack");
    return;
  }

  PartPane::Pointer pane = part.Cast<PartPane>();

  PresentablePart::Pointer presentablePart(new PresentablePart(pane, Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetParent(this->GetControl())));
  presentableParts.push_back(presentablePart);

  if (isActive)
  {
    part->SetContainer(ILayoutContainer::Pointer(this));

    // The active part should always be enabled
    if (part->GetControl() != nullptr)
    Tweaklets::Get(GuiWidgetsTweaklet::KEY)->SetEnabled(part->GetControl(), true);
  }

  presentationSite->GetPresentation()->AddPart(presentablePart, cookie);

  if (requestedCurrent == 0)
  {
    this->SetSelection(pane);
  }

  //  if (childObscuredByZoom(part))
  //  {
  //    presentablePart.enableInputs(false);
  //  }
}

void PartStack::UpdateContainerVisibleTab()
{
  ChildVector parts = this->GetChildren();

  if (parts.size() < 1)
  {
    this->SetSelection(LayoutPart::Pointer(nullptr));
    return;
  }

  PartPane::Pointer selPart;
  int topIndex = 0;
  WorkbenchPage::Pointer page = this->GetPage();

  if (page != 0)
  {
    QList<IWorkbenchPartReference::Pointer> sortedParts = page->GetSortedParts();
    for (ChildVector::iterator partIter = parts.begin();
        partIter != parts.end(); ++partIter)
    {
      if (partIter->Cast<PartPane>() != 0)
      {
        IWorkbenchPartReference::Pointer part = partIter->Cast<PartPane>()
        ->GetPartReference();
        int index = static_cast<int>(std::find(sortedParts.begin(), sortedParts.end(), part) - sortedParts.begin());
        if (index >= topIndex)
        {
          topIndex = index;
          selPart = partIter->Cast<PartPane>();
        }
      }
    }

  }

  if (selPart == 0)
  {
    PresentableVector presentableParts = this->GetPresentableParts();
    if (presentableParts.size() != 0)
    {
      IPresentablePart::Pointer part = presentableParts.front();

      selPart = this->GetPaneFor(part);
    }
  }

  this->SetSelection(selPart);
}

void PartStack::ShowSystemMenu()
{
  //this->GetPresentation()->ShowSystemMenu();
}

void PartStack::ShowPaneMenu()
{
  //this->GetPresentation()->ShowPaneMenu();
}

void PartStack::ShowPartList()
{
  this->GetPresentation()->ShowPartList();
}

QList<QWidget*> PartStack::GetTabList(LayoutPart::Pointer part)
{
  if (part != 0)
  {
    IPresentablePart::Pointer presentablePart = this->GetPresentablePart(part);
    StackPresentation::Pointer presentation = this->GetPresentation();

    if (presentablePart != 0 && presentation != 0)
    {
      return presentation->GetTabList(presentablePart);
    }
  }

  return QList<QWidget*>();
}

void PartStack::DragStart(IPresentablePart::Pointer beingDragged, QPoint& initialLocation,
    bool keyboard)
{
  if (beingDragged == 0)
  {
    this->PaneDragStart(PartPane::Pointer(nullptr), initialLocation, keyboard);
  }
  else
  {
    if (presentationSite->IsPartMoveable(beingDragged))
    {
      PartPane::Pointer pane = this->GetPaneFor(beingDragged);

      if (pane != 0)
      {
        this->PaneDragStart(pane, initialLocation, keyboard);
      }
    }
  }
}

void PartStack::PaneDragStart(PartPane::Pointer pane, QPoint& initialLocation,
    bool keyboard)
{
  if (pane == 0)
  {
    if (this->CanMoveFolder())
    {
      if (presentationSite->GetState() == IStackPresentationSite::STATE_MAXIMIZED)
      {
        // Calculate where the initial location was BEFORE the 'restore'...as a percentage
        QRect bounds = Geometry::ToDisplay(this->GetParent(),
            Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->GetPresentation()->GetControl()));
        float xpct = (initialLocation.x() - bounds.x()) / (float)(bounds.width());
        float ypct = (initialLocation.y() - bounds.y()) / (float)(bounds.height());

        // Only restore if we're dragging views/view stacks
        if (this->GetAppearance() != PresentationFactoryUtil::ROLE_EDITOR)
          this->SetState(IStackPresentationSite::STATE_RESTORED);

        // Now, adjust the initial location to be within the bounds of the restored rect
        bounds = Geometry::ToDisplay(this->GetParent(),
            Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->GetPresentation()->GetControl()));
        initialLocation.setX((int) (bounds.x() + (xpct * bounds.width())));
        initialLocation.setY((int) (bounds.y() + (ypct * bounds.height())));
      }

      DragUtil::PerformDrag(Object::Pointer(this), Geometry::ToDisplay(this->GetParent(),
              Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->GetPresentation()->GetControl())),
          initialLocation, !keyboard);
    }
  }
  else
  {
    if (presentationSite->GetState() == IStackPresentationSite::STATE_MAXIMIZED)
    {
      // Calculate where the initial location was BEFORE the 'restore'...as a percentage
      QRect bounds = Geometry::ToDisplay(this->GetParent(),
          Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->GetPresentation()->GetControl()));
      float xpct = (initialLocation.x() - bounds.x()) / (float)(bounds.width());
      float ypct = (initialLocation.y() - bounds.y()) / (float)(bounds.height());

      // Only restore if we're dragging views/view stacks
      if (this->GetAppearance() != PresentationFactoryUtil::ROLE_EDITOR)
        this->SetState(IStackPresentationSite::STATE_RESTORED);

      // Now, adjust the initial location to be within the bounds of the restored rect
      // See bug 100908
      bounds = Geometry::ToDisplay(this->GetParent(),
          Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->GetPresentation()->GetControl()));
      initialLocation.setX((int) (bounds.x() + (xpct * bounds.width())));
      initialLocation.setY((int) (bounds.y() + (ypct * bounds.height())));
    }

    DragUtil::PerformDrag(pane, Geometry::ToDisplay(this->GetParent(),
            Tweaklets::Get(GuiWidgetsTweaklet::KEY)->GetBounds(this->GetPresentation()->GetControl())),
        initialLocation, !keyboard);
  }
}

IMemento::Pointer PartStack::GetSavedPresentationState()
{
  return savedPresentationState;
}

void PartStack::FireInternalPropertyChange(int id)
{
  ObjectInt::Pointer val(new ObjectInt(id));
  Object::Pointer source(this);
  PropertyChangeEvent::Pointer event(new PropertyChangeEvent(source,
          IWorkbenchPartConstants::INTEGER_PROPERTY, val, val));
  propEvents.propertyChange(event);
}

QString PartStack::GetProperty(const QString& id)
{
  return properties[id];
}

void PartStack::SetProperty(const QString& id, const QString& value)
{
  if (value == "")
  {
    properties.remove(id);
  }
  else
  {
    properties.insert(id, value);
  }
}

void PartStack::CopyAppearanceProperties(PartStack::Pointer copyTo)
{
  copyTo->appearance = this->appearance;
  if (!properties.empty())
  {
    for (QHash<QString, QString>::iterator iter = properties.begin();
        iter != properties.end(); ++iter)
    {
      copyTo->SetProperty(iter.key(), iter.value());
    }
  }
}

void PartStack::ResizeChild(LayoutPart::Pointer  /*childThatChanged*/)
{

}

}
