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

#ifndef CHERRYSAVEABLESLIST_H_
#define CHERRYSAVEABLESLIST_H_

#include "../cherryISaveablesLifecycleListener.h"

#include "../cherryISaveablesSource.h"
#include "../cherryIWorkbenchWindow.h"
#include "../cherryIWorkbenchPart.h"

#include "../cherryWorkbenchPart.h"

#include <map>
#include <set>
#include <list>
#include <vector>

namespace cherry
{

/**
 * The model manager maintains a list of open saveable models.
 *
 * @see Saveable
 * @see ISaveablesSource
 *
 * @since 3.2
 */
class SaveablesList: public ISaveablesLifecycleListener
{

public:

  cherryObjectMacro(SaveablesList);

private:

  ISaveablesLifecycleListener::Events events;

  // event source (mostly ISaveablesSource) -> Set of Saveable
  std::map<Object*, Saveable::Set> modelMap;

  // reference counting map, Saveable -> Integer
  std::map<Saveable::Pointer, int> modelRefCounts;

  std::set<ISaveablesSource::Pointer> nonPartSources;

  // returns true if this model has not yet been in getModels()
  bool AddModel(Object::Pointer source, Saveable::Pointer model);

  /**
   * returns true if the given key was added for the first time
   *
   * @param referenceMap
   * @param key
   * @return true if the ref count of the given key is now 1
   */
  bool IncrementRefCount(std::map<Saveable::Pointer, int>& referenceMap,
      Saveable::Pointer key);

  /**
   * returns true if the given key has been removed
   *
   * @param referenceMap
   * @param key
   * @return true if the ref count of the given key was 1
   */
  bool DecrementRefCount(std::map<Saveable::Pointer, int>& referenceMap,
      Saveable::Pointer key);

  // returns true if this model was removed from getModels();
  bool RemoveModel(Object::Pointer source, Saveable::Pointer model);

  void LogWarning(const std::string& message, Object::Pointer source,
      Saveable::Pointer model);

  /**
   * Updates the set of non-part saveables sources.
   * @param source
   */
  void UpdateNonPartSource(ISaveablesSource::Pointer source);

  /**
   * @param source
   * @param modelArray
   */
  void RemoveModels(Object::Pointer source,
      const std::vector<Saveable::Pointer>& modelArray);

  /**
   * @param source
   * @param modelArray
   */
  void AddModels(Object::Pointer source,
      const std::vector<Saveable::Pointer>& modelArray);

  /**
   * @param event
   */
  void FireModelLifecycleEvent(SaveablesLifecycleEvent::Pointer event);

  /**
   * @param window
   * @param modelsClosing
   * @param canCancel
   * @return true if the user canceled
   */
  bool
      PromptForSavingIfNecessary(IWorkbenchWindow::Pointer window,
          const Saveable::Set& modelsClosing,
          const std::map<Saveable::Pointer, int>& modelsDecrementing,
          bool canCancel);

  /**
   * @param modelsClosing
   * @param modelsDecrementing
   */
  void FillModelsClosing(Saveable::Set& modelsClosing,
      const std::map<Saveable::Pointer, int>& modelsDecrementing);


  /**
   * Returns the saveable models provided by the given part. If the part does
   * not provide any models, a default model is returned representing the
   * part.
   *
   * @param part
   *            the workbench part
   * @return the saveable models
   */
  std::vector<Saveable::Pointer> GetSaveables(IWorkbenchPart::Pointer part);

  //TODO SaveablesList ListSelectionDialog
  //  class MyListSelectionDialog extends
  //     ListSelectionDialog {
  //   private final boolean canCancel;
  //   private Button checkbox;
  //   private boolean dontPromptSelection;
  //   private boolean stillOpenElsewhere;
  //
  //   private MyListSelectionDialog(Shell shell, Object input,
  //       IStructuredContentProvider contentprovider,
  //       ILabelProvider labelProvider, String message, boolean canCancel, boolean stillOpenElsewhere) {
  //     super(shell, input, contentprovider, labelProvider, message);
  //     this.canCancel = canCancel;
  //     this.stillOpenElsewhere = stillOpenElsewhere;
  //     if (!canCancel) {
  //       int shellStyle = getShellStyle();
  //       shellStyle &= ~SWT.CLOSE;
  //       setShellStyle(shellStyle);
  //     }
  //   }
  //
  //   /**
  //    * @return
  //    */
  //   public boolean getDontPromptSelection() {
  //     return dontPromptSelection;
  //   }
  //
  //   protected void createButtonsForButtonBar(Composite parent) {
  //     createButton(parent, IDialogConstants.OK_ID,
  //         IDialogConstants.OK_LABEL, true);
  //     if (canCancel) {
  //       createButton(parent, IDialogConstants.CANCEL_ID,
  //           IDialogConstants.CANCEL_LABEL, false);
  //     }
  //   }
  //
  //   protected Control createDialogArea(Composite parent) {
  //      Composite dialogAreaComposite = (Composite) super.createDialogArea(parent);
  //
  //      if (stillOpenElsewhere) {
  //        Composite checkboxComposite = new Composite(dialogAreaComposite, SWT.NONE);
  //        checkboxComposite.setLayout(new GridLayout(2, false));
  //
  //        checkbox = new Button(checkboxComposite, SWT.CHECK);
  //        checkbox.addSelectionListener(new SelectionAdapter() {
  //         public void widgetSelected(SelectionEvent e) {
  //           dontPromptSelection = checkbox.getSelection();
  //         }
  //        });
  //        GridData gd = new GridData();
  //        gd.horizontalAlignment = SWT.BEGINNING;
  //        checkbox.setLayoutData(gd);
  //
  //        Label label = new Label(checkboxComposite, SWT.NONE);
  //        label.setText(WorkbenchMessages.EditorManager_closeWithoutPromptingOption);
  //        gd = new GridData();
  //        gd.grabExcessHorizontalSpace = true;
  //        gd.horizontalAlignment = SWT.BEGINNING;
  //      }
  //
  //      return dialogAreaComposite;
  //   }
  // }

public:

