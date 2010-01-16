/*=========================================================================

Program:   BlueBerry Platform
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

#ifndef BERRYEDITORMANAGER_H_
#define BERRYEDITORMANAGER_H_

#include <berrySmartPointer.h>

#include "../berryIEditorPart.h"
#include "../berryIEditorReference.h"
#include "../berryIPathEditorInput.h"
#include "../berryIMemento.h"
#include "berryEditorDescriptor.h"

#include <list>
#include <vector>

namespace berry
{

struct IWorkbenchPart;
struct IWorkbenchWindow;
class WorkbenchWindow;
class WorkbenchPage;
class EditorSite;
class EditorReference;
struct IEditorRegistry;
class EditorAreaHelper;

/**
 * \ingroup org_blueberry_ui_internal
 *
 * Manage a group of element editors. Prevent the creation of two editors on the
 * same element.
 *
 * 06/12/00 - DS - Given the ambiguous editor input type, the manager delegates
 * a number of responsibilities to the editor itself.
 *
 * <ol>
 * <li>The editor should determine its own title.</li>
 * <li>The editor should listen to resource deltas and close itself if the
 * input is deleted. It may also choose to stay open if the editor has dirty
 * state.</li>
 * <li>The editor should persist its own state plus editor input.</li>
 * </ol>
 */
class EditorManager
{ // implements IExtensionChangeHandler {

  friend class EditorReference;

  EditorAreaHelper* editorPresentation;

  //TODO WeakPointer
  SmartPointer<WorkbenchWindow> window;

  //TODO WeakPointer
  SmartPointer<WorkbenchPage> page;

  //std::map<std::string, EditorActionBars::Pointer> actionCache = new HashMap();

  static const std::string PIN_EDITOR_KEY; // = "PIN_EDITOR"; //$NON-NLS-1$

  //static const std::string PIN_EDITOR = "ovr16/pinned_ovr.gif"; //$NON-NLS-1$

  // When the user removes or adds the close editors automatically preference
  // the icon should be removed or added accordingly
  //IPropertyChangeListener editorPropChangeListnener = null;

  // Handler for the pin editor keyboard shortcut
  //IHandlerActivation pinEditorHandlerActivation = null;

  static const std::string RESOURCES_TO_SAVE_MESSAGE; // = "Select resources to save:";

  static const std::string SAVE_RESOURCES_TITLE; // = "Save Resources";

public:

  /**
   * EditorManager constructor comment.
   */
  EditorManager(SmartPointer<WorkbenchWindow> window,
      SmartPointer<WorkbenchPage> workbenchPage,
      EditorAreaHelper* pres);

protected:

  /**
   * Check to determine if the editor resources are no longer needed removes
   * property change listener for editors removes pin editor keyboard shortcut
   * handler disposes cached images and clears the cached images hash table
   */
  void CheckDeleteEditorResources();

  /**
   * Check to determine if the property change listener for editors should be
   * created
   */
  //void CheckCreateEditorPropListener();

  /**
   * Check to determine if the handler for the pin editor keyboard shortcut
   * should be created.
   */
  //void CheckCreatePinEditorShortcutKeyHandler();

  /**
   * Method to create the editor's pin ImageDescriptor
   *
   * @return the single image descriptor for the editor's pin icon
   */
  //  ImageDescriptor GetEditorPinImageDesc() {
  //    ImageRegistry registry = JFaceResources.getImageRegistry();
  //    ImageDescriptor pinDesc = registry.getDescriptor(PIN_EDITOR_KEY);
  //    // Avoid registering twice
  //    if (pinDesc == null) {
  //      pinDesc = WorkbenchImages.getWorkbenchImageDescriptor(PIN_EDITOR);
  //      registry.put(PIN_EDITOR_KEY, pinDesc);
  //
  //    }
  //    return pinDesc;
  //  }


private:

  /**
   * Answer a list of dirty editors.
   */
  std::vector<IEditorPart::Pointer> CollectDirtyEditors();

  /**
   * Returns whether the manager contains an editor.
   */
public:
  bool ContainsEditor(IEditorReference::Pointer ref);

