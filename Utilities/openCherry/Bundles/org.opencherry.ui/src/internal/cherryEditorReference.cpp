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

#include "cherryEditorReference.h"

#include "cherryEditorManager.h"
#include "cherryEditorDescriptor.h"
#include "cherryEditorRegistry.h"
#include "cherryEditorSite.h"
#include "cherryEditorAreaHelper.h"
#include "cherryWorkbenchPlugin.h"
#include "cherryWorkbenchPage.h"
#include "cherryNullEditorInput.h"
#include "cherryPartTester.h"
#include "../tweaklets/cherryWorkbenchPageTweaklet.h"

#include "../cherryImageDescriptor.h"
#include "../cherryPlatformUI.h"

namespace cherry
{

EditorReference::EditorReference(EditorManager* man,
    IEditorInput::Pointer input, EditorDescriptor::Pointer desc,
    IMemento::Pointer editorState) :
  manager(man), expectingInputChange(false),
      reportedMalfunctioningEditor(false)
{
  this->InitListenersAndHandlers();
  restoredInput = input;
  this->editorState = editorState;
  this->Init(desc->GetId(), "", desc->GetImageDescriptor(),
  desc->GetLabel(), "");
}

EditorReference::EditorReference(EditorManager* man, IMemento::Pointer memento) :
  manager(man), expectingInputChange(false),
      reportedMalfunctioningEditor(false)
{
  this->InitListenersAndHandlers();
  this->editorMemento = memento;
  if (manager->UseIPersistableEditor())
  {
    //editorState = editorMemento->GetChild(WorkbenchConstants::TAG_EDITOR_STATE);
  }
  else
  {
    editorState = 0;
  }
  //  String id = memento.getString(IWorkbenchConstants.TAG_ID);
  //  String title = memento.getString(IWorkbenchConstants.TAG_TITLE);
  //  String tooltip = Util.safeString(memento
  //  .getString(IWorkbenchConstants.TAG_TOOLTIP));
  //  String partName = memento
  //  .getString(IWorkbenchConstants.TAG_PART_NAME);
  //
  //  IMemento propBag = memento.getChild(IWorkbenchConstants.TAG_PROPERTIES);
  //  if (propBag != null)
  //  {
  //    IMemento[] props = propBag
  //    .getChildren(IWorkbenchConstants.TAG_PROPERTY);
  //    for (int i = 0; i < props.length; i++)
  //    {
  //      propertyCache.put(props[i].getID(), props[i].getTextData());
  //    }
  //  }

  // For compatibility set the part name to the title if not found

  //  if (partName.empty())
  //  {
  //    partName = title;
  //  }

  // Get the editor descriptor.
  //  EditorDescriptor::Pointer desc;
  //  if (id != null)
  //  {
  //    desc = getDescriptor(id);
  //  }
  //  // desc may be null if id is null or desc is not found, but findImage below handles this
  //  String location = memento.getString(IWorkbenchConstants.TAG_PATH);
  //  IPath path = location == null ? null : new Path(location);
  //  ImageDescriptor iDesc = this.manager.findImage(desc, path);
  //
  //  this.name = memento.getString(IWorkbenchConstants.TAG_NAME);
  //  if (this.name == null)
  //  {
  //    this.name = title;
  //  }
  //  setPinned("true".equals(memento.getString(IWorkbenchConstants.TAG_PINNED))); //$NON-NLS-1$
  //
  //  IMemento inputMem = memento.getChild(IWorkbenchConstants.TAG_INPUT);
  //  if (inputMem != null)
  //  {
  //    this.factoryId = inputMem
  //    .getString(IWorkbenchConstants.TAG_FACTORY_ID);
  //  }
  //
  //  init(id, title, tooltip, iDesc, partName, ""); //$NON-NLS-1$
}

EditorDescriptor::Pointer EditorReference::GetDescriptor()
{
  return this->GetDescriptor(this->GetId());
}

EditorDescriptor::Pointer EditorReference::GetDescriptor(const std::string& id)
{
  EditorDescriptor::Pointer desc;
  IEditorRegistry* reg = WorkbenchPlugin::GetDefault()->GetEditorRegistry();
  desc = reg->FindEditor(id).Cast<EditorDescriptor> ();
  return desc;
}

void EditorReference::InitListenersAndHandlers()
{
  // Create a property change listener to track the "close editors automatically"
  // preference and show/remove the pin icon on editors
  // Only 1 listener will be created in the EditorManager when necessary
  //this->manager->CheckCreateEditorPropListener();
  // Create a keyboard shortcut handler for pinning editors
  // Only 1 handler will be created in the EditorManager when necessary
  //this->manager->CheckCreatePinEditorShortcutKeyHandler();
}

PartPane::Pointer EditorReference::CreatePane()
{
  PartPane::Pointer pane(
      new PartPane(IWorkbenchPartReference::Pointer(this), this->manager->page));
  return pane;
  //return Tweaklets::Get(WorkbenchTweaklet::KEY)->CreateEditorPane(this,
  //    this->manager->page);
}

void EditorReference::PinStatusUpdated()
{
  //firePropertyChange(IWorkbenchPart.PROP_TITLE);
}

std::string EditorReference::GetFactoryId()
{
  //  IEditorPart editor = getEditor(false);
  //  if (editor != null)
  //  {
  //    IPersistableElement persistable = editor.getEditorInput()
  //    .getPersistable();
  //    if (persistable != null)
  //    {
  //      return persistable.getFactoryId();
  //    }
  //    return null;
  //  }
  //  return factoryId;
  return "";
}

std::string EditorReference::ComputePartName()
{
  return WorkbenchPartReference::ComputePartName();
}

std::string EditorReference::GetName()
{
  if (part.IsNotNull())
  {
    return this->GetEditor(false)->GetEditorInput()->GetName();
  }
  return name;
}

IEditorPart::Pointer EditorReference::GetEditor(bool restore)
{
  return this->GetPart(restore).Cast<IEditorPart> ();
}

void EditorReference::SetName(const std::string& name)
{
  this->name = name;
}

IMemento::Pointer EditorReference::GetMemento()
{
  return editorMemento;
}

IWorkbenchPage::Pointer EditorReference::GetPage() const
{
  return this->manager->page;
}

IEditorInput::Pointer EditorReference::GetEditorInput()
{
  IEditorPart::Pointer part = this->GetEditor(false);
  if (part.IsNotNull())
  {
    return part->GetEditorInput();
  }
  return this->GetRestoredInput();
}

IEditorInput::Pointer EditorReference::GetRestoredInput()
{
  if (restoredInput.IsNotNull())
  {
    return restoredInput;
  }

  // Get the input factory.
  //    IMemento::Pointer editorMem = this->GetMemento();
  //    if (editorMem == null)
  //    {
  //      throw new PartInitException(NLS.bind(WorkbenchMessages.EditorManager_no_persisted_state, getId(), getName()));
  //    }
  //    IMemento inputMem = editorMem
  //    .getChild(IWorkbenchConstants.TAG_INPUT);
  //    String factoryID = null;
  //    if (inputMem != null)
  //    {
  //      factoryID = inputMem
  //      .getString(IWorkbenchConstants.TAG_FACTORY_ID);
  //    }
  //    if (factoryID == null)
  //    {
  //      throw new PartInitException(NLS.bind(WorkbenchMessages.EditorManager_no_input_factory_ID, getId(), getName()));
  //    }
  //    IAdaptable input = null;
  //    String label = null; // debugging only
  //    if (UIStats.isDebugging(UIStats.CREATE_PART_INPUT))
  //    {
  //      label = getName() != null ? getName() : factoryID;
  //    }
  //    try
  //    {
  //      UIStats.start(UIStats.CREATE_PART_INPUT, label);
  //      IElementFactory factory = PlatformUI.getWorkbench()
  //      .getElementFactory(factoryID);
  //      if (factory == null)
  //      {
  //        throw new PartInitException(NLS.bind(WorkbenchMessages.EditorManager_bad_element_factory, new Object[]
  //                { factoryID, getId(), getName()}));
  //      }
  //
  //      // Get the input element.
  //      input = factory.createElement(inputMem);
  //      if (input == null)
  //      {
  //        throw new PartInitException(NLS.bind(WorkbenchMessages.EditorManager_create_element_returned_null, new Object[]
  //                { factoryID, getId(), getName()}));
  //      }
  //    }finally
  //    {
  //      UIStats.end(UIStats.CREATE_PART_INPUT, input, label);
  //    }
  //    if (!(input instanceof IEditorInput))
  //    {
  //      throw new PartInitException(NLS.bind(WorkbenchMessages.EditorManager_wrong_createElement_result, new Object[]
  //              { factoryID, getId(), getName()}));
  //    }
  //    restoredInput = (IEditorInput) input;
  return restoredInput;
}

IWorkbenchPart::Pointer EditorReference::CreatePart()
{
  if (EditorRegistry::EMPTY_EDITOR_ID == this->GetId())
  {
    return this->GetEmptyEditor(this->GetDescriptor());
  }

  IWorkbenchPart::Pointer result;

  // Try to restore the editor -- this does the real work of restoring the editor
  //
  try
  {
    result = this->CreatePartHelper().Cast<IWorkbenchPart> ();
  } catch (PartInitException e)
  {
    // If unable to create the part, create an error part instead
    // and pass the error to the status handling facility
    //    IStatus originalStatus = exception.getStatus();
    //    IStatus logStatus = StatusUtil.newStatus(originalStatus,
    //        NLS.bind("Unable to create editor ID {0}: {1}", //$NON-NLS-1$
    //            getId(), originalStatus.getMessage()));
    //    IStatus displayStatus = StatusUtil.newStatus(originalStatus,
    //        NLS.bind(WorkbenchMessages.EditorManager_unableToCreateEditor,
    //            originalStatus.getMessage()));
    WorkbenchPlugin::Log("Unable to create editor ID " + this->GetId() + ": "
        + e.displayText());

    // Pass the error to the status handling facility
    //StatusManager.getManager().handle(logStatus);

    EditorDescriptor::Pointer descr = this->GetDescriptor();
    std::string label = this->GetId();
    if (descr.IsNotNull())
      label = descr->GetLabel();

    IEditorPart::Pointer part =
        Tweaklets::Get(WorkbenchPageTweaklet::KEY)->CreateErrorEditorPart(label,
            e.displayText());
    if (part.IsNotNull())
    {
      IEditorInput::Pointer input;
      try
      {
        input = this->GetEditorInput();
      } catch (PartInitException e1)
      {
        input = new NullEditorInput(EditorReference::Pointer(this));
      }

      PartPane::Pointer pane = this->GetPane();

      pane->CreateControl(
          manager->page->GetEditorPresentation()->GetLayoutPart()->GetControl());

      EditorSite::Pointer site(
          new EditorSite(IEditorReference::Pointer(this), part, manager->page, descr));

      //site.setActionBars(new EditorActionBars(manager.page, site.getWorkbenchWindow(), getId()));

      part->Init(site, input);

      try
      {
        part->CreatePartControl(pane->GetControl());
      } catch (...)
      {
        //content.dispose();
        //StatusUtil.handleStatus(e, StatusManager.SHOW
        //    | StatusManager.LOG);
        WorkbenchPlugin::Log("Error creating editor");
        return IWorkbenchPart::Pointer(0);
      }

      result = part.Cast<IWorkbenchPart> ();
    }
  }

  return result;
}

bool EditorReference::SetInput(IEditorInput::Pointer input)
{

  if (part.IsNotNull())
  {
    if (part.Cast<IReusableEditor> ().IsNotNull())
    {
      IReusableEditor::Pointer editor = part.Cast<IReusableEditor> ();

      expectingInputChange = true;

      editor->SetInput(input);

      // If the editor never fired a PROP_INPUT event, log the fact that we've discovered
      // a buggy editor and fire the event for free. Firing the event for free isn't required
      // and cannot be relied on (it only works if the input change was triggered by this
      // method, and there are definitely other cases where events will still be lost),
      // but older versions of the workbench did this so we fire it here in the spirit
      // of playing nice.
      if (expectingInputChange)
      {

        // Log the fact that this editor is broken
        this->ReportMalfunction(
            "Editor is not firing a PROP_INPUT event in response to IReusableEditor.setInput(...)"); //$NON-NLS-1$

        // Fire the property for free (can't be relied on since there are other ways the input
        // can change, but we do it here to be consistent with older versions of the workbench)
        //firePropertyChange(IWorkbenchPartConstants.PROP_INPUT);
      }

      return editor->GetEditorInput() == input;

    }
    // Can't change the input if the editor already exists and isn't an IReusableEditor
    return false;
  }

  // Changing the input is trivial and always succeeds if the editor doesn't exist yet
  if (input != restoredInput)
  {
    restoredInput = input;

    //firePropertyChange(IWorkbenchPartConstants.PROP_INPUT);
  }

  return true;
}

void EditorReference::ReportMalfunction(const std::string& string)
{
  if (!reportedMalfunctioningEditor)
  {
    reportedMalfunctioningEditor = true;

    std::string errorMessage = "Problem detected with part " + this->GetId(); //$NON-NLS-1$
    if (part.IsNotNull())
    {
      errorMessage.append("(class = ").append(part->GetNameOfClass()).append(
          ")"); //$NON-NLS-1$ //$NON-NLS-2$
    }

    errorMessage += ": " + string; //$NON-NLS-1$

    //StatusManager.getManager().handle(StatusUtil.newStatus(getDescriptor().getPluginId(), errorMessage, null));
    std::cout << errorMessage << std::endl;
  }
}

IEditorPart::Pointer EditorReference::CreatePartHelper()
{

  EditorSite::Pointer site;
  IEditorPart::Pointer part;

  try
  {
    IEditorInput::Pointer editorInput = this->GetEditorInput();

    // Get the editor descriptor.
    std::string editorID = this->GetId();
    EditorDescriptor::Pointer desc = this->GetDescriptor();

    if (desc.IsNull())
    {
      throw PartInitException("No editor descriptor for id " + editorID);
    }

    if (desc->IsInternal())
    {
      // Create an editor instance.
      part = manager->CreatePart(desc);

      this->CreatePartProperties(part);

    }
    //    else if (desc->GetId() == IEditorRegistry.SYSTEM_INPLACE_EDITOR_ID)
    //    {
    //
    //      part = ComponentSupport.getSystemInPlaceEditor();
    //
    //      if (part == null)
    //      {
    //        throw new PartInitException(WorkbenchMessages.EditorManager_no_in_place_support);
    //      }
    //    }
    else
    {
      throw PartInitException("Invalid editor descriptor for id " + editorID);
    }
    // Create a pane for this part
    PartPane::Pointer pane = this->GetPane();

    //pane->CreateControl((Composite) manager.page.getEditorPresentation().getLayoutPart().getControl());


    // Link everything up to the part reference (the part reference itself should not have
    // been modified until this point)
    site = manager->CreateSite(IEditorReference::Pointer(this), part, desc, editorInput);

    // if there is saved state that's appropriate, pass it on
    if (/*part instanceof IPersistableEditor &&*/editorState.IsNotNull())
    {
      //part->RestoreState(editorState);
    }

    // Remember the site and the action bars (now that we've created them, we'll need to dispose
    // them if an exception occurs)
    //actionBars = (EditorActionBars) site.getActionBars();

    part->CreatePartControl(pane->GetControl());

    // The editor should now be fully created. Exercise its public interface, and sanity-check
    // it wherever possible. If it's going to throw exceptions or behave badly, it's much better
    // that it does so now while we can still cancel creation of the part.
    PartTester::TestEditor(part);

    return part;

  } catch (std::exception e)
  {
    throw PartInitException(e.what());
  }

}

IEditorPart::Pointer EditorReference::GetEmptyEditor(
    EditorDescriptor::Pointer descr)
{
  IEditorPart::Pointer part =
      Tweaklets::Get(WorkbenchPageTweaklet::KEY)->CreateErrorEditorPart("(Empty)");

  IEditorInput::Pointer input;
  try
  {
    input = this->GetEditorInput();
  } catch (PartInitException e1)
  {
    input = new NullEditorInput(EditorReference::Pointer(this));
  }

  PartPane::Pointer pane = this->GetPane();

  pane->CreateControl(
      manager->page->GetEditorPresentation()->GetLayoutPart()->GetControl());

  EditorSite::Pointer site(new EditorSite(IEditorReference::Pointer(this),
                                          part, manager->page, descr));

  //site.setActionBars(new EditorActionBars(manager.page, site.getWorkbenchWindow(), getId()));

  part->Init(site, input);

  try
  {
    part->CreatePartControl(pane->GetControl());
  } catch (std::exception e)
  {
    //StatusManager.getManager().handle(
    //    StatusUtil.newStatus(WorkbenchPlugin.PI_WORKBENCH, e));
    std::cout << e.what() << std::endl;
    return IEditorPart::Pointer(0);
  }

  this->part = part.Cast<IWorkbenchPart> ();
  // Add a dispose listener to the part. This dispose listener does nothing but log an exception
  // if the part's widgets get disposed unexpectedly. The workbench part reference is the only
  // object that should dispose this control, and it will remove the listener before it does so.

  this->RefreshFromPart();
  //this->ReleaseReferences();

  if (this->GetPage().Cast<WorkbenchPage> ()->GetActiveEditorReference()
      != this)
  {
    //fireInternalPropertyChange(INTERNAL_PROPERTY_OPENED);
  }

  return part;
}

}
