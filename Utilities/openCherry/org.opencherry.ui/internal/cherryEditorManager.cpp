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

#include "cherryEditorManager.h"

namespace cherry
{

const std::string EditorManager::PIN_EDITOR_KEY = "PIN_EDITOR";
const std::string EditorManager::RESOURCES_TO_SAVE_MESSAGE = "Select resources to save:";
const std::string EditorManager::SAVE_RESOURCES_TITLE = "Save Resources";

EditorManager::EditorManager(WorkbenchWindow::Pointer window,
    WorkbenchPage::Pointer workbenchPage)
{
  Assert.isNotNull(window);
  Assert.isNotNull(workbenchPage);
  Assert.isNotNull(pres);
  this.window = window;
  this.page = workbenchPage;
  this.editorPresentation = pres;

  page.getExtensionTracker().registerHandler(this, null);
}

void EditorManager::CheckDeleteEditorResources()
{
  // get the current number of editors
  IEditorReference[] editors = page.getEditorReferences();
  // If there are no editors
  if (editors.length == 0)
  {
    if (editorPropChangeListnener != null)
    {
      // remove property change listener for editors
      IPreferenceStore prefStore = WorkbenchPlugin.getDefault()
      .getPreferenceStore();
      prefStore
      .removePropertyChangeListener(editorPropChangeListnener);
      editorPropChangeListnener = null;
    }
    if (pinEditorHandlerActivation != null)
    {
      // remove pin editor keyboard shortcut handler
      final IHandlerService handlerService = (IHandlerService) window.getWorkbench().getService(IHandlerService.class);
      handlerService.deactivateHandler(pinEditorHandlerActivation);
      pinEditorHandlerActivation = null;
    }
  }
}

void EditorManager::CheckCreateEditorPropListener()
{
  if (editorPropChangeListnener == null)
  {
    // Add a property change listener for closing editors automatically
    // preference
    // Add or remove the pin icon accordingly
    editorPropChangeListnener = new IPropertyChangeListener()
    {
    public void propertyChange(PropertyChangeEvent event)
      {
        if (event.getProperty().equals(
                IPreferenceConstants.REUSE_EDITORS_BOOLEAN))
        {
          IEditorReference[] editors = getEditors();
          for (int i = 0; i < editors.length; i++)
          {
            ((EditorReference) editors[i]).pinStatusUpdated();
          }
        }
      }
    };
    WorkbenchPlugin.getDefault().getPreferenceStore()
    .addPropertyChangeListener(editorPropChangeListnener);
  }
}

void EditorManager::CheckCreatePinEditorShortcutKeyHandler()
{
  if (pinEditorHandlerActivation == null)
  {
    final Shell shell = window.getShell();
    final IHandler pinEditorHandler = new AbstractHandler()
    {
    public final Object execute(final ExecutionEvent event)
      {
        // check if the "Close editors automatically" preference is
        // set
        IPreferenceStore store = WorkbenchPlugin.getDefault().getPreferenceStore();
        if (store
            .getBoolean(IPreferenceConstants.REUSE_EDITORS_BOOLEAN)
            || ((TabBehaviour)Tweaklets.get(TabBehaviour.KEY)).alwaysShowPinAction())
        {

          IWorkbenchPartReference ref = editorPresentation
          .getVisibleEditor();
          if (ref instanceof WorkbenchPartReference)
          {
            WorkbenchPartReference concreteRef = (WorkbenchPartReference) ref;

            concreteRef.setPinned(concreteRef.isPinned());
          }
        }
        return null;
      }
    };

    // Assign the handler for the pin editor keyboard shortcut.
    final IHandlerService handlerService = (IHandlerService) window.getWorkbench().getService(IHandlerService.class);
    pinEditorHandlerActivation = handlerService.activateHandler(
        "org.eclipse.ui.window.pinEditor", pinEditorHandler, //$NON-NLS-1$
        new ActiveShellExpression(shell));
  }
}

std::vector<IEditorPart::Pointer> EditorManager::CollectDirtyEditors()
{
  List result = new ArrayList(3);
  IEditorReference[] editors = page.getEditorReferences();
  for (int i = 0; i < editors.length; i++)
  {
    IEditorPart part = (IEditorPart) editors[i].getPart(false);
    if (part != null && part.isDirty())
    {
      result.add(part);
    }

  }
  return result;
}

bool EditorManager::ContainsEditor(IEditorReference::Pointer ref)
{
  IEditorReference[] editors = page.getEditorReferences();
  for (int i = 0; i < editors.length; i++)
  {
    if (ref == editors[i])
    {
      return true;
    }
  }
  return false;
}

EditorActionBars* EditorManager::CreateEditorActionBars(
    EditorDescriptor::Pointer desc, IEditorSite::Pointer site)
{
  // Get the editor type.
  String type = desc.getId();

  // If an action bar already exists for this editor type return it.
  EditorActionBars actionBars = (EditorActionBars) actionCache.get(type);
  if (actionBars != null)
  {
    actionBars.addRef();
    return actionBars;
  }

  // Create a new action bar set.
  actionBars = new EditorActionBars(page, site.getWorkbenchWindow(), type);
  actionBars.addRef();
  actionCache.put(type, actionBars);

  // Read base contributor.
  IEditorActionBarContributor contr = desc.createActionBarContributor();
  if (contr != null)
  {
    actionBars.setEditorContributor(contr);
    contr.init(actionBars, page);
  }

  // Read action extensions.
  EditorActionBuilder builder = new EditorActionBuilder();
  contr = builder.readActionExtensions(desc);
  if (contr != null)
  {
    actionBars.setExtensionContributor(contr);
    contr.init(actionBars, page);
  }

  // Return action bars.
  return actionBars;
}

EditorActionBars* EditorManager::CreateEmptyEditorActionBars(
    IEditorSite::Pointer site)
{
  // Get the editor type.
  String type = String.valueOf(System.currentTimeMillis());

  // Create a new action bar set.
  // Note: It is an empty set.
  EditorActionBars actionBars = new EditorActionBars(page, site.getWorkbenchWindow(), type);
  actionBars.addRef();
  actionCache.put(type, actionBars);

  // Return action bars.
  return actionBars;
}

void EditorManager::DisposeEditorActionBars(EditorActionBars* actionBars)
{
  actionBars.removeRef();
  if (actionBars.getRef() <= 0)
  {
    String type = actionBars.getEditorType();
    actionCache.remove(type);
    // refresh the cool bar manager before disposing of a cool item
    ICoolBarManager2 coolBar = (ICoolBarManager2) window.getCoolBarManager2();
    if (coolBar != null)
    {
      coolBar.refresh();
    }
    actionBars.dispose();
  }
}

IEditorPart::Pointer EditorManager::FindEditor(IEditorInput::Pointer input)
{
  return findEditor(null, input, IWorkbenchPage.MATCH_INPUT);
}

IEditorPart::Pointer EditorManager::FindEditor(const std::string& editorId,
    IEditorInput::Pointer input, int matchFlags)
{
  IEditorReference[] refs = findEditors(input, editorId, matchFlags);
  if (refs.length == 0)
  {
    return null;
  }
  return refs[0].getEditor(true);
}

std::vector<IEditorReference::Pointer> EditorManager::FindEditors(
    IEditorInput::Pointer input, const std::string& editorId, int matchFlags)
{
  if (matchFlags == IWorkbenchPage.MATCH_NONE)
  {
    return new IEditorReference[0];
  }
  List result = new ArrayList();
  ArrayList othersList = new ArrayList(Arrays.asList(page
          .getEditorReferences()));
  if (!othersList.isEmpty())
  {
    IEditorReference active = page.getActiveEditorReference();
    if (active != null)
    {
      othersList.remove(active);
      ArrayList activeList = new ArrayList(1);
      activeList.add(active);
      findEditors(activeList, input, editorId, matchFlags, result);
    }
    findEditors(othersList, input, editorId, matchFlags, result);
  }
  return (IEditorReference[]) result.toArray(new IEditorReference[result
  .size()]);
}

void EditorManager::FindEditors(
    std::vector<IEditorReference::Pointer>& editorList,
    IEditorInput::Pointer input, const std::string& editorId, int matchFlags,
    std::vector<IEditorReference::Pointer>& result)
{
  if (matchFlags == IWorkbenchPage.MATCH_NONE)
  {
    return;
  }

  // Phase 0: Remove editors whose ids don't match (if matching by id)
  if (((matchFlags & IWorkbenchPage.MATCH_ID) != 0) && editorId != null)
  {
    for (Iterator i = editorList.iterator(); i.hasNext();)
    {
      EditorReference editor = (EditorReference) i.next();
      if (!editorId.equals(editor.getId()))
      {
        i.remove();
      }
    }
  }

  // If not matching on editor input, just return the remaining editors.
  // In practice, this case is never used.
  if ((matchFlags & IWorkbenchPage.MATCH_INPUT) == 0)
  {
    result.addAll(editorList);
    return;
  }

  // Phase 1: check editors that have their own matching strategy
  for (Iterator i = editorList.iterator(); i.hasNext();)
  {
    EditorReference editor = (EditorReference) i.next();
    IEditorDescriptor desc = editor.getDescriptor();
    if (desc != null)
    {
      IEditorMatchingStrategy matchingStrategy = desc
      .getEditorMatchingStrategy();
      if (matchingStrategy != null)
      {
        i.remove(); // We're handling this one here, so remove it
        // from the list.
        if (matchingStrategy.matches(editor, input))
        {
          result.add(editor);
        }
      }
    }
  }

  // Phase 2: check materialized editors (without their own matching
  // strategy)
  for (Iterator i = editorList.iterator(); i.hasNext();)
  {
    IEditorReference editor = (IEditorReference) i.next();
    IEditorPart part = (IEditorPart) editor.getPart(false);
    if (part != null)
    {
      i.remove(); // We're handling this one here, so remove it from
      // the list.
      if (part.getEditorInput() != null && part.getEditorInput().equals(input))
      {
        result.add(editor);
      }
    }
  }

  // Phase 3: check unmaterialized editors for input equality,
  // delaying plug-in activation further by only restoring the editor
  // input
  // if the editor reference's factory id and name match.
  String name = input.getName();
  IPersistableElement persistable = input.getPersistable();
  if (name == null || persistable == null)
  {
    return;
  }
  String id = persistable.getFactoryId();
  if (id == null)
  {
    return;
  }
  for (Iterator i = editorList.iterator(); i.hasNext();)
  {
    EditorReference editor = (EditorReference) i.next();
    if (name.equals(editor.getName()) && id.equals(editor.getFactoryId()))
    {
      IEditorInput restoredInput;
      try
      {
        restoredInput = editor.getEditorInput();
        if (Util.equals(restoredInput, input))
        {
          result.add(editor);
        }
      }
      catch (PartInitException e1)
      {
        WorkbenchPlugin.log(e1);
      }
    }
  }
}

int EditorManager::GetEditorCount()
{
  return page.getEditorReferences().length;
}

IEditorRegistry* EditorManager::GetEditorRegistry()
{
  return WorkbenchPlugin.getDefault().getEditorRegistry();
}

std::vector<IEditorPart::Pointer> EditorManager::GetDirtyEditors()
{
  List dirtyEditors = collectDirtyEditors();
  return (IEditorPart[]) dirtyEditors
  .toArray(new IEditorPart[dirtyEditors.size()]);
}

std::vector<IEditorReference::Pointer> EditorManager::GetEditors()
{
  return page.getEditorReferences();
}

IEditorPart::Pointer EditorManager::GetVisibleEditor()
{
  IEditorReference ref = editorPresentation.getVisibleEditor();
  if (ref == null)
  {
    return null;
  }
  return (IEditorPart) ref.getPart(true);
}

bool EditorManager::IsSaveAllNeeded()
{
  IEditorReference[] editors = page.getEditorReferences();
  for (int i = 0; i < editors.length; i++)
  {
    IEditorReference ed = editors[i];
    if (ed.isDirty())
    {
      return true;
    }
  }
  return false;
}

IEditorReference::Pointer EditorManager::FindReusableEditor(
    EditorDescriptor::Pointer desc)
{
  return ((TabBehaviour)Tweaklets.get(TabBehaviour.KEY)).findReusableEditor(page);
}

IEditorReference::Pointer EditorManager::OpenEditor(
    const std::string& editorId, IEditorInput::Pointer input, bool setVisible,
    IMemento* editorState)
{
  if (editorId == null || input == null)
  {
    throw new IllegalArgumentException();
  }

  IEditorRegistry reg = getEditorRegistry();
  EditorDescriptor desc = (EditorDescriptor) reg.findEditor(editorId);
  if (desc == null)
  {
    throw new PartInitException(NLS.bind(
            WorkbenchMessages.EditorManager_unknownEditorIDMessage,
            editorId));
  }

  IEditorReference result = openEditorFromDescriptor(desc, input, editorState);
  return result;
}

IEditorReference::Pointer EditorManager::OpenEditorFromDescriptor(
    EditorDescriptor::Pointer desc, IEditorInput::Pointer input,
    IMemento* editorState)
{
  IEditorReference result = null;
  if (desc.isInternal())
  {
    result = reuseInternalEditor(desc, input);
    if (result == null)
    {
      result = new EditorReference(this, input, desc, editorState);
    }
  }
  else if (desc.getId()
  .equals(IEditorRegistry.SYSTEM_INPLACE_EDITOR_ID))
  {
    if (ComponentSupport.inPlaceEditorSupported())
    {
      result = new EditorReference(this, input, desc);
    }
  }
  else if (desc.getId().equals(IEditorRegistry.SYSTEM_EXTERNAL_EDITOR_ID))
  {
    IPathEditorInput pathInput = getPathEditorInput(input);
    if (pathInput != null)
    {
      result = openSystemExternalEditor(pathInput.getPath());
    }
    else
    {
      throw new PartInitException(
          WorkbenchMessages.EditorManager_systemEditorError);
    }
  }
  else if (desc.isOpenExternal())
  {
    result = openExternalEditor(desc, input);
  }
  else
  {
    // this should never happen
    throw new PartInitException(NLS.bind(
            WorkbenchMessages.EditorManager_invalidDescriptor, desc
            .getId()));
  }

  if (result != null)
  {
    createEditorTab((EditorReference) result, ""); //$NON-NLS-1$
  }

  Workbench wb = (Workbench) window.getWorkbench();
  wb.getEditorHistory().add(input, desc);
  return result;
}

IEditorReference::Pointer EditorManager::OpenExternalEditor(
    EditorDescriptor::Pointer desc, IEditorInput::Pointer input)
{
  final CoreException ex[] = new CoreException[1];

  final IPathEditorInput pathInput = getPathEditorInput(input);
  if (pathInput != null && pathInput.getPath() != null)
  {
    BusyIndicator.showWhile(getDisplay(), new Runnable()
        {
        public void run()
          {
            try
            {
              if (desc.getLauncher() != null)
              {
                // open using launcher
                Object launcher = WorkbenchPlugin.createExtension(
                    desc.getConfigurationElement(), "launcher"); //$NON-NLS-1$
                ((IEditorLauncher) launcher).open(pathInput
                    .getPath());
              }
              else
              {
                // open using command
                ExternalEditor oEditor = new ExternalEditor(
                    pathInput.getPath(), desc);
                oEditor.open();
              }
            }
            catch (CoreException e)
            {
              ex[0] = e;
            }
          }
        }
        );
      }
  else
  {
    throw new PartInitException(NLS.bind(
            WorkbenchMessages.EditorManager_errorOpeningExternalEditor,
            desc.getFileName(), desc.getId()));
  }

  if (ex[0] != null)
  {
    throw new PartInitException(NLS.bind(
            WorkbenchMessages.EditorManager_errorOpeningExternalEditor,
            desc.getFileName(), desc.getId()), ex[0]);
  }

  // we do not have an editor part for external editors
  return null;
}

EditorSite::Pointer EditorManager::CreateSite(IEditorReference::Pointer ref,
    IEditorPart::Pointer part, EditorDescriptor::Pointer desc,
    IEditorInput::Pointer input)
{
  EditorSite site = new EditorSite(ref, part, page, desc);
  if (desc != null)
  {
    site.setActionBars(createEditorActionBars(desc, site));
  }
  else
  {
    site.setActionBars(createEmptyEditorActionBars(site));
  }
  final String label = part.getTitle(); // debugging only
  try
  {
    try
    {
      UIStats.start(UIStats.INIT_PART, label);
      part.init(site, input);
    }finally
    {
      UIStats.end(UIStats.INIT_PART, part, label);
    }

    // Sanity-check the site
    if (part.getSite() != site || part.getEditorSite() != site)
    {
      throw new PartInitException(NLS.bind(
              WorkbenchMessages.EditorManager_siteIncorrect, desc
              .getId()));
    }

  }
  catch (Exception e)
  {
    disposeEditorActionBars((EditorActionBars) site.getActionBars());
    site.dispose();
    if (e instanceof PartInitException)
    {
      throw (PartInitException) e;
    }

    throw new PartInitException(
        WorkbenchMessages.EditorManager_errorInInit, e);
  }

  return site;
}

IEditorReference::Pointer EditorManager::ReuseInternalEditor(
    EditorDescriptor::Pointer desc, IEditorInput::Pointer input)
{

  Assert.isNotNull(desc, "descriptor must not be null"); //$NON-NLS-1$
  Assert.isNotNull(input, "input must not be null"); //$NON-NLS-1$

  IEditorReference reusableEditorRef = findReusableEditor(desc);
  if (reusableEditorRef != null)
  {
    return ((TabBehaviour) Tweaklets.get(TabBehaviour.KEY))
    .reuseInternalEditor(page, this, editorPresentation, desc, input,
        reusableEditorRef);
  }
  return null;
}

IEditorPart::Pointer EditorManager::CreatePart(EditorDescriptor::Pointer desc)
{
  try
  {
    IEditorPart result = desc.createEditor();
    IConfigurationElement element = desc.getConfigurationElement();
    if (element != null)
    {
      page.getExtensionTracker().registerObject(
          element.getDeclaringExtension(), result,
          IExtensionTracker.REF_WEAK);
    }
    return result;
  }
  catch (CoreException e)
  {
    throw new PartInitException(StatusUtil.newStatus(
            desc.getPluginID(),
            WorkbenchMessages.EditorManager_instantiationError, e));
  }
}

IEditorReference::Pointer EditorManager::OpenSystemExternalEditor(
    Poco::Path location)
{
  if (location == null)
  {
    throw new IllegalArgumentException();
  }

  final boolean result[] =
  { false};
  BusyIndicator.showWhile(getDisplay(), new Runnable()
      {
      public void run()
        {
          if (location != null)
          {
            result[0] = Program.launch(location.toOSString());
          }
        }
      }
      );

      if (!result[0])
      {
        throw new PartInitException(NLS.bind(
                WorkbenchMessages.EditorManager_unableToOpenExternalEditor,
                location));
      }

      // We do not have an editor part for external editors
      return null;
    }