  /*
   * Creates the action bars for an editor. Editors of the same type should
   * share a single editor action bar, so this implementation may return an
   * existing action bar vector.
   */
private:
  //EditorActionBars* CreateEditorActionBars(EditorDescriptor::Pointer desc,
  //    IEditorSite::Pointer site);

  /*
   * Creates the action bars for an editor.
   */
private:
  //EditorActionBars* CreateEmptyEditorActionBars(IEditorSite::Pointer site);

  /*
   * Dispose
   */
protected:
  //void DisposeEditorActionBars(EditorActionBars* actionBars);

  /**
   * Returns an open editor matching the given editor input. If none match,
   * returns <code>null</code>.
   *
   * @param input
   *            the editor input
   * @return the matching editor, or <code>null</code> if no match fond
   */
public:
  IEditorPart::Pointer FindEditor(IEditorInput::Pointer input);

  /**
   * Returns an open editor matching the given editor input and/or editor id,
   * as specified by matchFlags. If none match, returns <code>null</code>.
   *
   * @param editorId
   *            the editor id
   * @param input
   *            the editor input
   * @param matchFlags
   *            flags specifying which aspects to match
   * @return the matching editor, or <code>null</code> if no match fond
   * @since 3.1
   */
public:
  IEditorPart::Pointer FindEditor(const std::string& editorId,
      IEditorInput::Pointer input, int matchFlags);

  /**
   * Returns the open editor references matching the given editor input and/or
   * editor id, as specified by matchFlags. If none match, returns an empty
   * array.
   *
   * @param editorId
   *            the editor id
   * @param input
   *            the editor input
   * @param matchFlags
   *            flags specifying which aspects to match
   * @return the matching editor, or <code>null</code> if no match fond
   * @since 3.1
   */
public:
  std::vector<IEditorReference::Pointer> FindEditors(
      IEditorInput::Pointer input, const std::string& editorId, int matchFlags);

  /**
   * Returns an open editor matching the given editor id and/or editor input.
   * Returns <code>null</code> if none match.
   *
   * @param editorId
   *            the editor id
   * @param input
   *            the editor input
   * @param editorList
   *            a mutable list containing the references for the editors to
   *            check (warning: items may be removed)
   * @param result
   *            the list to which matching editor references should be added
   * @since 3.1
   */
private:
  void FindEditors(std::list<SmartPointer<IEditorReference> >& editorList,
      IEditorInput::Pointer input, const std::string& editorId, int matchFlags,
      std::vector<IEditorReference::Pointer>& result);

  /**
   * Answer the number of editors.
   */
public:
  std::size_t GetEditorCount();

  /*
   * Answer the editor registry.
   */
private:
  IEditorRegistry* GetEditorRegistry();

  /*
   * See IWorkbenchPage.
   */
public:
  std::vector<IEditorPart::Pointer> GetDirtyEditors();

  /*
   * See IWorkbenchPage.
   */
public:
  std::list<IEditorReference::Pointer> GetEditors();

  /*
   * See IWorkbenchPage#getFocusEditor
   */
public:
  IEditorPart::Pointer GetVisibleEditor();

  /**
   * Answer true if save is needed in any one of the editors.
   */
public:
  bool IsSaveAllNeeded();

  /*
   * Prompt the user to save the reusable editor. Return false if a new editor
   * should be opened.
   */
private:
  IEditorReference::Pointer FindReusableEditor(EditorDescriptor::Pointer desc);

  /**
   * @param editorId
   *            the editor part id
   * @param input
   *            the input
   * @param setVisible
   *            if this is to be created visible ... not used
   * @param editorState
   *            an {@link IMemento} &lt;editorState&gt; for persistable
   *            editors. Can be <code>null</code>.
   * @return a created editor reference
   * @throws PartInitException
   */
public:
  IEditorReference::Pointer OpenEditor(const std::string& editorId,
      IEditorInput::Pointer input, bool setVisible, IMemento::Pointer editorState);

  /*
   * Open a new editor
   */
public:
  IEditorReference::Pointer OpenEditorFromDescriptor(
      EditorDescriptor::Pointer desc, IEditorInput::Pointer input,
      IMemento::Pointer editorState);

