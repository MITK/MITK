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

#include "cherryPartStack.h"

namespace cherry
{

const int PartStack::PROP_SELECTION = 0x42;

PartStack::PartStack() :
  LayoutPart("PartStack"), isActive(true), ignoreSelectionChanges(false)
{

}

bool PartStack::IsStandalone()
{
  return (appearance == PresentationFactoryUtil.ROLE_STANDALONE || appearance
      == PresentationFactoryUtil.ROLE_STANDALONE_NOTITLE);
}

IWorkbenchPart::Pointer PartStack::GetSelectedPart()
{
  return presentationCurrent;
}

void PartStack::TestInvariants()
{
  Control focusControl = Display.getCurrent().getFocusControl();

  boolean currentFound = false;

  LayoutPart[] children = getChildren();

  for (int idx = 0; idx < children.length; idx++)
  {
    LayoutPart child = children[idx];

    // No null children allowed
    Assert.isNotNull(child, "null children are not allowed in PartStack"); //$NON-NLS-1$

    // This object can only contain placeholders or PartPanes
    Assert.isTrue(child instanceof PartPlaceholder
        || child instanceof PartPane,
        "PartStack can only contain PartPlaceholders or PartPanes"); //$NON-NLS-1$

    // Ensure that all the PartPanes have an associated presentable part 
    IPresentablePart part = getPresentablePart(child);
    if (child instanceof PartPane)
    {
      Assert.isNotNull(part,
          "All PartPanes must have a non-null IPresentablePart"); //$NON-NLS-1$
    }

    // Ensure that the child's backpointer points to this stack
    ILayoutContainer childContainer = child.getContainer();

    // Disable tests for placeholders -- PartPlaceholder backpointers don't
    // obey the usual rules -- they sometimes point to a container placeholder
    // for this stack instead of the real stack.
    if (!(child instanceof PartPlaceholder)
)      {

        if (isDisposed())
        {

          // Currently, we allow null backpointers if the widgetry is disposed.
          // However, it is never valid for the child to have a parent other than
          // this object
          if (childContainer != null)
          {
            Assert
            .isTrue(childContainer == this,
                "PartStack has a child that thinks it has a different parent"); //$NON-NLS-1$
          }
        }
        else
        {
          // If the widgetry exists, the child's backpointer must point to us
          Assert
          .isTrue(childContainer == this,
              "PartStack has a child that thinks it has a different parent"); //$NON-NLS-1$

          // If this child has focus, then ensure that it is selected and that we have
          // the active appearance.

          if (SwtUtil.isChild(child.getControl(), focusControl))
          {
            Assert.isTrue(child == current,
                "The part with focus is not the selected part"); //$NON-NLS-1$
            //  focus check commented out since it fails when focus workaround in LayoutPart.setVisible is not present            
            //              Assert.isTrue(getActive() == StackPresentation.AS_ACTIVE_FOCUS);
          }
        }
      }

      // Ensure that "current" points to a valid child
      if (child == current)
      {
        currentFound = true;
      }

      // Test the child's internal state
      child.testInvariants();
    }

    // If we have at least one child, ensure that the "current" pointer points to one of them
    if (!isDisposed() && getPresentableParts().size()> 0)
    {
      Assert.isTrue(currentFound);

      if (!isDisposed())
      {
        StackPresentation presentation = getPresentation();

        // If the presentation controls have focus, ensure that we have the active appearance
        if (SwtUtil.isChild(presentation.getControl(), focusControl))
        {
          Assert
          .isTrue(
              getActive() == StackPresentation.AS_ACTIVE_FOCUS,
              "The presentation has focus but does not have the active appearance"); //$NON-NLS-1$
        }
      }
    }

    // Check to that we're displaying the zoomed icon iff we're actually maximized
    Assert.isTrue((getState() == IStackPresentationSite.STATE_MAXIMIZED)
        == (getContainer() != null && getContainer().childIsZoomed(this)));

  }