    ImageDescriptor EditorManager::FindImage(EditorDescriptor::Pointer desc,
        Poco::Path path)
    {
      if (desc == null)
      {
        // @issue what should be the default image?
        return ImageDescriptor.getMissingImageDescriptor();
      }

      if (desc.isOpenExternal() && path != null)
      {
        return PlatformUI.getWorkbench().getEditorRegistry()
        .getImageDescriptor(path.toOSString());
      }

      return desc.getImageDescriptor();
    }

    IStatus EditorManager::RestoreState(IMemento* memento)
    {
      // Restore the editor area workbooks layout/relationship
      final MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID,
          IStatus.OK,
          WorkbenchMessages.EditorManager_problemsRestoringEditors, null);
      final String activeWorkbookID[] = new String[1];
      final ArrayList visibleEditors = new ArrayList(5);
      final IEditorReference activeEditor[] = new IEditorReference[1];

      IMemento areaMem = memento.getChild(IWorkbenchConstants.TAG_AREA);
      if (areaMem != null)
      {
        result.add(editorPresentation.restoreState(areaMem));
        activeWorkbookID[0] = areaMem
        .getString(IWorkbenchConstants.TAG_ACTIVE_WORKBOOK);
      }

      // Loop through the editors.

      IMemento[] editorMems = memento
      .getChildren(IWorkbenchConstants.TAG_EDITOR);
      for (int x = 0; x < editorMems.length; x++)
      {
        // for dynamic UI - call restoreEditorState to replace code which is
        // commented out
        restoreEditorState(editorMems[x], visibleEditors, activeEditor, result);
      }