  /**
   * Open a specific external editor on an file based on the descriptor.
   */
//private:
//  IEditorReference::Pointer OpenExternalEditor(EditorDescriptor::Pointer desc,
//      IEditorInput::Pointer input);

  /*
   * Opens an editor part.
   */
private: void CreateEditorTab(SmartPointer<EditorReference> ref, const std::string& workbookId);

  /*
   * Create the site and initialize it with its action bars.
   */
protected:
  SmartPointer<EditorSite> CreateSite(IEditorReference::Pointer ref,
      IEditorPart::Pointer part, EditorDescriptor::Pointer desc,
      IEditorInput::Pointer input) const;

  /*
   * See IWorkbenchPage.
   */
private:
  IEditorReference::Pointer ReuseInternalEditor(EditorDescriptor::Pointer desc,
      IEditorInput::Pointer input);

protected:
  IEditorPart::Pointer CreatePart(EditorDescriptor::Pointer desc) const;

  /**
   * Open a system external editor on the input path.
   */
//private:
//  IEditorReference::Pointer OpenSystemExternalEditor(Poco::Path location);

protected:
  //ImageDescriptor FindImage(EditorDescriptor::Pointer desc, Poco::Path path);

  /**
   * @see org.blueberry.ui.IPersistable
   */
public:
  /*IStatus*/bool RestoreState(IMemento::Pointer memento);

  /**
   * Save all of the editors in the workbench. Return true if successful.
   * Return false if the user has canceled the command.
   * @param confirm true if the user should be prompted before the save
   * @param closing true if the page is being closed
   * @param addNonPartSources true if saveables from non-part sources should be saved too.
   * @return false if the user canceled or an error occurred while saving
   */
public:
  bool SaveAll(bool confirm, bool closing, bool addNonPartSources);

  /**
   * Saves the given dirty editors and views, optionally prompting the user.
   *
   * @param dirtyParts
   *            the dirty views and editors
   * @param confirm
   *            <code>true</code> to prompt whether to save, <code>false</code>
   *            to save without prompting
   * @param closing
   *            <code>true</code> if the parts are being closed,
   *            <code>false</code> if just being saved without closing
   * @param addNonPartSources true if non-part sources should be saved too
   * @param window
   *            the window to use as the parent for the dialog that prompts to
   *            save multiple dirty editors and views
   * @return <code>true</code> on success, <code>false</code> if the user
   *         canceled the save or an error occurred while saving
   */
public:
  static bool SaveAll(const std::vector<IWorkbenchPart::Pointer>& dirtyParts,
      bool confirm, bool closing, bool addNonPartSources, SmartPointer<IWorkbenchWindow>);

  /*
   * Saves the workbench part.
   */
public:
  bool SavePart(ISaveablePart::Pointer saveable, IWorkbenchPart::Pointer part, bool confirm);

  /**
   * @see IPersistablePart
   */
public:
  /*IStatus*/bool SaveState(const IMemento::Pointer memento);

  /**
   * Shows an editor. If <code>setFocus == true</code> then give it focus,
   * too.
   *
   * @return true if the active editor was changed, false if not.
   */
public:
  bool SetVisibleEditor(IEditorReference::Pointer newEd, bool setFocus);

private:
  IPathEditorInput::Pointer GetPathEditorInput(IEditorInput::Pointer input);

  /*
   * Made public for Mylar in 3.3 - see bug 138666. Can be made private once
   * we have real API for this.
   */
private:
  void RestoreEditorState(IMemento::Pointer editorMem,
      std::vector<IEditorReference::Pointer>& visibleEditors,
      std::vector<IEditorReference::Pointer>& activeEditor);

  // for dynamic UI
protected:
  void SaveEditorState(IMemento::Pointer mem, IEditorReference::Pointer ed/*, MultiStatus res*/);

  // for dynamic UI
public:
  IMemento::Pointer GetMemento(IEditorReference::Pointer e);

  IEditorReference::Pointer OpenEmptyTab();

public:
  static bool UseIPersistableEditor();

};

}

#endif /*BERRYEDITORMANAGER_H_*/
