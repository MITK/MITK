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

#include "berryEditorManager.h"

#include "berryIWorkbenchPart.h"
#include "berryIWorkbenchWindow.h"
#include "berryIEditorRegistry.h"
#include "berryUIException.h"

#include "berryWorkbenchWindow.h"
#include "berryWorkbenchPage.h"
#include "berryEditorSite.h"
#include "berryEditorReference.h"
#include "berryWorkbenchPlugin.h"
#include "berryWorkbenchConstants.h"
#include "berryNullEditorInput.h"
#include "berryEditorAreaHelper.h"
#include "berryPartStack.h"


#include <Poco/Bugcheck.h>

namespace berry
{

const QString EditorManager::PIN_EDITOR_KEY = "PIN_EDITOR";
const QString EditorManager::RESOURCES_TO_SAVE_MESSAGE = "Select resources to save:";
const QString EditorManager::SAVE_RESOURCES_TITLE = "Save Resources";

EditorManager::EditorManager(WorkbenchWindow::Pointer wind,
    WorkbenchPage::Pointer workbenchPage,
    EditorAreaHelper* pres)
 : editorPresentation(pres), window(wind.GetPointer()), page(workbenchPage.GetPointer()) {
  poco_check_ptr(editorPresentation);
  poco_assert(window != 0);
  poco_assert(page != 0);

  //page.getExtensionTracker().registerHandler(this, null);
}

void EditorManager::CheckDeleteEditorResources()
{
//  // get the current number of editors
//  IEditorReference[] editors = page.getEditorReferences();
//  // If there are no editors
//  if (editors.length == 0)
//  {
//    if (editorPropChangeListnener != null)
//    {
//      // remove property change listener for editors
//      IPreferenceStore prefStore = WorkbenchPlugin.getDefault()
//      .getPreferenceStore();
//      prefStore
//      .removePropertyChangeListener(editorPropChangeListnener);
//      editorPropChangeListnener = null;
//    }
//    if (pinEditorHandlerActivation != null)
//    {
//      // remove pin editor keyboard shortcut handler
//      final IHandlerService handlerService = (IHandlerService) window.getWorkbench().getService(IHandlerService.class);
//      handlerService.deactivateHandler(pinEditorHandlerActivation);
//      pinEditorHandlerActivation = null;
//    }
//  }
}

//void EditorManager::CheckCreateEditorPropListener()
//{
//  if (editorPropChangeListnener == null)
//  {
//    // Add a property change listener for closing editors automatically
//    // preference
//    // Add or remove the pin icon accordingly
//    editorPropChangeListnener = new IPropertyChangeListener()
//    {
//    public void propertyChange(PropertyChangeEvent event)
//      {
//        if (event.getProperty().equals(
//                IPreferenceConstants.REUSE_EDITORS_BOOLEAN))
//        {
//          IEditorReference[] editors = getEditors();
//          for (int i = 0; i < editors.length; i++)
//          {
//            ((EditorReference) editors[i]).pinStatusUpdated();
//          }
//        }
//      }
//    };
//    WorkbenchPlugin.getDefault().getPreferenceStore()
//    .addPropertyChangeListener(editorPropChangeListnener);
//  }
//}

//void EditorManager::CheckCreatePinEditorShortcutKeyHandler()
//{
//  if (pinEditorHandlerActivation == null)
//  {
//    final Shell shell = window.getShell();
//    final IHandler pinEditorHandler = new AbstractHandler()
//    {
//    public final Object execute(final ExecutionEvent event)
//      {
//        // check if the "Close editors automatically" preference is
//        // set
//        IPreferenceStore store = WorkbenchPlugin.getDefault().getPreferenceStore();
//        if (store
//            .getBoolean(IPreferenceConstants.REUSE_EDITORS_BOOLEAN)
//            || ((TabBehaviour)Tweaklets.get(TabBehaviour.KEY)).alwaysShowPinAction())
//        {
//
//          IWorkbenchPartReference ref = editorPresentation
//          .getVisibleEditor();
//          if (ref instanceof WorkbenchPartReference)
//          {
//            WorkbenchPartReference concreteRef = (WorkbenchPartReference) ref;
//
//            concreteRef.setPinned(concreteRef.isPinned());
//          }
//        }
//        return null;
//      }
//    };
//
//    // Assign the handler for the pin editor keyboard shortcut.
//    final IHandlerService handlerService = (IHandlerService) window.getWorkbench().getService(IHandlerService.class);
//    pinEditorHandlerActivation = handlerService.activateHandler(
//        "org.blueberry.ui.window.pinEditor", pinEditorHandler, //$NON-NLS-1$
//        new ActiveShellExpression(shell));
//  }
//}

QList<IEditorPart::Pointer> EditorManager::CollectDirtyEditors()
{
  QList<IEditorPart::Pointer> result;
  QList<IEditorReference::Pointer> editors(page->GetEditorReferences());
  for (QList<IEditorReference::Pointer>::iterator i = editors.begin();
       i != editors.end(); ++i)
  {
    IEditorPart::Pointer part = (*i)->GetPart(false).Cast<IEditorPart>();
    if (part.IsNotNull() && part->IsDirty())
    {
      result.push_back(part);
    }

  }
  return result;
}

bool EditorManager::ContainsEditor(IEditorReference::Pointer ref)
{
  QList<IEditorReference::Pointer> editors(page->GetEditorReferences());
  return std::find(editors.begin(), editors.end(), ref) != editors.end();
}

//EditorActionBars* EditorManager::CreateEditorActionBars(
//    EditorDescriptor::Pointer desc, IEditorSite::Pointer site)
//{
//  // Get the editor type.
//  String type = desc.getId();
//
//  // If an action bar already exists for this editor type return it.
//  EditorActionBars actionBars = (EditorActionBars) actionCache.get(type);
//  if (actionBars != null)
//  {
//    actionBars.addRef();
//    return actionBars;
//  }
//
//  // Create a new action bar set.
//  actionBars = new EditorActionBars(page, site.getWorkbenchWindow(), type);
//  actionBars.addRef();
//  actionCache.put(type, actionBars);
//
//  // Read base contributor.
//  IEditorActionBarContributor contr = desc.createActionBarContributor();
//  if (contr != null)
//  {
//    actionBars.setEditorContributor(contr);
//    contr.init(actionBars, page);
//  }
//
//  // Read action extensions.
//  EditorActionBuilder builder = new EditorActionBuilder();
//  contr = builder.readActionExtensions(desc);
//  if (contr != null)
//  {
//    actionBars.setExtensionContributor(contr);
//    contr.init(actionBars, page);
//  }
//
//  // Return action bars.
//  return actionBars;
//}

//EditorActionBars* EditorManager::CreateEmptyEditorActionBars(
//    IEditorSite::Pointer site)
//{
//  // Get the editor type.
//  String type = String.valueOf(System.currentTimeMillis());
//
//  // Create a new action bar set.
//  // Note: It is an empty set.
//  EditorActionBars actionBars = new EditorActionBars(page, site.getWorkbenchWindow(), type);
//  actionBars.addRef();
//  actionCache.put(type, actionBars);
//
//  // Return action bars.
//  return actionBars;
//}

//void EditorManager::DisposeEditorActionBars(EditorActionBars* actionBars)
//{
//  actionBars.removeRef();
//  if (actionBars.getRef() <= 0)
//  {
//    String type = actionBars.getEditorType();
//    actionCache.remove(type);
//    // refresh the cool bar manager before disposing of a cool item
//    ICoolBarManager2 coolBar = (ICoolBarManager2) window.getCoolBarManager2();
//    if (coolBar != null)
//    {
//      coolBar.refresh();
//    }
//    actionBars.dispose();
//  }
//}

IEditorPart::Pointer EditorManager::FindEditor(IEditorInput::Pointer input)
{
  return this->FindEditor("", input, IWorkbenchPage::MATCH_INPUT);
}

IEditorPart::Pointer EditorManager::FindEditor(const QString& editorId,
    IEditorInput::Pointer input, int matchFlags)
{
  QList<IEditorReference::Pointer> refs(this->FindEditors(input, editorId, matchFlags));
  if (refs.size() == 0)
  {
    return IEditorPart::Pointer();
  }
  return refs[0]->GetEditor(true);
}

QList<IEditorReference::Pointer> EditorManager::FindEditors(
    IEditorInput::Pointer input, const QString& editorId, int matchFlags)
{
  if (matchFlags == IWorkbenchPage::MATCH_NONE)
  {
    return QList<IEditorReference::Pointer>();
  }
  QList<IEditorReference::Pointer> result;
  QList<IEditorReference::Pointer> othersList(page->GetEditorReferences());
  if (!othersList.empty())
  {
    IEditorReference::Pointer active = page->GetActiveEditorReference();
    if (active.IsNotNull())
    {
      othersList.removeAll(active);
      QList<IEditorReference::Pointer> activeList;
      activeList.push_back(active);
      this->FindEditors(activeList, input, editorId, matchFlags, result);
    }
    this->FindEditors(othersList, input, editorId, matchFlags, result);
  }
  return result;
}

void EditorManager::FindEditors(
    QList<IEditorReference::Pointer>& editorList,
    IEditorInput::Pointer input, const QString& editorId, int matchFlags,
    QList<IEditorReference::Pointer>& result)
{
  if (matchFlags == IWorkbenchPage::MATCH_NONE)
  {
    return;
  }

  // Phase 0: Remove editors whose ids don't match (if matching by id)
  if (((matchFlags & IWorkbenchPage::MATCH_ID) != 0) && !editorId.isEmpty())
  {
    for (QList<IEditorReference::Pointer>::iterator i = editorList.begin();
      i != editorList.end();)
    {
      if (editorId != (*i)->GetId())
      {
        i = editorList.erase(i);
        continue;
      }
      ++i;
    }
  }

  // If not matching on editor input, just return the remaining editors.
  // In practice, this case is never used.
  if ((matchFlags & IWorkbenchPage::MATCH_INPUT) == 0)
  {
    result.append(editorList);
    return;
  }

  // Phase 1: check editors that have their own matching strategy
  for (QList<IEditorReference::Pointer>::iterator i = editorList.begin();
       i != editorList.end();)
  {
    EditorReference::Pointer editor = i->Cast<EditorReference>();
    IEditorDescriptor::Pointer desc = editor->GetDescriptor();
    if (desc.IsNotNull())
    {
      IEditorMatchingStrategy::Pointer matchingStrategy = desc
      ->GetEditorMatchingStrategy();
      if (matchingStrategy.IsNotNull())
      {
        i = editorList.erase(i); // We're handling this one here, so remove it
                                 // from the list.
        if (matchingStrategy->Matches(editor, input))
        {
          result.push_back(editor);
        }
        continue;
      }
    }
    ++i;
  }

  // Phase 2: check materialized editors (without their own matching
  // strategy)
  for (QList<IEditorReference::Pointer>::iterator i = editorList.begin();
       i != editorList.end();)
  {
    EditorReference::Pointer editor = i->Cast<EditorReference>();
    IEditorPart::Pointer part = editor->GetPart(false).Cast<IEditorPart>();
    if (part.IsNotNull())
    {
      i = editorList.erase(i); // We're handling this one here, so remove it from
      // the list.
      if (part->GetEditorInput().IsNotNull() && part->GetEditorInput() == input)
      {
        result.push_back(editor);
      }
    }
    else ++i;
  }

  // Phase 3: check unmaterialized editors for input equality,
  // delaying plug-in activation further by only restoring the editor
  // input
  // if the editor reference's factory id and name match.

//  QString name = input->GetName();
//  IPersistableElement persistable = input.getPersistable();
//  if (name == null || persistable == null)
//  {
//    return;
//  }
//  String id = persistable.getFactoryId();
//  if (id == null)
//  {
//    return;
//  }
//  for (Iterator i = editorList.iterator(); i.hasNext();)
//  {
//    EditorReference editor = (EditorReference) i.next();
//    if (name.equals(editor.getName()) && id.equals(editor.getFactoryId()))
//    {
//      IEditorInput restoredInput;
//      try
//      {
//        restoredInput = editor.getEditorInput();
//        if (Util.equals(restoredInput, input))
//        {
//          result.add(editor);
//        }
//      }
//      catch (PartInitException e1)
//      {
//        WorkbenchPlugin.log(e1);
//      }
//    }
//  }
}

std::size_t EditorManager::GetEditorCount()
{
  return page->GetEditorReferences().size();
}

IEditorRegistry* EditorManager::GetEditorRegistry()
{
  return WorkbenchPlugin::GetDefault()->GetEditorRegistry();
}

QList<IEditorPart::Pointer> EditorManager::GetDirtyEditors()
{
  return this->CollectDirtyEditors();
}

QList<IEditorReference::Pointer> EditorManager::GetEditors()
{
  return page->GetEditorReferences();
}

IEditorPart::Pointer EditorManager::GetVisibleEditor()
{
  IEditorReference::Pointer ref = editorPresentation->GetVisibleEditor();
  if (ref.IsNull())
  {
    return IEditorPart::Pointer(0);
  }
  return ref->GetPart(true).Cast<IEditorPart>();
}

bool EditorManager::IsSaveAllNeeded()
{
  QList<IEditorReference::Pointer> editors(page->GetEditorReferences());
  for (QList<IEditorReference::Pointer>::iterator i = editors.begin();
       i != editors.end(); ++i)
  {
    if ((*i)->IsDirty())
    {
      return true;
    }
  }
  return false;
}

IEditorReference::Pointer EditorManager::FindReusableEditor(
    EditorDescriptor::Pointer  /*desc*/)
{
  //return ((TabBehaviour)Tweaklets.get(TabBehaviour.KEY)).findReusableEditor(page);
  return IEditorReference::Pointer(0);
}

IEditorReference::Pointer EditorManager::OpenEditor(
    const QString& editorId, IEditorInput::Pointer input, bool /*setVisible*/,
    IMemento::Pointer editorState)
{
  if (input.IsNull())
  {
    throw Poco::InvalidArgumentException();
  }

  IEditorRegistry* reg = this->GetEditorRegistry();
  EditorDescriptor::Pointer desc = reg->FindEditor(editorId).Cast<EditorDescriptor>();
  if (desc.IsNull())
  {
    throw PartInitException("Unable to open editor, unknown editor id: " + editorId);
  }

  return this->OpenEditorFromDescriptor(desc, input, editorState);
}

IEditorReference::Pointer EditorManager::OpenEditorFromDescriptor(
    EditorDescriptor::Pointer desc, IEditorInput::Pointer input,
    IMemento::Pointer editorState)
{
  IEditorReference::Pointer result;
  if (desc->IsInternal())
  {
    result = this->ReuseInternalEditor(desc, input);
    if (result.IsNull())
    {
      result = new EditorReference(this, input, desc, editorState);
    }
  }
//  else if (desc->GetId() == IEditorRegistry::SYSTEM_INPLACE_EDITOR_ID)
//  {
//    if (ComponentSupport.inPlaceEditorSupported())
//    {
//      result = new EditorReference(this, input, desc);
//    }
//  }
//  else if (desc->GetId() == IEditorRegistry::SYSTEM_EXTERNAL_EDITOR_ID)
//  {
//    IPathEditorInput pathInput = getPathEditorInput(input);
//    if (pathInput != null)
//    {
//      result = openSystemExternalEditor(pathInput.getPath());
//    }
//    else
//    {
//      throw new PartInitException(
//          WorkbenchMessages.EditorManager_systemEditorError);
//    }
//  }
//  else if (desc->IsOpenExternal())
//  {
//    result = openExternalEditor(desc, input);
//  }
  else
  {
    // this should never happen
    throw PartInitException("Invalid editor descriptor for id:" + desc->GetId());
  }

  if (result.IsNotNull())
  {
    this->CreateEditorTab(result.Cast<EditorReference>(), ""); //$NON-NLS-1$
  }

//  Workbench wb = (Workbench) window.getWorkbench();
//  wb.getEditorHistory().add(input, desc);
  return result;
}

//IEditorReference::Pointer EditorManager::OpenExternalEditor(
//    EditorDescriptor::Pointer desc, IEditorInput::Pointer input)
//{
//  final CoreException ex[] = new CoreException[1];
//
//  final IPathEditorInput pathInput = getPathEditorInput(input);
//  if (pathInput != null && pathInput.getPath() != null)
//  {
//    BusyIndicator.showWhile(getDisplay(), new Runnable()
//        {
//        public void run()
//          {
//            try
//            {
//              if (desc.getLauncher() != null)
//              {
//                // open using launcher
//                Object launcher = WorkbenchPlugin.createExtension(
//                    desc.getConfigurationElement(), "launcher"); //$NON-NLS-1$
//                ((IEditorLauncher) launcher).open(pathInput
//                    .getPath());
//              }
//              else
//              {
//                // open using command
//                ExternalEditor oEditor = new ExternalEditor(
//                    pathInput.getPath(), desc);
//                oEditor.open();
//              }
//            }
//            catch (CoreException e)
//            {
//              ex[0] = e;
//            }
//          }
//        }
//        );
//      }
//  else
//  {
//    throw new PartInitException(NLS.bind(
//            WorkbenchMessages.EditorManager_errorOpeningExternalEditor,
//            desc.getFileName(), desc.getId()));
//  }
//
//  if (ex[0] != null)
//  {
//    throw new PartInitException(NLS.bind(
//            WorkbenchMessages.EditorManager_errorOpeningExternalEditor,
//            desc.getFileName(), desc.getId()), ex[0]);
//  }
//
//  // we do not have an editor part for external editors
//  return null;
//}

void EditorManager::CreateEditorTab(EditorReference::Pointer ref, const QString& workbookId)
{
  editorPresentation->AddEditor(ref, workbookId);
}

EditorSite::Pointer EditorManager::CreateSite(IEditorReference::Pointer ref,
    IEditorPart::Pointer part, EditorDescriptor::Pointer desc,
    IEditorInput::Pointer input) const
{
  EditorSite::Pointer site(new EditorSite(ref, part, page, desc));
  if (desc.IsNotNull())
  {
    //site.setActionBars(createEditorActionBars(desc, site));
  }
  else
  {
    //site.setActionBars(createEmptyEditorActionBars(site));
  }
  const QString label = part->GetPartName(); // debugging only
  try
  {
    part->Init(site, input);

    // Sanity-check the site
    if (!(part->GetSite() == site) || !(part->GetEditorSite() == site))
    {
      throw PartInitException("Editor initialization failed: " + desc->GetId() + ". Site is incorrect.");
    }

  }
  catch (PartInitException e)
  {
    throw e;
  }
  catch (std::exception e)
  {
    throw PartInitException(QString("An exception was thrown during initialization: ") + e.what());
  }

  return site;
}

IEditorReference::Pointer EditorManager::ReuseInternalEditor(
    EditorDescriptor::Pointer  /*desc*/, IEditorInput::Pointer  /*input*/)
{

//  poco_assert(desc.IsNotNull()); // "descriptor must not be null"); //$NON-NLS-1$
//  poco_assert(input.IsNotNull()); // "input must not be null"); //$NON-NLS-1$
//
//  IEditorReference::Pointer reusableEditorRef = this->FindReusableEditor(desc);
//  if (reusableEditorRef.IsNotNull())
//  {
//    return this->ReuseInternalEditor(page, this, editorPresentation, desc, input,
//        reusableEditorRef);
//  }
  return IEditorReference::Pointer(0);
}

IEditorPart::Pointer EditorManager::CreatePart(EditorDescriptor::Pointer desc) const
{
//  try
//  {
    IEditorPart::Pointer result = desc->CreateEditor();
//    IConfigurationElement element = desc.getConfigurationElement();
//    if (element != null)
//    {
//      page.getExtensionTracker().registerObject(
//          element.getDeclaringExtension(), result,
//          IExtensionTracker.REF_WEAK);
//    }
    return result;
 // }
//  catch (CoreException e)
//  {
//    throw PartInitException(StatusUtil.newStatus(
//            desc.getPluginID(),
//            WorkbenchMessages.EditorManager_instantiationError, e));
//  }
}

//IEditorReference::Pointer EditorManager::OpenSystemExternalEditor(
//    Poco::Path location)
//{
//  if (location == null)
//  {
//    throw new IllegalArgumentException();
//  }
//
//  final boolean result[] =
//  { false};
//  BusyIndicator.showWhile(getDisplay(), new Runnable()
//      {
//      public void run()
//        {
//          if (location != null)
//          {
//            result[0] = Program.launch(location.toOSString());
//          }
//        }
//      }
//      );
//
//      if (!result[0])
//      {
//        throw new PartInitException(NLS.bind(
//                WorkbenchMessages.EditorManager_unableToOpenExternalEditor,
//                location));
//      }
//
//      // We do not have an editor part for external editors
//      return null;
//    }

//    ImageDescriptor EditorManager::FindImage(EditorDescriptor::Pointer desc,
//        Poco::Path path)
//    {
//      if (desc == null)
//      {
//        // @issue what should be the default image?
//        return ImageDescriptor.getMissingImageDescriptor();
//      }
//
//      if (desc.isOpenExternal() && path != null)
//      {
//        return PlatformUI.getWorkbench().getEditorRegistry()
//        .getImageDescriptor(path.toOSString());
//      }
//
//      return desc.getImageDescriptor();
//    }