  void PartStack::DescribeLayout(std::string& buf)
  {
    int activeState = getActive();
    if (activeState == StackPresentation.AS_ACTIVE_FOCUS)
    {
      buf.append("active "); //$NON-NLS-1$
    }
    else if (activeState == StackPresentation.AS_ACTIVE_NOFOCUS)
    {
      buf.append("active_nofocus "); //$NON-NLS-1$
    }

    buf.append("("); //$NON-NLS-1$

    LayoutPart[] children = ((ILayoutContainer) this).getChildren();

    int visibleChildren = 0;

    for (int idx = 0; idx < children.length; idx++)
    {

      LayoutPart next = children[idx];
      if (!(next instanceof PartPlaceholder))
      {
        if (idx> 0)
        {
          buf.append(", "); //$NON-NLS-1$
        }

        if (next == requestedCurrent)
        {
          buf.append("*"); //$NON-NLS-1$
        }

        next.describeLayout(buf);

        visibleChildren++;
      }
    }

    buf.append(")"); //$NON-NLS-1$
  }

  void PartStack::Add(LayoutPart::Pointer child)
  {
    add(child, null);
  }

  void PartStack::Add(LayoutPart::Pointer newChild, Object* cookie)
  {
    children.add(newChild);

    // Fix for bug 78470:
    if(!(newChild.getContainer() instanceof ContainerPlaceholder))
    {
      newChild.setContainer(this);
    }

    showPart(newChild, cookie);
  }

  bool PartStack::AllowsAdd(LayoutPart::Pointer toAdd)
  {
    return !isStandalone();
  }

  bool PartStack::AllowsAutoFocus()
  {
    if (presentationSite.getState() == IStackPresentationSite.STATE_MINIMIZED)
    {
      return false;
    }

    return super.allowsAutoFocus();
  }

  void PartStack::Close(const std::vector<IWorkbenchPart::Pointer>& parts)
  {
    for (int idx = 0; idx < parts.length; idx++)
    {
      IPresentablePart part = parts[idx];

      close(part);
    }
  }

  void PartStack::Close(IWorkbenchPart::Pointer part)
  {
    if (!presentationSite.isCloseable(part))
    {
      return;
    }

    LayoutPart layoutPart = getPaneFor(part);

    if (layoutPart != null && layoutPart instanceof PartPane)
    {
      PartPane viewPane = (PartPane) layoutPart;

      viewPane.doHide();
    }
  }

  void* PartStack::CreateControl(void* parent)
  {
    if (!isDisposed())
    {
      return;
    }

    AbstractPresentationFactory factory = getFactory();

    PresentationSerializer serializer = new PresentationSerializer(
        getPresentableParts());

    StackPresentation presentation = PresentationFactoryUtil
    .createPresentation(factory, appearance, parent,
        presentationSite, serializer, savedPresentationState);

    createControl(parent, presentation);
    getControl().moveBelow(null);
  }

  void PartStack::SetActive(bool isActive)
  {

    this.isActive = isActive;
    // Add all visible children to the presentation
    Iterator iter = children.iterator();
    while (iter.hasNext())
    {
      LayoutPart part = (LayoutPart) iter.next();

      part.setContainer(isActive ? this : null);
    }

    for (Iterator iterator = presentableParts.iterator(); iterator.hasNext();)
    {
      PresentablePart next = (PresentablePart) iterator.next();

      next.enableInputs(isActive);
      next.enableOutputs(isActive);
    }
  }

  void PartStack::CreateControl(void* parent)
  { //, StackPresentation presentation) {

    Assert.isTrue(isDisposed());

    if (presentationSite.getPresentation() != null)
    {
      return;
    }

    presentationSite.setPresentation(presentation);

    // Add all visible children to the presentation
    // Use a copy of the current set of children to avoid a ConcurrentModificationException
    // if a part is added to the same stack while iterating over the children (bug 78470)
    LayoutPart[] childParts = (LayoutPart[]) children.toArray(new LayoutPart[children.size()]);
    for (int i = 0; i < childParts.length; i++)
    {
      LayoutPart part = childParts[i];
      showPart(part, null);
    }

    if (savedPresentationState!=null)
    {
      PresentationSerializer serializer = new PresentationSerializer(
          getPresentableParts());
      presentation.restoreState(serializer, savedPresentationState);
    }

    Control ctrl = getPresentation().getControl();

    ctrl.setData(this);

    // We should not have a placeholder selected once we've created the widgetry
    if (requestedCurrent instanceof PartPlaceholder)
    {
      requestedCurrent = null;
      updateContainerVisibleTab();
    }

    refreshPresentationSelection();
  }

