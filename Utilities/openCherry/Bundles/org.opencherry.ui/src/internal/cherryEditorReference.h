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

#ifndef CHERRYEDITORREFERENCE_H_
#define CHERRYEDITORREFERENCE_H_

#include "cherryWorkbenchPartReference.h"
#include "../cherryIEditorReference.h"
#include "../cherryIEditorInput.h"
#include "../cherryIMemento.h"
#include "../cherryIWorkbenchPart.h"
#include "../cherryIEditorPart.h"

namespace cherry
{

class EditorManager;
class EditorDescriptor;
class PartPane;
struct IWorkbenchPage;

/**
 * \ingroup org_opencherry_ui_internal
 *
 */
class EditorReference : public WorkbenchPartReference, public IEditorReference
{

private:
  const EditorManager* manager;

private:
  IMemento::Pointer editorMemento;

private:
  IMemento::Pointer editorState;

  /**
   * Flag that lets us detect malfunctioning editors that don't fire PROP_INPUT events.
   * It is never needed for a correctly-functioning
   */
private:
  bool expectingInputChange;

  /**
   * Flag that determines whether we've already reported that this editor is malfunctioning.
   * This prevents us from spamming the event log if we repeatedly detect the same error in
   * a particular editor. If we ever detect an editor is violating its public contract in
   * a way we can recover from (such as a missing property change event), we report the error
   * once and then silently ignore errors from the same editor.
   */
private:
  bool reportedMalfunctioningEditor;

  /**
   * User-readable name of the editor's input
   */
  std::string name;

  std::string factoryId;

  IEditorInput::Pointer restoredInput;

  /**
   * @param manager
   *            The editor manager for this reference
   * @param input
   *            our input
   * @param desc
   *            the descriptor from the declaration
   * @param editorState
   *            propogate state from another editor. Can be <code>null</code>.
   */
public:

  cherryClassMacro(EditorReference)

  EditorReference(EditorManager* manager, IEditorInput::Pointer input,
      SmartPointer<EditorDescriptor> desc, IMemento::Pointer editorState = 0);

  /**
   * Constructs a new editor reference for use by editors being restored from
   * a memento.
   */
  EditorReference(EditorManager* manager, IMemento::Pointer memento);

public:
  SmartPointer<EditorDescriptor> GetDescriptor();

  /**
   * @since 3.1
   *
   * @param id the id
   * @return the editor descriptor
   */
private:
  SmartPointer<EditorDescriptor> GetDescriptor(const std::string& id);

  /**
   * Initializes the necessary editor listeners and handlers
   */
private:
  void InitListenersAndHandlers();

protected:
  SmartPointer<PartPane> CreatePane();

  /**
   * This method is called when there should be a change in the editor pin
   * status (added or removed) so that it will ask its presentable part
   * to fire a PROP_TITLE event in order for the presentation to request
   * the new icon for this editor
   */
public:
  void PinStatusUpdated();

public:
  std::string GetFactoryId();

protected:
  std::string ComputePartName();

public:
  std::string GetName();

public:
  IEditorPart::Pointer GetEditor(bool restore);

public:
  void SetName(const std::string& name);

public:
  IMemento::Pointer GetMemento();

public:
  SmartPointer<IWorkbenchPage> GetPage();

public:
  IEditorInput::Pointer GetEditorInput();

private:
  IEditorInput::Pointer GetRestoredInput();

  /* (non-Javadoc)
   * @see org.opencherry.ui.IWorkbenchPartReference#getTitleImage()
   * This method will append a pin to the icon of the editor
   * if the "automatically close editors" option in the
   * preferences is enabled and the editor has been pinned.
   */
  //public: ImageDescriptor computeImageDescriptor() {
  //        ImageDescriptor descriptor = super.computeImageDescriptor();
  //        if (!isPinned()) {
  //      return descriptor;
  //    }
  //
  //        // Check if the pinned preference is set
  //        IPreferenceStore prefStore = WorkbenchPlugin.getDefault()
  //                .getPreferenceStore();
  //        boolean bUsePin = prefStore
  //        .getBoolean(IPreferenceConstants.REUSE_EDITORS_BOOLEAN)
  //        || ((TabBehaviour)Tweaklets.get(TabBehaviour.KEY)).alwaysShowPinAction();
  //
  //        if (!bUsePin) {
  //      return descriptor;
  //    }
  //
  //        ImageDescriptor pinDesc = this.manager.getEditorPinImageDesc();
  //        if (pinDesc == null) {
  //      return descriptor;
  //    }
  //
  //        return new OverlayIcon(descriptor, pinDesc, new Point(16, 16));
  //    }

  /**
   * Wrapper for restoring the editor. First, this delegates to busyRestoreEditorHelper
   * to do the real work of restoring the view. If unable to restore the editor, this
   * method tries to substitute an error part and return success.
   *
   * @return the created part
   */
protected:
  IWorkbenchPart::Pointer CreatePart();

  //protected: void PartPropertyChanged(Object source, int propId) {
  //
  //        // Detect badly behaved editors that don't fire PROP_INPUT events
  //        // when they're supposed to. This branch is only needed to handle
  //        // malfunctioning editors.
  //        if (propId == IWorkbenchPartConstants.PROP_INPUT) {
  //            expectingInputChange = false;
  //        }
  //
  //        super.partPropertyChanged(source, propId);
  //    }

  /**
   * Attempts to set the input of the editor to the given input. Note that the input
   * can't always be changed for an editor. Editors that don't implement IReusableEditor
   * can't have their input changed once they've been materialized.
   *
   * @since 3.1
   *
   * @param input new input
   * @return true iff the input was actually changed
   */
public:
  bool SetInput(IEditorInput::Pointer input);

  /**
   * Reports a recoverable malfunction in the system log. A recoverable malfunction would be
   * something like failure to fire an expected property change. Only the first malfunction is
   * recorded to avoid spamming the system log with repeated failures in the same editor.
   *
   * @since 3.1
   *
   * @param string
   */
private:
  void ReportMalfunction(const std::string& string);

private:
  IEditorPart::Pointer CreatePartHelper();

  /**
   * Creates and returns an empty editor (<code>ErrorEditorPart</code>).
   *
   * @param descr the editor descriptor
   * @return the empty editor part or <code>null</code> in case of an exception
   */
public:
  IEditorPart::Pointer GetEmptyEditor(SmartPointer<EditorDescriptor> descr);

};

} // namespace cherry

#endif /*CHERRYEDITORREFERENCE_H_*/