    bool EditorManager::RestoreState(IMemento::Pointer memento)
    {
      // Restore the editor area workbooks layout/relationship
//      MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID,
//          IStatus.OK,
//          WorkbenchMessages.EditorManager_problemsRestoringEditors, null);
      bool result = true;

      QString activeWorkbookID;
      QList<IEditorReference::Pointer> visibleEditors;
      QList<IEditorReference::Pointer> activeEditor;

      IMemento::Pointer areaMem = memento->GetChild(WorkbenchConstants::TAG_AREA);
      if (areaMem)
      {
        //result.add(editorPresentation.restoreState(areaMem));
        editorPresentation->RestoreState(areaMem);
        areaMem->GetString(WorkbenchConstants::TAG_ACTIVE_WORKBOOK, activeWorkbookID);
      }

      // Loop through the editors.

      QList<IMemento::Pointer> editorMems(memento->GetChildren(WorkbenchConstants::TAG_EDITOR));
      for (int x = 0; x < editorMems.size(); x++)
      {
        // for dynamic UI - call restoreEditorState to replace code which is
        // commented out
        RestoreEditorState(editorMems[x], visibleEditors, activeEditor); //, result);
      }

      // restore the presentation
      if (areaMem)
      {
        //result.add(editorPresentation.restorePresentationState(areaMem));
        result &= editorPresentation->RestorePresentationState(areaMem);
      }
      try
      {
//        StartupThreading.runWithThrowable(new StartupRunnable()
//            {
//
//            public void runWithException() throws Throwable
//              {
                // Update each workbook with its visible editor.
                for (int i = 0; i < visibleEditors.size(); i++)
                {
                  SetVisibleEditor(visibleEditors[i], false);
                }

                // Update the active workbook
                if (!activeWorkbookID.isEmpty())
                {
                  editorPresentation->SetActiveEditorWorkbookFromID(activeWorkbookID);
                }

                if (!activeEditor.empty() && activeEditor[0])
                {
                  IWorkbenchPart::Pointer editor = activeEditor[0]->GetPart(true);

                  if (editor)
                  {
                    page->Activate(editor);
                  }
                }
//              }});
      }
      catch (...)
      {
        // The exception is already logged.
//        result
//        .add(new Status(
//                IStatus.ERR,
//                PlatformUI.PLUGIN_ID,
//                0,
//                WorkbenchMessages.EditorManager_exceptionRestoringEditor,
//                t));
        result &= false;
      }

      return result;
    }