  ~PartStack::PartStack()
  {

    if (isDisposed())
    {
      return;
    }

    savePresentationState();

    presentationSite.dispose();

    for (Iterator iter = presentableParts.iterator(); iter.hasNext();)
    {
      PresentablePart part = (PresentablePart) iter.next();

      part.dispose();
    }
    presentableParts.clear();

    presentationCurrent = null;
    current = null;
    fireInternalPropertyChange(PROP_SELECTION);
  }

  std::vector<LayoutPart::Pointer> PartStack::GetChildren()
  {
    return (LayoutPart[]) children.toArray(new LayoutPart[children.size()]);
  }

  void* PartStack::GetControl()
  {
    StackPresentation presentation = getPresentation();

    if (presentation == null)
    {
      return null;
    }

    return presentation.getControl();
  }

  /**
   * Answer the number of children.
   */
  int PartStack::GetItemCount()
  {
    if (isDisposed())
    {
      return children.size();
    }
    return getPresentableParts().size();
  }

  IPartPane::Pointer PartStack::GetPaneFor(IWorkbenchPart::Pointer part)
  {
    if (part == null || !(part instanceof PresentablePart))
    {
      return null;
    }

    return ((PresentablePart)part).getPane();
  }

  void* PartStack::GetParent()
  {
    return getControl().getParent();
  }

  IPartPane::Pointer PartStack::GetSelection()
  {
    if (current instanceof PartPane)
    {
      return (PartPane) current;
    }
    return null;
  }

  void PartStack::PresentationSelectionChanged(IWorkbenchPart::Pointer newSelection)
  {
    // Ignore selection changes that occur as a result of removing a part
    if (ignoreSelectionChanges)
    {
      return;
    }
    LayoutPart newPart = getPaneFor(newSelection);

    // This method should only be called on objects that are already in the layout
    Assert.isNotNull(newPart);

    if (newPart == requestedCurrent)
    {
      return;
    }

    setSelection(newPart);

    if (newPart != null)
    {
      newPart.setFocus();
    }

  }

  void PartStack::Remove(LayoutPart::Pointer child)
  {
    PresentablePart presentablePart = getPresentablePart(child);

    // Need to remove it from the list of children before notifying the presentation
    // since it may setVisible(false) on the part, leading to a partHidden notification,
    // during which findView must not find the view being removed.  See bug 60039. 
    children.remove(child);

    StackPresentation presentation = getPresentation();

    if (presentablePart != null && presentation != null)
    {
      ignoreSelectionChanges = true;
      presentableParts .remove(presentablePart);
      presentation.removePart(presentablePart);
      presentablePart.dispose();
      ignoreSelectionChanges = false;
    }

    if (!isDisposed())
    {
      child.setContainer(null);
    }

    if (child == requestedCurrent)
    {
      updateContainerVisibleTab();
    }
  }

  void PartStack::Reparent(void* newParent)
  {

    Control control = getControl();
    if ((control == null) || (control.getParent() == newParent) || !control.isReparentable())
    {
      return;
    }

    super.reparent(newParent);

    Iterator iter = children.iterator();
    while (iter.hasNext())
    {
      LayoutPart next = (LayoutPart) iter.next();
      next.reparent(newParent);
    }
  }

