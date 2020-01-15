/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berrySaveablesList.h"
#include "berryWorkbenchPlugin.h"
#include "berryDefaultSaveable.h"

#include "berryWorkbenchPart.h"

namespace berry
{

bool SaveablesList::AddModel(Object::Pointer source, Saveable::Pointer model)
{
  if (model == 0)
  {
    this->LogWarning("Ignored attempt to add invalid saveable", source, model); //$NON-NLS-1$
    return false;
  }
  bool result = false;
  Saveable::Set& modelsForSource = modelMap[source.GetPointer()];

  if (modelsForSource.find(model) == modelsForSource.end())
  {
    modelsForSource.insert(model);
    result = this->IncrementRefCount(modelRefCounts, model);
  }
  else
  {
    this->LogWarning("Ignored attempt to add saveable that was already registered",
        source, model); //$NON-NLS-1$
  }
  return result;
}

bool SaveablesList::IncrementRefCount(
    QHash<Saveable::Pointer, int>& referenceMap, Saveable::Pointer key)
{
  bool result = false;
  int& refCount = referenceMap[key];
  if (refCount == 0)
  {
    result = true;
  }
  refCount++;
  return result;
}

bool SaveablesList::DecrementRefCount(
    QHash<Saveable::Pointer, int>& referenceMap, Saveable::Pointer key)
{
  bool result = false;
  int& refCount = referenceMap[key];
  poco_assert(refCount != 0);
  if (refCount == 1)
  {
    referenceMap.remove(key);
    result = true;
  }
  else
  {
    --refCount;
  }
  return result;
}

bool SaveablesList::RemoveModel(Object::Pointer source, Saveable::Pointer model)
{
  bool result = false;
  QHash<Object*, Saveable::Set>::iterator it = modelMap.find(source.GetPointer());
  if (it == modelMap.end())
  {
    this->LogWarning(
        "Ignored attempt to remove a saveable when no saveables were known",
        source, model); //$NON-NLS-1$
  }
  else
  {
    Saveable::Set& modelsForSource = it.value();
    if (modelsForSource.remove(model))
    {
      result = this->DecrementRefCount(modelRefCounts, model);
      if (modelsForSource.empty())
      {
        modelMap.remove(source.GetPointer());
      }
    }
    else
    {
      this->LogWarning(
          "Ignored attempt to remove a saveable that was not registered",
          source, model); //$NON-NLS-1$
    }
  }
  return result;
}

void SaveablesList::LogWarning(const QString& message,
    Object::Pointer source, Saveable::Pointer model)
{
  // create a new exception
  QString text = message + "; " + "unknown saveable: " + model->GetName()
          + " from part: " + source->GetClassName();
  // record the current stack trace to help with debugging
  //assertionFailedException.fillInStackTrace();
  WorkbenchPlugin::Log(text);
}

void SaveablesList::UpdateNonPartSource(ISaveablesSource::Pointer source)
{
  QList<Saveable::Pointer> saveables = source->GetSaveables();
  if (saveables.empty())
  {
    nonPartSources.remove(source);
  }
  else
  {
    nonPartSources.insert(source);
  }
}

void SaveablesList::RemoveModels(Object::Pointer source,
    const QList<Saveable::Pointer>& modelArray)
{
  QList<Saveable::Pointer> removed;
  for (int i = 0; i < modelArray.size(); i++)
  {
    Saveable::Pointer model = modelArray[i];
    if (this->RemoveModel(source, model))
    {
      removed.push_back(model);
    }
  }
  if (removed.size() > 0)
  {
    Object::Pointer source(this);
    SaveablesLifecycleEvent::Pointer event(new SaveablesLifecycleEvent(source,
        SaveablesLifecycleEvent::POST_OPEN, removed, false));
    this->FireModelLifecycleEvent(event);
  }
}

void SaveablesList::AddModels(Object::Pointer source,
    const QList<Saveable::Pointer>& modelArray)
{
  QList<Saveable::Pointer> added;
  for (int i = 0; i < modelArray.size(); i++)
  {
    Saveable::Pointer model = modelArray[i];
    if (this->AddModel(source, model))
    {
      added.push_back(model);
    }
  }
  if (added.size() > 0)
  {
    Object::Pointer source(this);
    SaveablesLifecycleEvent::Pointer event(new SaveablesLifecycleEvent(source,
        SaveablesLifecycleEvent::POST_OPEN, added, false));
    this->FireModelLifecycleEvent(event);
  }
}

void SaveablesList::FireModelLifecycleEvent(
    SaveablesLifecycleEvent::Pointer event)
{
  events.lifecycleChange(event);
}

bool SaveablesList::PromptForSavingIfNecessary(
    IWorkbenchWindow::Pointer /*window*/, const Saveable::Set& /*modelsClosing*/,
    const QHash<Saveable::Pointer, int>& /*modelsDecrementing*/, bool /*canCancel*/)
{
//  List modelsToOptionallySave = new ArrayList();
//  for (Iterator it = modelsDecrementing.keySet().iterator(); it.hasNext();)
//  {
//    Saveable modelDecrementing = (Saveable) it.next();
//    if (modelDecrementing.isDirty() && !modelsClosing.contains(
//        modelDecrementing))
//    {
//      modelsToOptionallySave.add(modelDecrementing);
//    }
//  }
//
//  boolean shouldCancel =
//      modelsToOptionallySave.isEmpty() ? false : promptForSaving(
//          modelsToOptionallySave, window, window, canCancel, true);
//
//  if (shouldCancel)
//  {
//    return true;
//  }
//
//  List modelsToSave = new ArrayList();
//  for (Iterator it = modelsClosing.iterator(); it.hasNext();)
//  {
//    Saveable modelClosing = (Saveable) it.next();
//    if (modelClosing.isDirty())
//    {
//      modelsToSave.add(modelClosing);
//    }
//  }
//  return modelsToSave.isEmpty() ? false : promptForSaving(modelsToSave, window,
//      window, canCancel, false);
  return false;
}

void SaveablesList::FillModelsClosing(Saveable::Set& modelsClosing,
    const QHash<Saveable::Pointer, int>& modelsDecrementing)
{
  for (QHash<Saveable::Pointer, int>::const_iterator it = modelsDecrementing.begin();
       it != modelsDecrementing.end(); ++it)
  {
    Saveable::Pointer model = it.key();
    if (it.value() == modelRefCounts[model])
    {
      modelsClosing.insert(model);
    }
  }
}

QList<Saveable::Pointer> SaveablesList::GetSaveables(
    IWorkbenchPart::Pointer part)
{
  if (part.Cast<ISaveablesSource> () != 0)
  {
    ISaveablesSource::Pointer source = part.Cast<ISaveablesSource>();
    return source->GetSaveables();
  }
  else if (part.Cast<ISaveablePart> () != 0)
  {
    QList<Saveable::Pointer> result;
    Saveable::Pointer defaultSaveable(new DefaultSaveable(part));
    result.push_back(defaultSaveable);
    return result;
  }
  else
  {
    return QList<Saveable::Pointer>();
  }
}

Saveable::Set SaveablesList::GetOpenModels()
{
  Saveable::Set allDistinctModels;
  for (QHash<Object*, Saveable::Set>::iterator it = modelMap.begin();
       it != modelMap.end(); ++it)
    allDistinctModels.unite(it.value());

  return allDistinctModels;
}

void SaveablesList::HandleLifecycleEvent(const SaveablesLifecycleEvent::Pointer& event)
{
  if (event->GetSource().Cast<IWorkbenchPart> () == 0)
  {
    // just update the set of non-part sources. No prompting necessary.
    // See bug 139004.
    this->UpdateNonPartSource(event->GetSource().Cast<ISaveablesSource>());
    return;
  }
  QList<Saveable::Pointer> modelArray = event->GetSaveables();
  int eventType = event->GetEventType();
  if (eventType == SaveablesLifecycleEvent::POST_OPEN)
  {
    this->AddModels(event->GetSource(), modelArray);
  }
  else if (eventType == SaveablesLifecycleEvent::PRE_CLOSE)
  {
    QList<Saveable::Pointer> models = event->GetSaveables();
    QHash<Saveable::Pointer, int> modelsDecrementing;
    Saveable::Set modelsClosing;
    for (int i = 0; i < models.size(); i++)
    {
      this->IncrementRefCount(modelsDecrementing, models[i]);
    }

    this->FillModelsClosing(modelsClosing, modelsDecrementing);
    bool canceled = this->PromptForSavingIfNecessary(
        PlatformUI::GetWorkbench()->GetActiveWorkbenchWindow(), modelsClosing,
        modelsDecrementing, !event->IsForce());
    if (canceled)
    {
      event->SetVeto(true);
    }
  }
  else if (eventType == SaveablesLifecycleEvent::POST_CLOSE)
  {
    this->RemoveModels(event->GetSource(), modelArray);
  }
  else if (eventType == SaveablesLifecycleEvent::DIRTY_CHANGED)
  {
    Object::Pointer source(this);
    SaveablesLifecycleEvent::Pointer lifeCycleEvent(
      new SaveablesLifecycleEvent(source, event->GetEventType(), event->GetSaveables(), false));
    this->FireModelLifecycleEvent(lifeCycleEvent);
  }
}

void SaveablesList::AddModelLifecycleListener(ISaveablesLifecycleListener* listener)
{
  events.AddListener(listener);
}

void SaveablesList::RemoveModelLifecycleListener(ISaveablesLifecycleListener* listener)
{
  events.RemoveListener(listener);
}

SaveablesList::PostCloseInfo::Pointer SaveablesList::PreCloseParts(
    const QList<IWorkbenchPart::Pointer>& partsToClose, bool save,
    IWorkbenchWindow::Pointer window)
{
  // reference count (how many occurrences of a model will go away?)
  PostCloseInfo::Pointer postCloseInfo(new PostCloseInfo());
  for (QList<IWorkbenchPart::Pointer>::const_iterator it = partsToClose.begin();
       it != partsToClose.end(); ++it)
  {
    WorkbenchPart::Pointer part = it->Cast<WorkbenchPart>();
    postCloseInfo->partsClosing.push_back(part);
    if (part.Cast<ISaveablePart> () != 0)
    {
      ISaveablePart::Pointer saveablePart = part.Cast<ISaveablePart>();
      if (save && !saveablePart->IsSaveOnCloseNeeded())
      {
        // pretend for now that this part is not closing
        continue;
      }
    }
//    if (save && part.Cast<ISaveablePart2> () != 0)
//    {
//      ISaveablePart2 saveablePart2 = (ISaveablePart2) part;
//      // TODO show saveablePart2 before prompting, see
//      // EditorManager.saveAll
//      int response = SaveableHelper.savePart(saveablePart2, window, true);
//      if (response == ISaveablePart2.CANCEL)
//      {
//        // user canceled
//        return 0;
//      }
//      else if (response != ISaveablePart2.DEFAULT)
//      {
//        // only include this part in the following logic if it returned
//        // DEFAULT
//        continue;
//      }
//    }
    QList<Saveable::Pointer> modelsFromSource = this->GetSaveables(part);
    for (int i = 0; i < modelsFromSource.size(); i++)
    {
      this->IncrementRefCount(postCloseInfo->modelsDecrementing, modelsFromSource[i]);
    }
  }
  this->FillModelsClosing(postCloseInfo->modelsClosing,
      postCloseInfo->modelsDecrementing);
  if (save)
  {
    bool canceled = this->PromptForSavingIfNecessary(window,
        postCloseInfo->modelsClosing, postCloseInfo->modelsDecrementing, true);
    if (canceled)
    {
      return PostCloseInfo::Pointer(nullptr);
    }
  }
  return postCloseInfo;
}

bool SaveablesList::PromptForSaving(
    const QList<Saveable::Pointer>& /*modelsToSave*/,
    /*final IShellProvider shellProvider, IRunnableContext runnableContext,*/
    bool /*canCancel*/, bool /*stillOpenElsewhere*/)
{
  //  // Save parts, exit the method if cancel is pressed.
  //  if (modelsToSave.size() > 0) {
  //    boolean canceled = SaveableHelper.waitForBackgroundSaveJobs(modelsToSave);
  //    if (canceled) {
  //      return true;
  //    }
  //
  //    IPreferenceStore apiPreferenceStore = PrefUtil.getAPIPreferenceStore();
  //    boolean dontPrompt = stillOpenElsewhere && !apiPreferenceStore.getBoolean(IWorkbenchPreferenceConstants.PROMPT_WHEN_SAVEABLE_STILL_OPEN);
  //
  //    if (dontPrompt) {
  //      modelsToSave.clear();
  //      return false;
  //    } else if (modelsToSave.size() == 1) {
  //      Saveable model = (Saveable) modelsToSave.get(0);
  //      // Show a dialog.
  //      QList<QString> buttons;
  //      if(canCancel) {
  //        buttons.push_back(IDialogConstants.YES_LABEL);
  //            buttons.push_back(IDialogConstants.NO_LABEL);
  //            buttons.push_back(IDialogConstants.CANCEL_LABEL);
  //      } else {
  //        buttons.push_back(IDialogConstants.YES_LABEL);
  //            buttons.push_back(IDialogConstants.NO_LABEL);
  //      }
  //
  //      // don't save if we don't prompt
  //      int choice = ISaveablePart2.NO;
  //
  //      MessageDialog dialog;
  //      if (stillOpenElsewhere) {
  //        String message = NLS
  //            .bind(
  //                WorkbenchMessages.EditorManager_saveChangesOptionallyQuestion,
  //                model.getName());
  //        MessageDialogWithToggle dialogWithToggle = new MessageDialogWithToggle(shellProvider.getShell(),
  //            WorkbenchMessages.Save_Resource, 0, message,
  //            MessageDialog.QUESTION, buttons, 0, WorkbenchMessages.EditorManager_closeWithoutPromptingOption, false) {
  //          protected int getShellStyle() {
  //            return (canCancel ? SWT.CLOSE : SWT.NONE)
  //                | SWT.TITLE | SWT.BORDER
  //                | SWT.APPLICATION_MODAL
  //                | getDefaultOrientation();
  //          }
  //        };
  //        dialog = dialogWithToggle;
  //      } else {
  //        String message = NLS
  //            .bind(
  //                WorkbenchMessages.EditorManager_saveChangesQuestion,
  //                model.getName());
  //        dialog = new MessageDialog(shellProvider.getShell(),
  //            WorkbenchMessages.Save_Resource, 0, message,
  //            MessageDialog.QUESTION, buttons, 0) {
  //          protected int getShellStyle() {
  //            return (canCancel ? SWT.CLOSE : SWT.NONE)
  //                | SWT.TITLE | SWT.BORDER
  //                | SWT.APPLICATION_MODAL
  //                | getDefaultOrientation();
  //          }
  //        };
  //      }
  //
  //      choice = SaveableHelper.testGetAutomatedResponse();
  //      if (SaveableHelper.testGetAutomatedResponse() == SaveableHelper.USER_RESPONSE) {
  //        choice = dialog.open();
  //
  //        if(stillOpenElsewhere) {
  //          // map value of choice back to ISaveablePart2 values
  //          switch (choice) {
  //          case IDialogConstants.YES_ID:
  //            choice = ISaveablePart2.YES;
  //            break;
  //          case IDialogConstants.NO_ID:
  //            choice = ISaveablePart2.NO;
  //            break;
  //          case IDialogConstants.CANCEL_ID:
  //            choice = ISaveablePart2.CANCEL;
  //            break;
  //          default:
  //            break;
  //          }
  //          MessageDialogWithToggle dialogWithToggle = (MessageDialogWithToggle) dialog;
  //          if (choice != ISaveablePart2.CANCEL && dialogWithToggle.getToggleState()) {
  //            apiPreferenceStore.setValue(IWorkbenchPreferenceConstants.PROMPT_WHEN_SAVEABLE_STILL_OPEN, false);
  //          }
  //        }
  //      }
  //
  //      // Branch on the user choice.
  //      // The choice id is based on the order of button labels
  //      // above.
  //      switch (choice) {
  //      case ISaveablePart2.YES: // yes
  //        break;
  //      case ISaveablePart2.NO: // no
  //        modelsToSave.clear();
  //        break;
  //      default:
  //      case ISaveablePart2.CANCEL: // cancel
  //        return true;
  //      }
  //    } else {
  //      MyListSelectionDialog dlg = new MyListSelectionDialog(
  //          shellProvider.getShell(),
  //          modelsToSave,
  //          new ArrayContentProvider(),
  //          new WorkbenchPartLabelProvider(),
  //          stillOpenElsewhere ? WorkbenchMessages.EditorManager_saveResourcesOptionallyMessage
  //              : WorkbenchMessages.EditorManager_saveResourcesMessage,
  //          canCancel, stillOpenElsewhere);
  //      dlg.setInitialSelections(modelsToSave.toArray());
  //      dlg.setTitle(EditorManager.SAVE_RESOURCES_TITLE);
  //
  //      // this "if" statement aids in testing.
  //      if (SaveableHelper.testGetAutomatedResponse() == SaveableHelper.USER_RESPONSE) {
  //        int result = dlg.open();
  //        // Just return 0 to prevent the operation continuing
  //        if (result == IDialogConstants.CANCEL_ID)
  //          return true;
  //
  //        if (dlg.getDontPromptSelection()) {
  //          apiPreferenceStore.setValue(IWorkbenchPreferenceConstants.PROMPT_WHEN_SAVEABLE_STILL_OPEN, false);
  //        }
  //
  //        modelsToSave = Arrays.asList(dlg.getResult());
  //      }
  //    }
  //  }
  //  // Create save block.
  //  return saveModels(modelsToSave, shellProvider, runnableContext);
  return true;
}

bool SaveablesList::SaveModels(const QList<Saveable::Pointer>& /*finalModels*/
/*final IShellProvider shellProvider, IRunnableContext runnableContext*/)
{
  //  IRunnableWithProgress progressOp = new IRunnableWithProgress() {
  //    public void run(IProgressMonitor monitor) {
  //      IProgressMonitor monitorWrap = new EventLoopProgressMonitor(
  //          monitor);
  //      monitorWrap.beginTask("", finalModels.size()); //$NON-NLS-1$
  //      for (Iterator i = finalModels.iterator(); i.hasNext();) {
  //        Saveable model = (Saveable) i.next();
  //        // handle case where this model got saved as a result of
  //        // saving another
  //        if (!model.isDirty()) {
  //          monitor.worked(1);
  //          continue;
  //        }
  //        SaveableHelper.doSaveModel(model, new SubProgressMonitor(monitorWrap, 1), shellProvider, true);
  //        if (monitorWrap.isCanceled())
  //          break;
  //      }
  //      monitorWrap.done();
  //    }
  //  };
  //
  //  // Do the save.
  //  return !SaveableHelper.runProgressMonitorOperation(
  //      WorkbenchMessages.Save_All, progressOp, runnableContext,
  //      shellProvider);
  return true;
}

void SaveablesList::PostClose(PostCloseInfo::Pointer postCloseInfo)
{
  QList<Saveable::Pointer> removed;
  for (QList<WorkbenchPart::Pointer>::const_iterator it = postCloseInfo->partsClosing.begin();
      it != postCloseInfo->partsClosing.end(); ++it)
  {
    IWorkbenchPart::Pointer part = *it;
    QHash<Object*, Saveable::Set>::iterator it2 = modelMap.find(part.GetPointer());
    if (it2 != modelMap.end()) {
      // make a copy to avoid a ConcurrentModificationException - we
      // will remove from the original set as we iterate
      Saveable::Set saveables(it2.value());
      for (Saveable::Set::const_iterator it3 = saveables.begin();
          it3 != saveables.end(); ++it3)
      {
        if (RemoveModel(part, *it3)) {
          removed.push_back(*it3);
        }
      }
    }
  }
  if (!removed.empty()) {
    Object::Pointer source(this);
    SaveablesLifecycleEvent::Pointer event(new SaveablesLifecycleEvent(source,
        SaveablesLifecycleEvent::POST_CLOSE, removed, false));
    this->FireModelLifecycleEvent(event);
  }
}

void SaveablesList::PostOpen(IWorkbenchPart::Pointer part)
{
  this->AddModels(part, this->GetSaveables(part));
}

void SaveablesList::DirtyChanged(IWorkbenchPart::Pointer part)
{
  QList<Saveable::Pointer> saveables = this->GetSaveables(part);
  if (saveables.size() > 0) {
    Object::Pointer source(this);
    SaveablesLifecycleEvent::Pointer event(new SaveablesLifecycleEvent(source,
        SaveablesLifecycleEvent::DIRTY_CHANGED, saveables, false));
    this->FireModelLifecycleEvent(event);
  }
}

QList<Object::Pointer> SaveablesList::TestGetSourcesForModel(
    Saveable::Pointer /*model*/)
{
  QList<Object::Pointer> result;
  //  for (Iterator it = modelMap.entrySet().iterator(); it.hasNext();) {
  //    Map.Entry entry = (Map.Entry) it.next();
  //    Set values = (Set) entry.getValue();
  //    if (values.contains(model)) {
  //      result.add(entry.getKey());
  //    }
  //  }
  return result;
}

QList<ISaveablesSource::Pointer> SaveablesList::GetNonPartSources()
{
  return nonPartSources.toList();
}

QList<IWorkbenchPart::Pointer> SaveablesList::GetPartsForSaveable(
    Saveable::Pointer model)
{
  QList<IWorkbenchPart::Pointer> result;
  for (QHash<Object*, Saveable::Set>::iterator it = modelMap.begin(); it
      != modelMap.end(); ++it)
  {
    Saveable::Set& values = it.value();
    IWorkbenchPart::Pointer part(dynamic_cast<IWorkbenchPart*>(it.key()));
    if (values.find(model) != values.end() && part)
    {
      result.push_back(part);
    }
  }
  return result;
}

}