    bool EditorManager::SaveAll(bool confirm, bool closing,
        bool addNonPartSources)
    {
      // Get the list of dirty editors and views. If it is
      // empty just return.
      QList<ISaveablePart::Pointer> parts(page->GetDirtyParts());
      if (parts.empty())
      {
        return true;
      }

      QList<IWorkbenchPart::Pointer> wbParts;
      for (QList<ISaveablePart::Pointer>::const_iterator i = parts.begin();
        i != parts.end(); ++i)
      {
        if (IWorkbenchPart::Pointer part = i->Cast<IWorkbenchPart>())
        {
          wbParts.push_back(part);
        }
      }

      // If confirmation is required ..
      return this->SaveAll(wbParts, confirm, closing, addNonPartSources, IWorkbenchWindow::Pointer(window));
    }

    bool EditorManager::SaveAll(
        const QList<IWorkbenchPart::Pointer>& /*dirtyParts*/, bool /*confirm*/,
        bool /*closing*/, bool /*addNonPartSources*/, SmartPointer<IWorkbenchWindow> /*window*/)
    {
//      // clone the input list
//          dirtyParts = new ArrayList(dirtyParts);
//            List modelsToSave;
//          if (confirm) {
//            boolean saveable2Processed = false;
//            // Process all parts that implement ISaveablePart2.
//            // These parts are removed from the list after saving
//            // them. We then need to restore the workbench to
//            // its previous state, for now this is just last
//            // active perspective.
//            // Note that the given parts may come from multiple
//            // windows, pages and perspectives.
//            ListIterator listIterator = dirtyParts.listIterator();
//
//            WorkbenchPage currentPage = null;
//            Perspective currentPageOriginalPerspective = null;
//            while (listIterator.hasNext()) {
//              IWorkbenchPart part = (IWorkbenchPart) listIterator.next();
//              if (part instanceof ISaveablePart2) {
//                WorkbenchPage page = (WorkbenchPage) part.getSite()
//                    .getPage();
//                if (!Util.equals(currentPage, page)) {
//                  if (currentPage != null
//                      && currentPageOriginalPerspective != null) {
//                    if (!currentPageOriginalPerspective
//                        .equals(currentPage.getActivePerspective())) {
//                      currentPage
//                          .setPerspective(currentPageOriginalPerspective
//                              .getDesc());
//                    }
//                  }
//                  currentPage = page;
//                  currentPageOriginalPerspective = page
//                      .getActivePerspective();
//                }
//                if (confirm) {
//                  if (part instanceof IViewPart) {
//                    Perspective perspective = page
//                        .getFirstPerspectiveWithView((IViewPart) part);
//                    if (perspective != null) {
//                      page.setPerspective(perspective.getDesc());
//                    }
//                  }
//                  // show the window containing the page?
//                  IWorkbenchWindow partsWindow = page
//                      .getWorkbenchWindow();
//                  if (partsWindow != partsWindow.getWorkbench()
//                      .getActiveWorkbenchWindow()) {
//                    Shell shell = partsWindow.getShell();
//                    if (shell.getMinimized()) {
//                      shell.setMinimized(false);
//                    }
//                    shell.setActive();
//                  }
//                  page.bringToTop(part);
//                }
//                // try to save the part
//                int choice = SaveableHelper.savePart((ISaveablePart2) part,
//                    page.getWorkbenchWindow(), confirm);
//                if (choice == ISaveablePart2.CANCEL) {
//                  // If the user cancels, don't restore the previous
//                  // workbench state, as that will
//                  // be an unexpected switch from the current state.
//                  return false;
//                } else if (choice != ISaveablePart2.DEFAULT) {
//                  saveable2Processed = true;
//                  listIterator.remove();
//                }
//              }
//            }
//
//            // try to restore the workbench to its previous state
//            if (currentPage != null && currentPageOriginalPerspective != null) {
//              if (!currentPageOriginalPerspective.equals(currentPage
//                  .getActivePerspective())) {
//                currentPage.setPerspective(currentPageOriginalPerspective
//                    .getDesc());
//              }
//            }
//
//            // if processing a ISaveablePart2 caused other parts to be
//            // saved, remove them from the list presented to the user.
//            if (saveable2Processed) {
//              listIterator = dirtyParts.listIterator();
//              while (listIterator.hasNext()) {
//                ISaveablePart part = (ISaveablePart) listIterator.next();
//                if (!part.isDirty()) {
//                  listIterator.remove();
//                }
//              }
//            }
//
//                  modelsToSave = convertToSaveables(dirtyParts, closing, addNonPartSources);
//
//                  // If nothing to save, return.
//                  if (modelsToSave.isEmpty()) {
//              return true;
//            }
//            boolean canceled = SaveableHelper.waitForBackgroundSaveJobs(modelsToSave);
//            if (canceled) {
//              return false;
//            }
//                  // Use a simpler dialog if there's only one
//                  if (modelsToSave.size() == 1) {
//                    Saveable model = (Saveable) modelsToSave.get(0);
//              String message = NLS.bind(WorkbenchMessages.EditorManager_saveChangesQuestion, model.getName());
//              // Show a dialog.
//              String[] buttons = new String[] { IDialogConstants.YES_LABEL, IDialogConstants.NO_LABEL, IDialogConstants.CANCEL_LABEL };
//              MessageDialog d = new MessageDialog(
//                shellProvider.getShell(), WorkbenchMessages.Save_Resource,
//                null, message, MessageDialog.QUESTION, buttons, 0);
//
//              int choice = SaveableHelper.testGetAutomatedResponse();
//              if (SaveableHelper.testGetAutomatedResponse() == SaveableHelper.USER_RESPONSE) {
//                choice = d.open();
//              }
//
//              // Branch on the user choice.
//              // The choice id is based on the order of button labels
//              // above.
//              switch (choice) {
//              case ISaveablePart2.YES: // yes
//                break;
//              case ISaveablePart2.NO: // no
//                return true;
//              default:
//              case ISaveablePart2.CANCEL: // cancel
//                return false;
//              }
//                  }
//                  else {
//                    ListSelectionDialog dlg = new ListSelectionDialog(
//                            shellProvider.getShell(), modelsToSave,
//                            new ArrayContentProvider(),
//                            new WorkbenchPartLabelProvider(), RESOURCES_TO_SAVE_MESSAGE);
//                    dlg.setInitialSelections(modelsToSave.toArray());
//                    dlg.setTitle(SAVE_RESOURCES_TITLE);
//
//                    // this "if" statement aids in testing.
//                    if (SaveableHelper.testGetAutomatedResponse()==SaveableHelper.USER_RESPONSE) {
//                      int result = dlg.open();
//                        //Just return false to prevent the operation continuing
//                        if (result == IDialogConstants.CANCEL_ID) {
//                  return false;
//                }
//
//                        modelsToSave = Arrays.asList(dlg.getResult());
//                    }
//                  }
//              }
//              else {
//                modelsToSave = convertToSaveables(dirtyParts, closing, addNonPartSources);
//          }
//
//              // If the editor list is empty return.
//              if (modelsToSave.isEmpty()) {
//            return true;
//          }
//
//          // Create save block.
//              final List finalModels = modelsToSave;
//          IRunnableWithProgress progressOp = new IRunnableWithProgress() {
//            public void run(IProgressMonitor monitor) {
//              IProgressMonitor monitorWrap = new EventLoopProgressMonitor(
//                  monitor);
//              monitorWrap.beginTask("", finalModels.size()); //$NON-NLS-1$
//              for (Iterator i = finalModels.iterator(); i.hasNext();) {
//                Saveable model = (Saveable) i.next();
//                // handle case where this model got saved as a result of saving another
//                if (!model.isDirty()) {
//                  monitor.worked(1);
//                  continue;
//                }
//                SaveableHelper.doSaveModel(model, new SubProgressMonitor(monitorWrap, 1), shellProvider, closing || confirm);
//                if (monitorWrap.isCanceled()) {
//                  break;
//                }
//              }
//              monitorWrap.done();
//            }
//          };
//
//          // Do the save.
//          return SaveableHelper.runProgressMonitorOperation(
//              WorkbenchMessages.Save_All, progressOp, runnableContext, shellProvider);
      return true;
    }