  void PartStack::Replace(LayoutPart::Pointer oldChild, LayoutPart::Pointer newChild)
  {
    int idx = children.indexOf(oldChild);
    int numPlaceholders = 0;
    //subtract the number of placeholders still existing in the list 
    //before this one - they wont have parts.
    for (int i = 0; i < idx; i++)
    {
      if (children.get(i) instanceof PartPlaceholder)
      {
        numPlaceholders++;
      }
    }
    Integer cookie = new Integer(idx - numPlaceholders);
    children.add(idx, newChild);

    showPart(newChild, cookie);

    if (oldChild == requestedCurrent && !(newChild instanceof PartPlaceholder))
    {
      setSelection(newChild);
    }

    remove(oldChild);
  }

  bool PartStack::RestoreState(IMemento::Pointer memento)
  {
    // Read the active tab.
    String activeTabID = memento
    .getString(IWorkbenchConstants.TAG_ACTIVE_PAGE_ID);

    // Read the page elements.
    IMemento[] children = memento.getChildren(IWorkbenchConstants.TAG_PAGE);
    if (children != null)
    {
      // Loop through the page elements.
      for (int i = 0; i < children.length; i++)
      {
        // Get the info details.
        IMemento childMem = children[i];
        String partID = childMem
        .getString(IWorkbenchConstants.TAG_CONTENT);

        // Create the part.
        LayoutPart part = new PartPlaceholder(partID);
        part.setContainer(this);
        add(part);
        //1FUN70C: ITPUI:WIN - Shouldn't set Container when not active
        //part.setContainer(this);
        if (partID.equals(activeTabID))
        {
          setSelection(part);
          // Mark this as the active part.
          //current = part;
        }
      }
    }

    IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
    boolean useNewMinMax = preferenceStore.getBoolean(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
    final Integer expanded = memento.getInteger(IWorkbenchConstants.TAG_EXPANDED);
    if (useNewMinMax && expanded != null)
    {
      StartupThreading.runWithoutExceptions(new StartupRunnable()
          {
            void runWithException() throws Throwable
            {
              setState((expanded == null || expanded.intValue() != IStackPresentationSite.STATE_MINIMIZED) ? IStackPresentationSite.STATE_RESTORED
                  : IStackPresentationSite.STATE_MINIMIZED);
            }
          });
    }
    else
    {
      setState((expanded == null || expanded.intValue() != IStackPresentationSite.STATE_MINIMIZED) ? IStackPresentationSite.STATE_RESTORED
          : IStackPresentationSite.STATE_MINIMIZED);
    }

    Integer appearance = memento
    .getInteger(IWorkbenchConstants.TAG_APPEARANCE);
    if (appearance != null)
    {
      this.appearance = appearance.intValue();
    }

    // Determine if the presentation has saved any info here
    savedPresentationState = null;
    IMemento[] presentationMementos = memento
    .getChildren(IWorkbenchConstants.TAG_PRESENTATION);

    for (int idx = 0; idx < presentationMementos.length; idx++)
    {
      IMemento child = presentationMementos[idx];

      String id = child.getString(IWorkbenchConstants.TAG_ID);

      if (Util.equals(id, getFactory().getId()))
      {
        savedPresentationState = child;
        break;
      }
    }

    IMemento propertiesState = memento.getChild(IWorkbenchConstants.TAG_PROPERTIES);
    if (propertiesState != null)
    {
      IMemento[] props = propertiesState.getChildren(IWorkbenchConstants.TAG_PROPERTY);
      for (int i = 0; i < props.length; i++)
      {
        properties.put(props[i].getID(), props[i].getTextData());
      }
    }

    return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
  }

  void PartStack::SetVisible(bool makeVisible)
  {
    Control ctrl = getControl();

    boolean useShortcut = makeVisible || !isActive;

    if (!SwtUtil.isDisposed(ctrl) && useShortcut)
    {
      if (makeVisible == ctrl.getVisible())
      {
        return;
      }
    }

    if (makeVisible)
    {
      for (Iterator iterator = presentableParts.iterator(); iterator.hasNext();)
      {
        PresentablePart next = (PresentablePart) iterator.next();

        next.enableInputs(isActive);
        next.enableOutputs(isActive);
      }
    }

    super.setVisible(makeVisible);

    StackPresentation presentation = getPresentation();

    if (presentation != null)
    {
      presentation.setVisible(makeVisible);
    }

    if (!makeVisible)
    {
      for (Iterator iterator = presentableParts.iterator(); iterator.hasNext();)
      {
        PresentablePart next = (PresentablePart) iterator.next();

        next.enableInputs(false);
      }
    }
  }

  bool PartStack::SaveState(IMemento::Pointer memento)
  {

    // Save the active tab.
    if (requestedCurrent != null)
    {
      memento.putString(IWorkbenchConstants.TAG_ACTIVE_PAGE_ID, requestedCurrent
          .getCompoundId());
    }

    // Write out the presentable parts (in order)
    Set cachedIds = new HashSet();
    Iterator ppIter = getPresentableParts().iterator();
    while (ppIter.hasNext())
    {
      PresentablePart presPart = (PresentablePart) ppIter.next();

      IMemento childMem = memento
      .createChild(IWorkbenchConstants.TAG_PAGE);
      PartPane part = presPart.getPane();
      String tabText = part.getPartReference().getPartName();

      childMem.putString(IWorkbenchConstants.TAG_LABEL, tabText);
      childMem.putString(IWorkbenchConstants.TAG_CONTENT, presPart.getPane().getPlaceHolderId());

      // Cache the id so we don't write it out later
      cachedIds.add(presPart.getPane().getPlaceHolderId());
    }

    Iterator iter = children.iterator();
    while (iter.hasNext())
    {
      LayoutPart next = (LayoutPart) iter.next();

      PartPane part = null;
      if (next instanceof PartPane)
      {
        // Have we already written it out?
        if (cachedIds.contains(((PartPane)next).getPlaceHolderId()))
        continue;

        part = (PartPane)next;
      }

      IMemento childMem = memento
      .createChild(IWorkbenchConstants.TAG_PAGE);

      String tabText = "LabelNotFound"; //$NON-NLS-1$ 
      if (part != null)
      {
        tabText = part.getPartReference().getPartName();
      }
      childMem.putString(IWorkbenchConstants.TAG_LABEL, tabText);
      childMem.putString(IWorkbenchConstants.TAG_CONTENT, next
          .getCompoundId());
    }

    IPreferenceStore preferenceStore = PrefUtil.getAPIPreferenceStore();
    boolean useNewMinMax = preferenceStore.getBoolean(IWorkbenchPreferenceConstants.ENABLE_NEW_MIN_MAX);
    if (useNewMinMax)
    {
      memento.putInteger(IWorkbenchConstants.TAG_EXPANDED, presentationSite.getState());
    }
    else
    {
      memento
      .putInteger(
          IWorkbenchConstants.TAG_EXPANDED,
          (presentationSite.getState() == IStackPresentationSite.STATE_MINIMIZED) ? IStackPresentationSite.STATE_MINIMIZED
          : IStackPresentationSite.STATE_RESTORED);
    }

    memento.putInteger(IWorkbenchConstants.TAG_APPEARANCE, appearance);

    savePresentationState();

    if (savedPresentationState != null)
    {
      IMemento presentationState = memento
      .createChild(IWorkbenchConstants.TAG_PRESENTATION);
      presentationState.putMemento(savedPresentationState);
    }

    if (!properties.isEmpty())
    {
      IMemento propertiesState = memento.createChild(IWorkbenchConstants.TAG_PROPERTIES);
      Set ids = properties.keySet();
      for (Iterator iterator = ids.iterator(); iterator.hasNext();)
      {
        String id = (String)iterator.next();

        if (properties.get(id) == null) continue;

        IMemento prop = propertiesState.createChild(IWorkbenchConstants.TAG_PROPERTY, id);
        prop.putTextData((String)properties.get(id));
      }
    }

    return new Status(IStatus.OK, PlatformUI.PLUGIN_ID, 0, "", null); //$NON-NLS-1$
  }

  WorkbenchPage::Pointer PartStack::GetPage()
  {
    WorkbenchWindow window = (WorkbenchWindow) getWorkbenchWindow();

    if (window == null)
    {
      return null;
    }

    return (WorkbenchPage) window.getActivePage();
  }

  void PartStack::SetActive(int activeState)
  {

    if (activeState == StackPresentation.AS_ACTIVE_FOCUS && isMinimized)
    {
      setMinimized(false);
    }

    presentationSite.setActive(activeState);
  }

  int PartStack::GetActive()
  {
    return presentationSite.getActive();
  }

  void PartStack::SetSelection(LayoutPart::Pointer part)
  {
    if (part == requestedCurrent)
    {
      return;
    }

    requestedCurrent = part;

    refreshPresentationSelection();
  }

  void PartStack::HandleDeferredEvents()
  {
    super.handleDeferredEvents();

    refreshPresentationSelection();
  }

  void PartStack::RefreshPresentationSelection()
  {
    // If deferring UI updates, exit.
    if (isDeferred())
    {
      return;
    }

    // If the presentation is already displaying the desired part, then there's nothing
    // to do.
    if (current == requestedCurrent)
    {
      return;
    }

    StackPresentation presentation = getPresentation();
    if (presentation != null)
    {

      presentationCurrent = getPresentablePart(requestedCurrent);

      if (!isDisposed())
      {
        updateActions(presentationCurrent);
      }

      if (presentationCurrent != null && presentation != null)
      {
        requestedCurrent.createControl(getParent());
        if (requestedCurrent.getControl().getParent() != getControl()
            .getParent())
        {
          requestedCurrent.reparent(getControl().getParent());
        }

        presentation.selectPart(presentationCurrent);

      }

      // Update the return value of getVisiblePart
      current = requestedCurrent;
      fireInternalPropertyChange(PROP_SELECTION);
    }
  }

  int PartStack::GetState()
  {
    return presentationSite.getState();
  }

  void PartStack::SetState(const int newState)
  {
    final int oldState = presentationSite.getState();
    if (!supportsState(newState) || newState == oldState)
    {
      return;
    }

    final WorkbenchWindow wbw = (WorkbenchWindow) getPage().getWorkbenchWindow();
    if (wbw == null || wbw.getShell() == null || wbw.getActiveWorkbenchPage() == null)
    return;

    WorkbenchPage page = wbw.getActiveWorkbenchPage();
    if (page == null)
    return;

    boolean useNewMinMax = Perspective.useNewMinMax(page.getActivePerspective());

    // we have to fiddle with the zoom behavior to satisfy Intro req's
    // by usning the old zoom behavior for its stack
    if (newState == IStackPresentationSite.STATE_MAXIMIZED)
    useNewMinMax = useNewMinMax && !isIntroInStack();
    else if (newState == IStackPresentationSite.STATE_RESTORED)
    {
      PartStack maxStack = page.getActivePerspective().getPresentation().getMaximizedStack();
      useNewMinMax = useNewMinMax && maxStack == this;
    }

    if (useNewMinMax)
    {
      StartupThreading.runWithoutExceptions(new StartupRunnable()
          {
            void runWithException() throws Throwable
            {
              wbw.getPageComposite().setRedraw(false);
              try
              {
                if (newState == IStackPresentationSite.STATE_MAXIMIZED)
                {
                  smartZoom();
                }
                else if (oldState == IStackPresentationSite.STATE_MAXIMIZED)
                {
                  smartUnzoom();
                }

                if (newState == IStackPresentationSite.STATE_MINIMIZED)
                {
                  setMinimized(true);
                }
              }finally
              {
                wbw.getPageComposite().setRedraw(true);

                // Force a redraw (fixes Mac refresh)
                wbw.getShell().redraw();
              }

              setPresentationState(newState);
            }
          });
    }
    else
    {
      boolean minimized = (newState == IStackPresentationSite.STATE_MINIMIZED);
      setMinimized(minimized);

      if (newState == IStackPresentationSite.STATE_MAXIMIZED)
      {
        requestZoomIn();
      }
      else if (oldState == IStackPresentationSite.STATE_MAXIMIZED)
      {
        requestZoomOut();

        if (newState == IStackPresentationSite.STATE_MINIMIZED)
        setMinimized(true);
      }
    }
  }

  void PartStack::ShowPart(LayoutPart::Pointer part, Object* cookie)
  {

    if (isDisposed())
    {
      return;
    }

    if ((part instanceof PartPlaceholder))
    {
      part.setContainer(this);
      return;
    }

    if (!(part instanceof PartPane))
    {
      WorkbenchPlugin.log(NLS.bind(
              WorkbenchMessages.PartStack_incorrectPartInFolder, part
              .getID()));
      return;
    }

    PartPane pane = (PartPane)part;

    PresentablePart presentablePart = new PresentablePart(pane, getControl().getParent());
    presentableParts.add(presentablePart);

    if (isActive)
    {
      part.setContainer(this);

      // The active part should always be enabled
      if (part.getControl() != null)
      part.getControl().setEnabled(true);
    }

    presentationSite.getPresentation().addPart(presentablePart, cookie);

    if (requestedCurrent == null)
    {
      setSelection(part);
    }

    if (childObscuredByZoom(part))
    {
      presentablePart.enableInputs(false);
    }
  }

  void PartStack::UpdateContainerVisibleTab()
  {
    LayoutPart[] parts = getChildren();

    if (parts.length < 1)
    {
      setSelection(null);
      return;
    }

    PartPane selPart = null;
    int topIndex = 0;
    WorkbenchPage page = getPage();

    if (page != null)
    {
      IWorkbenchPartReference sortedPartsArray[] = page.getSortedParts();
      List sortedParts = Arrays.asList(sortedPartsArray);
      for (int i = 0; i < parts.length; i++)
      {
        if (parts[i] instanceof PartPane)
        {
          IWorkbenchPartReference part = ((PartPane) parts[i])
          .getPartReference();
          int index = sortedParts.indexOf(part);
          if (index >= topIndex)
          {
            topIndex = index;
            selPart = (PartPane) parts[i];
          }
        }
      }

    }

    if (selPart == null)
    {
      List presentableParts = getPresentableParts();
      if (presentableParts.size() != 0)
      {
        IPresentablePart part = (IPresentablePart) getPresentableParts()
        .get(0);

        selPart = (PartPane) getPaneFor(part);
      }
    }

    setSelection(selPart);
  }

  void PartStack::ShowSystemMenu()
  {
    getPresentation().showSystemMenu();
  }

  void PartStack::ShowPaneMenu()
  {
    getPresentation().showPaneMenu();
  }

  void PartStack::ShowPartList()
  {
    getPresentation().showPartList();
  }

  std::vector<void*> PartStack::GetTabList(LayoutPart::Pointer part)
  {
    if (part != null)
    {
      IPresentablePart presentablePart = getPresentablePart(part);
      StackPresentation presentation = getPresentation();

      if (presentablePart != null && presentation != null)
      {
        return presentation.getTabList(presentablePart);
      }
    }

    return new Control[0];
  }

  IMemento::Pointer PartStack::GetSavedPresentationState()
  {
    return savedPresentationState;
  }

  void PartStack::FireInternalPropertyChange(int id)
  {
    Object listeners[] = this.listeners.getListeners();
    for (int i = 0; i < listeners.length; i++)
    {
      ((IPropertyListener) listeners[i]).propertyChanged(this, id);
    }
  }

  std::string PartStack::GetProperty(const std::string& id)
  {
    return (String)properties.get(id);
  }

  void PartStack::SetProperty(const std::string& id, const std::string& value)
  {
    if (value==null)
    {
      properties.remove(id);
    }
    else
    {
      properties.put(id, value);
    }
  }

  void PartStack::CopyAppearanceProperties(PartStack::Pointer copyTo)
  {
    copyTo.appearance = this.appearance;
    if (!properties.isEmpty())
    {
      Set ids = properties.keySet();
      for (Iterator iterator = ids.iterator(); iterator.hasNext();)
      {
        String id = (String)iterator.next();
        copyTo.setProperty(id, (String)properties.get(id));
      }
    }
  }

}