  struct PostCloseInfo : public Object
  {
    cherryObjectMacro(PostCloseInfo);

    std::list<SmartPointer<WorkbenchPart> > partsClosing;

    std::map<Saveable::Pointer, int> modelsDecrementing;

    Saveable::Set modelsClosing;

  private:
    PostCloseInfo() {}

    friend class SaveablesList;
  };

  /**
   * Returns the list of open models managed by this model manager.
   *
   * @return a list of models
   */
  Saveable::Set GetOpenModels();

  /**
   * This implementation of handleModelLifecycleEvent must be called by
   * implementers of ISaveablesSource whenever the list of models of the model
   * source changes, or when the dirty state of models changes. The
   * ISaveablesSource instance must be passed as the source of the event
   * object.
   * <p>
   * This method may also be called by objects that hold on to models but are
   * not workbench parts. In this case, the event source must be set to an
   * object that is not an instanceof IWorkbenchPart.
   * </p>
   * <p>
   * Corresponding open and close events must originate from the same
   * (identical) event source.
   * </p>
   * <p>
   * This method must be called on the UI thread.
   * </p>
   */
  void HandleLifecycleEvent(SaveablesLifecycleEvent::Pointer event);

  /**
   * Adds the given listener to the list of listeners. Has no effect if the
   * same (identical) listener has already been added. The listener will be
   * notified about changes to the models managed by this model manager. Event
   * types include: <br>
   * POST_OPEN when models were added to the list of models <br>
   * POST_CLOSE when models were removed from the list of models <br>
   * DIRTY_CHANGED when the dirty state of models changed
   * <p>
   * Listeners should ignore all other event types, including PRE_CLOSE. There
   * is no guarantee that listeners are notified before models are closed.
   *
   * @param listener
   */
  void AddModelLifecycleListener(ISaveablesLifecycleListener::Pointer listener);

  /**
   * Removes the given listener from the list of listeners. Has no effect if
   * the given listener is not contained in the list.
   *
   * @param listener
   */
  void RemoveModelLifecycleListener(
      ISaveablesLifecycleListener::Pointer listener);

  /**
   * @param partsToClose
   * @param save
   * @param window
   * @return the post close info to be passed to postClose
   */
  PostCloseInfo::Pointer PreCloseParts(
      const std::list<SmartPointer<IWorkbenchPart> >& partsToClose, bool save,
      IWorkbenchWindow::Pointer window);

  /**
   * Prompt the user to save the given saveables.
   * @param modelsToSave the saveables to be saved
   * @param shellProvider the provider used to obtain a shell in prompting is
   *            required. Clients can use a workbench window for this.
   * @param runnableContext a runnable context that will be used to provide a
   *            progress monitor while the save is taking place. Clients can
   *            use a workbench window for this.
   * @param canCancel whether the operation can be canceled
   * @param stillOpenElsewhere whether the models are referenced by open parts
   * @return true if the user canceled
   */
  bool PromptForSaving(const std::list<Saveable::Pointer>& modelsToSave,
  /*final IShellProvider shellProvider, IRunnableContext runnableContext,*/
  bool canCancel, bool stillOpenElsewhere);

  /**
   * Save the given models.
   * @param finalModels the list of models to be saved
   * @param shellProvider the provider used to obtain a shell in prompting is
   *            required. Clients can use a workbench window for this.
   * @param runnableContext a runnable context that will be used to provide a
   *            progress monitor while the save is taking place. Clients can
   *            use a workbench window for this.
   * @return <code>true</code> if the operation was canceled
   */
  bool SaveModels(const std::list<Saveable::Pointer>& finalModels
  /*final IShellProvider shellProvider, IRunnableContext runnableContext*/);

  /**
   * @param postCloseInfoObject
   */
  void PostClose(PostCloseInfo::Pointer postCloseInfoObject);

  /**
   * @param actualPart
   */
  void PostOpen(IWorkbenchPart::Pointer part);

  /**
   * @param actualPart
   */
  void DirtyChanged(IWorkbenchPart::Pointer part);

  /**
   * For testing purposes. Not to be called by clients.
   *
   * @param model
   * @return
   */
  std::vector<Object::Pointer> TestGetSourcesForModel(Saveable::Pointer model);

  /**
   * @return a list of ISaveablesSource objects registered with this saveables
   *         list which are not workbench parts.
   */
  std::vector<ISaveablesSource::Pointer> GetNonPartSources();

  /**
   * @param model
   */
  std::vector<IWorkbenchPart::Pointer> GetPartsForSaveable(
      Saveable::Pointer model);

};

}

#endif /* CHERRYSAVEABLESLIST_H_ */