    bool EditorManager::SavePart(ISaveablePart::Pointer  /*saveable*/, IWorkbenchPart::Pointer  /*part*/, bool  /*confirm*/)
    {
      //TODO EditorManager save part (SaveableHelper)
      //return SaveableHelper.savePart(saveable, part, window, confirm);
      return true;
    }

    bool EditorManager::SaveState(const IMemento::Pointer memento)
    {

//      final MultiStatus result = new MultiStatus(PlatformUI.PLUGIN_ID,
//          IStatus.OK,
//          WorkbenchMessages.EditorManager_problemsSavingEditors, null);
      bool result = true;

      // Save the editor area workbooks layout/relationship
      IMemento::Pointer editorAreaMem = memento->CreateChild(WorkbenchConstants::TAG_AREA);
      //result.add(editorPresentation.saveState(editorAreaMem));
      result &= editorPresentation->SaveState(editorAreaMem);

      // Save the active workbook id
      editorAreaMem->PutString(WorkbenchConstants::TAG_ACTIVE_WORKBOOK,
          editorPresentation->GetActiveEditorWorkbookID());

      // Get each workbook
      QList<PartStack::Pointer> workbooks(editorPresentation->GetWorkbooks());

      for (QList<PartStack::Pointer>::iterator iter = workbooks.begin();
          iter != workbooks.end(); ++iter)
      {
        PartStack::Pointer workbook = *iter;

        // Use the list of editors found in EditorStack; fix for 24091
        QList<LayoutPart::Pointer> editorPanes(workbook->GetChildren());

        for (QList<LayoutPart::Pointer>::iterator i = editorPanes.begin();
            i != editorPanes.end(); ++i)
        {
          // Save each open editor.
          EditorReference::Pointer editorReference = i->Cast<PartPane>()->GetPartReference().Cast<EditorReference>();
          IEditorPart::Pointer editor = editorReference->GetEditor(false);
          if (!editor)
          {
            if (editorReference->GetMemento())
            {
              IMemento::Pointer editorMem = memento
              ->CreateChild(WorkbenchConstants::TAG_EDITOR);
              editorMem->PutMemento(editorReference->GetMemento());
            }
            continue;
          }

          // for dynamic UI - add the next line to replace the subsequent
          // code which is commented out
          SaveEditorState(memento, editorReference); //, result);
        }
      }
      return result;
    }