      // restore the presentation
      if (areaMem != null)
      {
        result.add(editorPresentation.restorePresentationState(areaMem));
      }
      try
      {
        StartupThreading.runWithThrowable(new StartupRunnable()
            {

            public void runWithException() throws Throwable
              {
                // Update each workbook with its visible editor.
                for (int i = 0; i < visibleEditors.size(); i++)
                {
                  setVisibleEditor((IEditorReference) visibleEditors.get(i),
                      false);
                }

                // Update the active workbook
                if (activeWorkbookID[0] != null)
                {
                  editorPresentation
                  .setActiveEditorWorkbookFromID(activeWorkbookID[0]);
                }

                if (activeEditor[0] != null)
                {
                  IWorkbenchPart editor = activeEditor[0].getPart(true);

                  if (editor != null)
                  {
                    page.activate(editor);
                  }
                }
              }});
      }
      catch (Throwable t)
      {
        // The exception is already logged.
        result
        .add(new Status(
                IStatus.ERROR,
                PlatformUI.PLUGIN_ID,
                0,
                WorkbenchMessages.EditorManager_exceptionRestoringEditor,
                t));
      }

      return result;
    }

    bool EditorManager::SaveAll(bool confirm, bool closing,
        bool addNonPartSources)
    {
      // Get the list of dirty editors and views. If it is
      // empty just return.
      ISaveablePart[] parts = page.getDirtyParts();
      if (parts.length == 0)
      {
        return true;
      }
      // saveAll below expects a mutable list
      List dirtyParts = new ArrayList(parts.length);
      for (int i = 0; i < parts.length; i++)
      {
        dirtyParts.add(parts[i]);
      }

      // If confirmation is required ..
      return saveAll(dirtyParts, confirm, closing, addNonPartSources, window);
    }

    bool EditorManager::SaveAll(
        const std::vector<IWorkbenchPart::Pointer>& dirtyParts, bool confirm,
        bool closing, bool addNonPartSources, IWorkbenchWindow::Pointer window)
    {
      return saveAll(dirtyParts, confirm, closing, addNonPartSources, window,
          window);
    }

    bool EditorManager::SavePart(ISaveablePart::Pointer saveable,
        IWorkbenchPart::Pointer part, bool confirm)
    {
      return SaveableHelper.savePart(saveable, part, window, confirm);
    }

    IStatus EditorManager::SaveState(const IMemento* memento)
    {

      final MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID,
          IStatus.OK,
          WorkbenchMessages.EditorManager_problemsSavingEditors, null);

      // Save the editor area workbooks layout/relationship
      IMemento editorAreaMem = memento
      .createChild(IWorkbenchConstants.TAG_AREA);
      result.add(editorPresentation.saveState(editorAreaMem));

      // Save the active workbook id
      editorAreaMem.putString(IWorkbenchConstants.TAG_ACTIVE_WORKBOOK,
          editorPresentation.getActiveEditorWorkbookID());

      // Get each workbook
      ArrayList workbooks = editorPresentation.getWorkbooks();

      for (Iterator iter = workbooks.iterator(); iter.hasNext();)
      {
        EditorStack workbook = (EditorStack) iter.next();

        // Use the list of editors found in EditorStack; fix for 24091
        EditorPane editorPanes[] = workbook.getEditors();

        for (int i = 0; i < editorPanes.length; i++)
        {
          // Save each open editor.
          IEditorReference editorReference = editorPanes[i]
          .getEditorReference();
          EditorReference e = (EditorReference) editorReference;
          final IEditorPart editor = editorReference.getEditor(false);
          if (editor == null)
          {
            if (e.getMemento() != null)
            {
              IMemento editorMem = memento
              .createChild(IWorkbenchConstants.TAG_EDITOR);
              editorMem.putMemento(e.getMemento());
            }
            continue;
          }

          // for dynamic UI - add the next line to replace the subsequent
          // code which is commented out
          saveEditorState(memento, e, result);
        }
      }
      return result;
    }

    bool EditorManager::SetVisibleEditor(IEditorReference::Pointer newEd,
        bool setFocus)
    {
      return editorPresentation.setVisibleEditor(newEd, setFocus);
    }

    IPathEditorInput::Pointer EditorManager::GetPathEditorInput(
        IEditorInput::Pointer input)
    {
      if (input instanceof IPathEditorInput)
      {
        return (IPathEditorInput) input;
      }

      return (IPathEditorInput)
      Util.getAdapter(input, IPathEditorInput.class);
    }

    void EditorManager::RestoreEditorState(IMemento* editorMem,
        std::vector<IEditorReference::Pointer>& visibleEditors,
        std::vector<IEditorReference::Pointer>& activeEditor)
    {
      // MultiStatus result) {
      // String strFocus = editorMem.getString(IWorkbenchConstants.TAG_FOCUS);
      // boolean visibleEditor = "true".equals(strFocus); //$NON-NLS-1$
      final EditorReference e = new EditorReference(this, editorMem);

      // if the editor is not visible, ensure it is put in the correct
      // workbook. PR 24091

      final String workbookID = editorMem
      .getString(IWorkbenchConstants.TAG_WORKBOOK);

      try
      {
        StartupThreading.runWithPartInitExceptions(new StartupRunnable ()
            {

            public void runWithException() throws Throwable
              {
                createEditorTab(e, workbookID);
              }});

      }
      catch (PartInitException ex)
      {
        result.add(ex.getStatus());
      }

      String strActivePart = editorMem
      .getString(IWorkbenchConstants.TAG_ACTIVE_PART);
      if ("true".equals(strActivePart))
      { //$NON-NLS-1$
        activeEditor[0] = e;
      }

      String strFocus = editorMem.getString(IWorkbenchConstants.TAG_FOCUS);
      boolean visibleEditor = "true".equals(strFocus); //$NON-NLS-1$
      if (visibleEditor)
      {
        visibleEditors.add(e);
      }
    }

    void EditorManager::SaveEditorState(IMemento* mem,
        IEditorReference::Pointer ed, MultiStatus res)
    {
      final EditorReference editorRef = (EditorReference) ed;
      final IEditorPart editor = ed.getEditor(false);
      final IMemento memento = mem;
      final MultiStatus result = res;
      if (!(editor.getEditorSite() instanceof EditorSite))
      {
        return;
      }
      final EditorSite site = (EditorSite) editor.getEditorSite();
      if (site.getPane() instanceof MultiEditorInnerPane)
      {
        return;
      }

      SafeRunner.run(new SafeRunnable()
          {
          public void run()
            {
              // Get the input.
              IEditorInput input = editor.getEditorInput();
              if (!input.exists())
              {
                return;
              }
              IPersistableElement persistable = input.getPersistable();
              if (persistable == null)
              {
                return;
              }

              // Save editor.
              IMemento editorMem = memento
              .createChild(IWorkbenchConstants.TAG_EDITOR);
              editorMem.putString(IWorkbenchConstants.TAG_TITLE, editorRef
                  .getTitle());
              editorMem.putString(IWorkbenchConstants.TAG_NAME, editorRef
                  .getName());
              editorMem.putString(IWorkbenchConstants.TAG_ID, editorRef
                  .getId());
              editorMem.putString(IWorkbenchConstants.TAG_TOOLTIP, editorRef
                  .getTitleToolTip());

              editorMem.putString(IWorkbenchConstants.TAG_PART_NAME,
                  editorRef.getPartName());

              if (editor instanceof IWorkbenchPart3)
              {
                Map properties = ((IWorkbenchPart3) editor)
                .getPartProperties();
                if (!properties.isEmpty())
                {
                  IMemento propBag = editorMem
                  .createChild(IWorkbenchConstants.TAG_PROPERTIES);
                  Iterator i = properties.entrySet().iterator();
                  while (i.hasNext())
                  {
                    Map.Entry entry = (Map.Entry) i.next();
                    IMemento p = propBag.createChild(
                        IWorkbenchConstants.TAG_PROPERTY,
                        (String) entry.getKey());
                    p.putTextData((String) entry.getValue());
                  }
                }
              }

              if (editorRef.isPinned())
              {
                editorMem.putString(IWorkbenchConstants.TAG_PINNED, "true"); //$NON-NLS-1$
              }

              EditorPane editorPane = (EditorPane) ((EditorSite) editor
                  .getEditorSite()).getPane();
              editorMem.putString(IWorkbenchConstants.TAG_WORKBOOK,
                  editorPane.getWorkbook().getID());

              if (editor == page.getActivePart())
              {
                editorMem.putString(IWorkbenchConstants.TAG_ACTIVE_PART,
                    "true"); //$NON-NLS-1$
              }

              if (editorPane == editorPane.getWorkbook().getSelection())
              {
                editorMem.putString(IWorkbenchConstants.TAG_FOCUS, "true"); //$NON-NLS-1$
              }

              if (input instanceof IPathEditorInput)
              {
                IPath path = ((IPathEditorInput) input).getPath();
                if (path != null)
                {
                  editorMem.putString(IWorkbenchConstants.TAG_PATH, path
                      .toString());
                }
              }

              // Save input.
              IMemento inputMem = editorMem
              .createChild(IWorkbenchConstants.TAG_INPUT);
              inputMem.putString(IWorkbenchConstants.TAG_FACTORY_ID,
                  persistable.getFactoryId());
              persistable.saveState(inputMem);

              // any editors that want to persist state
              if (editor instanceof IPersistableEditor)
              {
                IMemento editorState = editorMem
                .createChild(IWorkbenchConstants.TAG_EDITOR_STATE);
                ((IPersistableEditor) editor).saveState(editorState);
              }
            }

          public void handleException(Throwable e)
            {
              result
              .add(new Status(
                      IStatus.ERROR,
                      PlatformUI.PLUGIN_ID,
                      0,
                      NLS
                      .bind(
                          WorkbenchMessages.EditorManager_unableToSaveEditor,
                          editorRef.getTitle()), e));
            }
          }
          );
        }

    IMemento* EditorManager::GetMemento(IEditorReference::Pointer e)
    {
      if (e instanceof EditorReference)
      {
        return ((EditorReference) e).getMemento();
      }
      return null;
    }

    IEditorReference::Pointer EditorManager::OpenEmptyTab()
    {
      IEditorInput input = new NullEditorInput();
      EditorDescriptor desc = (EditorDescriptor) ((EditorRegistry) getEditorRegistry())
      .findEditor(EditorRegistry.EMPTY_EDITOR_ID);
      EditorReference result = new EditorReference(this, input, desc);
      try
      {
        createEditorTab(result, ""); //$NON-NLS-1$
        return result;
      }
      catch (PartInitException e)
      {
        StatusManager.getManager().handle(
            StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH, e));
      }
      return null;
    }

    bool EditorManager::UseIPersistableEditor()
    {
      IPreferenceStore store = WorkbenchPlugin.getDefault()
      .getPreferenceStore();
      return store.getBoolean(IPreferenceConstants.USE_IPERSISTABLE_EDITORS);
    }

    }
