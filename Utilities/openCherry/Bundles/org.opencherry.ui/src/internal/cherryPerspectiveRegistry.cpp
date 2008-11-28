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

#include "cherryPerspectiveRegistry.h"

#include "cherryWorkbench.h"
#include "cherryWorkbenchPlugin.h"
#include "cherryPerspectiveRegistryReader.h"

#include <Poco/String.h>

namespace cherry
{

const std::string PerspectiveRegistry::EXT = "_persp.xml";
const std::string PerspectiveRegistry::ID_DEF_PERSP = "PerspectiveRegistry.DEFAULT_PERSP";
const std::string PerspectiveRegistry::PERSP = "_persp";
const char PerspectiveRegistry::SPACE_DELIMITER = ' ';

PerspectiveRegistry::PerspectiveRegistry()
{
  //IExtensionTracker tracker = PlatformUI.getWorkbench() .getExtensionTracker();
  //tracker.registerHandler(this, null);

  //this->InitializePreferenceChangeListener();
  //WorkbenchPlugin::GetDefault()->GetPreferenceStore()->AddPropertyChangeListener(
  //    preferenceListener);

}

void PerspectiveRegistry::AddPerspective(PerspectiveDescriptor::Pointer desc)
{
  if (desc == 0)
  {
    return;
  }
  this->Add(desc);
}

PerspectiveDescriptor::Pointer PerspectiveRegistry::CreatePerspective(const std::string& label,
    PerspectiveDescriptor::Pointer originalDescriptor)
{
  // Sanity check to avoid invalid or duplicate labels.
  if (!this->ValidateLabel(label))
  {
    return 0;
  }
  if (this->FindPerspectiveWithLabel(label) != 0)
  {
    return 0;
  }

  // Calculate ID.
  std::string id(label);
  std::replace(id.begin(), id.end(), ' ', '_');
  Poco::trimInPlace(id);

  // Create descriptor.
  PerspectiveDescriptor::Pointer desc =
      new PerspectiveDescriptor(id, label, originalDescriptor);
  this->Add(desc);
  return desc;
}

void PerspectiveRegistry::RevertPerspectives(
    const std::list<PerspectiveDescriptor::Pointer>& perspToRevert)
{
  // indicate that the user is removing these perspectives
  for (std::list<PerspectiveDescriptor::Pointer>::const_iterator iter = perspToRevert.begin();
       iter != perspToRevert.end(); ++iter)
  {
    PerspectiveDescriptor::Pointer desc = *iter;
    perspToRemove.push_back(desc->GetId());
    desc->RevertToPredefined();
  }
}

void PerspectiveRegistry::DeletePerspectives(
    const std::list<PerspectiveDescriptor::Pointer>& perspToDelete)
{
  for (std::list<PerspectiveDescriptor::Pointer>::const_iterator iter = perspToDelete.begin();
       iter != perspToDelete.end(); ++iter)
  {
    this->DeletePerspective(*iter);
  }
}

void PerspectiveRegistry::DeletePerspective(IPerspectiveDescriptor::Pointer in)
{
  PerspectiveDescriptor::Pointer desc = in.Cast<PerspectiveDescriptor>();
  // Don't delete predefined perspectives
  if (!desc->IsPredefined())
  {
    perspToRemove.push_back(desc->GetId());
    perspectives.remove(desc);
    desc->DeleteCustomDefinition();
    this->VerifyDefaultPerspective();
  }
}

IPerspectiveDescriptor::Pointer PerspectiveRegistry::FindPerspectiveWithId(const std::string& id)
{
  for (std::list<PerspectiveDescriptor::Pointer>::iterator iter = perspectives.begin();
       iter != perspectives.end(); ++iter)
  {
    PerspectiveDescriptor::Pointer desc = *iter;
    if (desc->GetId() == id)
    {
//      if (WorkbenchActivityHelper.restrictUseOf(desc))
//      {
//        return null;
//      }
      return desc;
    }
  }

  return 0;
}

IPerspectiveDescriptor::Pointer PerspectiveRegistry::FindPerspectiveWithLabel(
    const std::string& label)
{
  for (std::list<PerspectiveDescriptor::Pointer>::iterator iter = perspectives.begin();
       iter != perspectives.end(); ++iter)
  {
    PerspectiveDescriptor::Pointer desc = *iter;
    if (desc->GetLabel() == label)
    {
//      if (WorkbenchActivityHelper.restrictUseOf(desc))
//      {
//        return 0;
//      }
      return desc;
    }
  }
  return 0;
}

std::string PerspectiveRegistry::GetDefaultPerspective()
{
  return defaultPerspID;
}

std::vector<IPerspectiveDescriptor::Pointer> PerspectiveRegistry::GetPerspectives()
{
//  Collection descs = WorkbenchActivityHelper.restrictCollection(perspectives,
//      new ArrayList());
//  return (IPerspectiveDescriptor[]) descs.toArray(
//      new IPerspectiveDescriptor[descs.size()]);

  std::vector<IPerspectiveDescriptor::Pointer> result;
  for (std::list<PerspectiveDescriptor::Pointer>::iterator iter = perspectives.begin();
         iter != perspectives.end(); ++iter)
  {
    result.push_back(iter->Cast<IPerspectiveDescriptor>());
  }
  return result;
}

void PerspectiveRegistry::Load()
{
  // Load the registries.
  this->LoadPredefined();
  this->LoadCustom();

  // Get default perspective.
  // Get it from the R1.0 dialog settings first. Fixes bug 17039
//  IDialogSettings dialogSettings =
//      WorkbenchPlugin.getDefault() .getDialogSettings();
//  std::string str = dialogSettings.get(ID_DEF_PERSP);
//  if (str != null && str.length() > 0)
//  {
//    this->SetDefaultPerspective(str);
//    dialogSettings.put(ID_DEF_PERSP, ""); //$NON-NLS-1$
//  }

  this->VerifyDefaultPerspective();
}

//void PerspectiveRegistry::SaveCustomPersp(PerspectiveDescriptor::Pointer desc,
//    XMLMemento::Pointer memento)
//{
//
//  IPreferenceStore store = WorkbenchPlugin.getDefault() .getPreferenceStore();
//
//  // Save it to the preference store.
//  Writer writer = new StringWriter();
//
//  memento.save(writer);
//  writer.close();
//  store.setValue(desc.getId() + PERSP, writer.toString());
//
//}

IMemento::Pointer PerspectiveRegistry::GetCustomPersp(const std::string& id)
{
  //TODO CustomPersp
//  Reader reader = null;
//
//  IPreferenceStore store = WorkbenchPlugin.getDefault() .getPreferenceStore();
//  std::string xmlString = store.getString(id + PERSP);
//  if (xmlString != null && xmlString.length() != 0)
//  { // defined in store
//    reader = new StringReader(xmlString);
//  }
//  XMLMemento memento = XMLMemento.createReadRoot(reader);
//  reader.close();
//  return memento;
  return 0;
}

void PerspectiveRegistry::SetDefaultPerspective(const std::string& id)
{
  IPerspectiveDescriptor::Pointer desc = this->FindPerspectiveWithId(id);
  if (desc != 0)
  {
    defaultPerspID = id;
    //TODO Preferences
//    PrefUtil.getAPIPreferenceStore().setValue(
//        IWorkbenchPreferenceConstants.DEFAULT_PERSPECTIVE_ID, id);
  }
}

bool PerspectiveRegistry::ValidateLabel(const std::string& label)
{
  return !Poco::trim(label).empty();
}

IPerspectiveDescriptor::Pointer PerspectiveRegistry::ClonePerspective(const std::string& id,
    const std::string& label,
    IPerspectiveDescriptor::Pointer originalDescriptor)
{

  // Check for invalid labels
  if (label == "" || Poco::trim(label).empty())
  {
    throw Poco::InvalidArgumentException();
  }

  // Check for duplicates
  IPerspectiveDescriptor::Pointer desc = this->FindPerspectiveWithId(id);
  if (desc != 0)
  {
    throw Poco::InvalidArgumentException();
  }

  // Create descriptor.
  desc
      = new PerspectiveDescriptor(id, label, originalDescriptor.Cast<PerspectiveDescriptor>());
  this->Add(desc.Cast<PerspectiveDescriptor>());
  return desc;
}

void PerspectiveRegistry::RevertPerspective(IPerspectiveDescriptor::Pointer perspToRevert)
{
  PerspectiveDescriptor::Pointer desc = perspToRevert.Cast<PerspectiveDescriptor>();
  perspToRemove.push_back(desc->GetId());
  desc->RevertToPredefined();
}

PerspectiveRegistry::~PerspectiveRegistry()
{
//  PlatformUI.getWorkbench().getExtensionTracker().unregisterHandler(this);
//  WorkbenchPlugin.getDefault().getPreferenceStore() .removePropertyChangeListener(
//      preferenceListener);
}

void PerspectiveRegistry::DeleteCustomDefinition(PerspectiveDescriptor::Pointer desc)
{
  //TODO Preferences
  // remove the entry from the preference store.
  //IPreferenceStore store = WorkbenchPlugin.getDefault() .getPreferenceStore();

  /*
   * To delete the perspective definition from the preference store, use
   * the setToDefault method. Since no default is defined, this will
   * remove the entry
   */
  //store.setToDefault(desc.getId() + PERSP);

}

bool PerspectiveRegistry::HasCustomDefinition(PerspectiveDescriptor::Pointer desc)
{
  //TODO Preferences
  //IPreferenceStore store = WorkbenchPlugin::GetDefault()->GetPreferenceStore();
  //return store.contains(desc.getId() + PERSP);
  return false;
}

void PerspectiveRegistry::InitializePreferenceChangeListener()
{
//  preferenceListener = new IPropertyChangeListener()
//  {
//  public void propertyChange(PropertyChangeEvent event)
//    {
//      /*
//       * To ensure the that no custom perspective definitions are
//       * deleted when preferences are imported, merge old and new
//       * values
//       */
//      if (event.getProperty().endsWith(PERSP))
//      {
//        /* A Perspective is being changed, merge */
//        mergePerspectives(event);
//      }
//      else if (event.getProperty().equals(
//              IPreferenceConstants.PERSPECTIVES))
//      {
//        /* The list of perpsectives is being changed, merge */
//        updatePreferenceList((IPreferenceStore) event.getSource());
//      }
//    }
//
//    void MergePerspectives(PropertyChangeEvent::Pointer event)
//    {
//      IPreferenceStore store = (IPreferenceStore) event.getSource();
//      if (event.getNewValue() == null
//          || event.getNewValue().equals(""))
//      { //$NON-NLS-1$
//        /*
//         * Perpsective is being removed; if the user has deleted or
//         * reverted a custom perspective, let the change pass
//         * through. Otherwise, restore the custom perspective entry
//         */
//
//        // Find the matching descriptor in the registry
//        IPerspectiveDescriptor[] perspectiveList = getPerspectives();
//        for (int i = 0; i < perspectiveList.length; i++)
//        {
//          std::string id = perspectiveList[i].getId();
//          if (event.getProperty().equals(id + PERSP))
//          { // found
//            // descriptor
//            // see if the perspective has been flagged for
//            // reverting or deleting
//            if (!perspToRemove.contains(id))
//            { // restore
//              store.setValue(id + PERSP, (std::string) event
//                  .getOldValue());
//            }
//            else
//            { // remove element from the list
//              perspToRemove.remove(id);
//            }
//          }
//        }
//      }
//      else if ((event.getOldValue() == null || event.getOldValue()
//              .equals("")))
//      { //$NON-NLS-1$
//
//        /*
//         * New perspective is being added, update the
//         * perspectiveRegistry to contain the new custom perspective
//         */
//
//        std::string id = event.getProperty().substring(0,
//            event.getProperty().lastIndexOf(PERSP));
//        if (findPerspectiveWithId(id) == null)
//        {
//          // perspective does not already exist in registry, add
//          // it
//          PerspectiveDescriptor desc = new PerspectiveDescriptor(
//              null, null, null);
//          StringReader reader = new StringReader((std::string) event
//              .getNewValue());
//          try
//          {
//            XMLMemento memento = XMLMemento
//            .createReadRoot(reader);
//            desc.restoreState(memento);
//            addPerspective(desc);
//          }
//          catch (WorkbenchException e)
//          {
//            unableToLoadPerspective(e.getStatus());
//          }
//        }
//      }
//      /* If necessary, add to the list of perspectives */
//      updatePreferenceList(store);
//    }
//
//    void UpdatePreferenceList(IPreferenceStore store)
//    {
//      IPerspectiveDescriptor[] perspectiveList = getPerspectives();
//      StringBuffer perspBuffer = new StringBuffer();
//      for (int i = 0; i < perspectiveList.length; i++)
//      {
//        PerspectiveDescriptor desc = (PerspectiveDescriptor) perspectiveList[i];
//        if (hasCustomDefinition(desc))
//        {
//          perspBuffer.append(desc.getId())
//          .append(SPACE_DELIMITER);
//        }
//      }
//      std::string newList = perspBuffer.toString().trim();
//      store.setValue(IPreferenceConstants.PERSPECTIVES, newList);
//    }
//  };
}

void PerspectiveRegistry::Add(PerspectiveDescriptor::Pointer desc)
{
  perspectives.push_back(desc);
//  IConfigurationElement::Pointer element = desc->GetConfigElement();
//  if (element != 0)
//  {
//    PlatformUI::GetWorkbench().getExtensionTracker().registerObject(
//        element.getDeclaringExtension(), desc, IExtensionTracker.REF_WEAK);
//  }
}

void PerspectiveRegistry::InternalDeletePerspective(PerspectiveDescriptor::Pointer desc)
{
  perspToRemove.push_back(desc->GetId());
  perspectives.remove(desc);
  desc->DeleteCustomDefinition();
  this->VerifyDefaultPerspective();
}

void PerspectiveRegistry::LoadCustom()
{
//  Reader reader = null;
//
//  /* Get the entries from the Preference store */
//  IPreferenceStore store = WorkbenchPlugin.getDefault() .getPreferenceStore();
//
//  /* Get the space-delimited list of custom perspective ids */
//  std::string customPerspectives = store .getString(
//      IPreferenceConstants.PERSPECTIVES);
//  std::string[] perspectivesList = StringConverter.asArray(customPerspectives);
//
//  for (int i = 0; i < perspectivesList.length; i++)
//  {
//    try
//    {
//      std::string xmlString = store.getString(perspectivesList[i] + PERSP);
//      if (xmlString != null && xmlString.length() != 0)
//      {
//        reader = new StringReader(xmlString);
//      }
//
//      // Restore the layout state.
//      XMLMemento memento = XMLMemento.createReadRoot(reader);
//      PerspectiveDescriptor newPersp =
//          new PerspectiveDescriptor(null, null, null);
//      newPersp.restoreState(memento);
//      std::string id = newPersp.getId();
//      IPerspectiveDescriptor oldPersp = findPerspectiveWithId(id);
//      if (oldPersp == null)
//      {
//        add(newPersp);
//      }
//      reader.close();
//    } catch (IOException e)
//    {
//      unableToLoadPerspective(null);
//    } catch (WorkbenchException e)
//    {
//      unableToLoadPerspective(e.getStatus());
//    }
//  }
//
//  // Get the entries from files, if any
//  // if -data @noDefault specified the state location may not be
//  // initialized
//  IPath path = WorkbenchPlugin.getDefault().getDataLocation();
//  if (path == null)
//  {
//    return;
//  }
//
//  File folder = path.toFile();
//
//  if (folder.isDirectory())
//  {
//    File[] fileList = folder.listFiles();
//    int nSize = fileList.length;
//    for (int nX = 0; nX < nSize; nX++)
//    {
//      File file = fileList[nX];
//      if (file.getName().endsWith(EXT))
//      {
//        // get the memento
//        InputStream stream = null;
//        try
//        {
//          stream = new FileInputStream(file);
//          reader = new BufferedReader(new InputStreamReader(stream, "utf-8")); //$NON-NLS-1$
//
//          // Restore the layout state.
//          XMLMemento memento = XMLMemento.createReadRoot(reader);
//          PerspectiveDescriptor newPersp =
//              new PerspectiveDescriptor(null, null, null);
//          newPersp.restoreState(memento);
//          IPerspectiveDescriptor oldPersp = findPerspectiveWithId(
//              newPersp .getId());
//          if (oldPersp == null)
//          {
//            add(newPersp);
//          }
//
//          // save to the preference store
//          saveCustomPersp(newPersp, memento);
//
//          // delete the file
//          file.delete();
//
//          reader.close();
//          stream.close();
//        } catch (IOException e)
//        {
//          unableToLoadPerspective(null);
//        } catch (WorkbenchException e)
//        {
//          unableToLoadPerspective(e.getStatus());
//        }
//      }
//    }
//  }
}

void PerspectiveRegistry::UnableToLoadPerspective(const std::string& status)
{
  std::string msg = "Unable to load perspective";
  if (status == "")
  {
    WorkbenchPlugin::Log(msg);
    //IStatus errStatus =
    //    new Status(IStatus.ERR, WorkbenchPlugin.PI_WORKBENCH, msg);
    //StatusManager.getManager().handle(errStatus, StatusManager.SHOW);
  }
  else
  {
    WorkbenchPlugin::Log(status + ": " + msg);
    //IStatus errStatus = StatusUtil.newStatus(status, msg);
    //StatusManager.getManager().handle(errStatus, StatusManager.SHOW);
  }
}

void PerspectiveRegistry::LoadPredefined()
{
  PerspectiveRegistryReader reader;
  reader.ReadPerspectives(this);
}

void PerspectiveRegistry::VerifyDefaultPerspective()
{
  // Step 1: Try current defPerspId value.
  IPerspectiveDescriptor::Pointer desc;
  if (defaultPerspID != "")
  {
    desc = this->FindPerspectiveWithId(defaultPerspID);
  }
  if (desc != 0)
  {
    return;
  }

  // Step 2. Read default value.
  //TODO Preferences
//  std::string str = PrefUtil.getAPIPreferenceStore().getString(
//      IWorkbenchPreferenceConstants.DEFAULT_PERSPECTIVE_ID);
//  if (str != null && str.length() > 0)
//  {
//    desc = this->FindPerspectiveWithId(str);
//  }
//  if (desc != 0)
//  {
//    defaultPerspID = str;
//    return;
//  }

  // Step 3. Use application-specific default
  defaultPerspID = Workbench::GetInstance()->GetDefaultPerspectiveId();
}

}