    bool EditorManager::SetVisibleEditor(IEditorReference::Pointer newEd,
        bool setFocus)
    {
      return editorPresentation->SetVisibleEditor(newEd, setFocus);
    }

    IPathEditorInput::Pointer EditorManager::GetPathEditorInput(
        IEditorInput::Pointer input)
    {
      if (input.Cast<IPathEditorInput>().IsNotNull())
      {
        return input.Cast<IPathEditorInput>();
      }

//      return (IPathEditorInput)
//      Util.getAdapter(input, IPathEditorInput.class);
      return IPathEditorInput::Pointer(0);
    }

    void EditorManager::RestoreEditorState(IMemento::Pointer /*editorMem*/,
        QList<IEditorReference::Pointer>& /*visibleEditors*/,
        QList<IEditorReference::Pointer>&  /*activeEditor*/)
    {
      // MultiStatus result) {

      //TODO Restore editor state
      // String strFocus = editorMem.getString(IWorkbenchConstants.TAG_FOCUS);
      // boolean visibleEditor = "true".equals(strFocus); //$NON-NLS-1$
//      EditorReference::Pointer e = new EditorReference(this, editorMem);
//
//      try
//      {
//        StartupThreading.runWithPartInitExceptions(new StartupRunnable ()
//            {
//
//            public void runWithException() throws Throwable
//              {
//                createEditorTab(e, workbookID);
//              }});
//
//      }
//      catch (PartInitException ex)
//      {
//        result.add(ex.getStatus());
//      }
//
//      String strActivePart = editorMem
//      .getString(IWorkbenchConstants.TAG_ACTIVE_PART);
//      if ("true".equals(strActivePart))
//      { //$NON-NLS-1$
//        activeEditor[0] = e;
//      }
//
//      String strFocus = editorMem.getString(IWorkbenchConstants.TAG_FOCUS);
//      boolean visibleEditor = "true".equals(strFocus); //$NON-NLS-1$
//      if (visibleEditor)
//      {
//        visibleEditors.add(e);
//      }
    }

    void EditorManager::SaveEditorState(IMemento::Pointer /*mem*/,
        IEditorReference::Pointer  /*ed*/)
    {
      //TODO Save editor state
//      final EditorReference editorRef = (EditorReference) ed;
//      final IEditorPart editor = ed.getEditor(false);
//      final IMemento memento = mem;
//      final MultiStatus result = res;
//      if (!(editor.getEditorSite() instanceof EditorSite))
//      {
//        return;
//      }
//      final EditorSite site = (EditorSite) editor.getEditorSite();
//      if (site.getPane() instanceof MultiEditorInnerPane)
//      {
//        return;
//      }
//
//      SafeRunner.run(new SafeRunnable()
//          {
//          public void run()
//            {
//              // Get the input.
//              IEditorInput input = editor.getEditorInput();
//              if (!input.exists())
//              {
//                return;
//              }
//              IPersistableElement persistable = input.getPersistable();
//              if (persistable == null)
//              {
//                return;
//              }
//
//              // Save editor.
//              IMemento editorMem = memento
//              .createChild(IWorkbenchConstants.TAG_EDITOR);
//              editorMem.putString(IWorkbenchConstants.TAG_TITLE, editorRef
//                  .getTitle());
//              editorMem.putString(IWorkbenchConstants.TAG_NAME, editorRef
//                  .getName());
//              editorMem.putString(IWorkbenchConstants.TAG_ID, editorRef
//                  .getId());
//              editorMem.putString(IWorkbenchConstants.TAG_TOOLTIP, editorRef
//                  .getTitleToolTip());
//
//              editorMem.putString(IWorkbenchConstants.TAG_PART_NAME,
//                  editorRef.getPartName());
//
//              if (editor instanceof IWorkbenchPart3)
//              {
//                Map properties = ((IWorkbenchPart3) editor)
//                .getPartProperties();
//                if (!properties.isEmpty())
//                {
//                  IMemento propBag = editorMem
//                  .createChild(IWorkbenchConstants.TAG_PROPERTIES);
//                  Iterator i = properties.entrySet().iterator();
//                  while (i.hasNext())
//                  {
//                    Map.Entry entry = (Map.Entry) i.next();
//                    IMemento p = propBag.createChild(
//                        IWorkbenchConstants.TAG_PROPERTY,
//                        (String) entry.getKey());
//                    p.putTextData((String) entry.getValue());
//                  }
//                }
//              }
//
//              if (editorRef.isPinned())
//              {
//                editorMem.putString(IWorkbenchConstants.TAG_PINNED, "true"); //$NON-NLS-1$
//              }
//
//              EditorPane editorPane = (EditorPane) ((EditorSite) editor
//                  .getEditorSite()).getPane();
//              editorMem.putString(IWorkbenchConstants.TAG_WORKBOOK,
//                  editorPane.getWorkbook().getID());
//
//              if (editor == page.getActivePart())
//              {
//                editorMem.putString(IWorkbenchConstants.TAG_ACTIVE_PART,
//                    "true"); //$NON-NLS-1$
//              }
//
//              if (editorPane == editorPane.getWorkbook().getSelection())
//              {
//                editorMem.putString(IWorkbenchConstants.TAG_FOCUS, "true"); //$NON-NLS-1$
//              }
//
//              if (input instanceof IPathEditorInput)
//              {
//                IPath path = ((IPathEditorInput) input).getPath();
//                if (path != null)
//                {
//                  editorMem.putString(IWorkbenchConstants.TAG_PATH, path
//                      .toString());
//                }
//              }
//
//              // Save input.
//              IMemento inputMem = editorMem
//              .createChild(IWorkbenchConstants.TAG_INPUT);
//              inputMem.putString(IWorkbenchConstants.TAG_FACTORY_ID,
//                  persistable.getFactoryId());
//              persistable.saveState(inputMem);
//
//              // any editors that want to persist state
//              if (editor instanceof IPersistableEditor)
//              {
//                IMemento editorState = editorMem
//                .createChild(IWorkbenchConstants.TAG_EDITOR_STATE);
//                ((IPersistableEditor) editor).saveState(editorState);
//              }
//            }
//
//          public void handleException(Throwable e)
//            {
//              result
//              .add(new Status(
//                      IStatus.ERR,
//                      PlatformUI.PLUGIN_ID,
//                      0,
//                      NLS
//                      .bind(
//                          WorkbenchMessages.EditorManager_unableToSaveEditor,
//                          editorRef.getTitle()), e));
//            }
//          }
//          );
        }

    IMemento::Pointer EditorManager::GetMemento(IEditorReference::Pointer e)
    {
      if (e.Cast<EditorReference>().IsNotNull())
      {
        return e.Cast<EditorReference>()->GetMemento();
      }
      return IMemento::Pointer(0);
    }

    IEditorReference::Pointer EditorManager::OpenEmptyTab()
    {
      IEditorInput::Pointer input(new NullEditorInput());
      EditorDescriptor::Pointer desc = (dynamic_cast<EditorRegistry*>(this->GetEditorRegistry()))
      ->FindEditor(EditorRegistry::EMPTY_EDITOR_ID).Cast<EditorDescriptor>();
      EditorReference::Pointer result(new EditorReference(this, input, desc));
      try
      {
        this->CreateEditorTab(result, "");
        return result;
      }
      catch (const PartInitException& e)
      {
//        StatusManager.getManager().handle(
//            StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH, e));
        BERRY_ERROR << e.what() << std::endl;
      }
      return IEditorReference::Pointer(0);
    }

    bool EditorManager::UseIPersistableEditor()
    {
//      IPreferenceStore store = WorkbenchPlugin.getDefault()
//      .getPreferenceStore();
//      return store.getBoolean(IPreferenceConstants.USE_IPERSISTABLE_EDITORS);
      return false;
    }

    }
