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

#include "berryPerspectiveRegistry.h"

#include "berryWorkbench.h"
#include "berryWorkbenchPlugin.h"
#include "berryPerspectiveRegistryReader.h"

namespace berry
{

const QString PerspectiveRegistry::EXT = "_persp.xml";
const QString PerspectiveRegistry::ID_DEF_PERSP = "PerspectiveRegistry.DEFAULT_PERSP";
const QString PerspectiveRegistry::PERSP = "_persp";
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

IPerspectiveDescriptor::Pointer PerspectiveRegistry::CreatePerspective(const QString& label,
    IPerspectiveDescriptor::Pointer originalDescriptor)
{
  // Sanity check to avoid invalid or duplicate labels.
  if (!this->ValidateLabel(label))
  {
    return IPerspectiveDescriptor::Pointer(0);
  }
  if (this->FindPerspectiveWithLabel(label) != 0)
  {
    return IPerspectiveDescriptor::Pointer(0);
  }

  // Calculate ID.
  QString id(label);
  id = id.replace(' ', '_').trimmed();

  // Create descriptor.
  PerspectiveDescriptor::Pointer desc(
      new PerspectiveDescriptor(id, label, originalDescriptor.Cast<PerspectiveDescriptor>()));
  this->Add(desc);
  return IPerspectiveDescriptor::Pointer(static_cast<IPerspectiveDescriptor*>(desc.GetPointer()));
}

void PerspectiveRegistry::RevertPerspectives(
    const QList<PerspectiveDescriptor::Pointer>& perspToRevert)
{
  // indicate that the user is removing these perspectives
  for (QList<PerspectiveDescriptor::Pointer>::const_iterator iter = perspToRevert.begin();
       iter != perspToRevert.end(); ++iter)
  {
    PerspectiveDescriptor::Pointer desc = *iter;
    perspToRemove.push_back(desc->GetId());
    desc->RevertToPredefined();
  }
}

void PerspectiveRegistry::DeletePerspectives(
    const QList<PerspectiveDescriptor::Pointer>& perspToDelete)
{
  for (QList<PerspectiveDescriptor::Pointer>::const_iterator iter = perspToDelete.begin();
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
    perspectives.removeAll(desc);
    desc->DeleteCustomDefinition();
    this->VerifyDefaultPerspective();
  }
}

IPerspectiveDescriptor::Pointer PerspectiveRegistry::FindPerspectiveWithId(const QString& id)
{
  for (QList<PerspectiveDescriptor::Pointer>::iterator iter = perspectives.begin();
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

  return IPerspectiveDescriptor::Pointer(0);
}

IPerspectiveDescriptor::Pointer PerspectiveRegistry::FindPerspectiveWithLabel(
    const QString& label)
{
  for (QList<PerspectiveDescriptor::Pointer>::iterator iter = perspectives.begin();
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
  return IPerspectiveDescriptor::Pointer(0);
}

QString PerspectiveRegistry::GetDefaultPerspective()
{
  return defaultPerspID;
}

QList<IPerspectiveDescriptor::Pointer> PerspectiveRegistry::GetPerspectives()
{
//  Collection descs = WorkbenchActivityHelper.restrictCollection(perspectives,
//      new ArrayList());
//  return (IPerspectiveDescriptor[]) descs.toArray(
//      new IPerspectiveDescriptor[descs.size()]);

  QList<IPerspectiveDescriptor::Pointer> result;
  for (QList<PerspectiveDescriptor::Pointer>::iterator iter = perspectives.begin();
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
//  QString str = dialogSettings.get(ID_DEF_PERSP);
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

IMemento::Pointer PerspectiveRegistry::GetCustomPersp(const QString&  /*id*/)
{
  //TODO CustomPersp
//  Reader reader = null;
//
//  IPreferenceStore store = WorkbenchPlugin.getDefault() .getPreferenceStore();
//  QString xmlString = store.getString(id + PERSP);
//  if (xmlString != null && xmlString.length() != 0)
//  { // defined in store
//    reader = new StringReader(xmlString);
//  }
//  XMLMemento memento = XMLMemento.createReadRoot(reader);
//  reader.close();
//  return memento;
  return IMemento::Pointer(0);
}

void PerspectiveRegistry::SetDefaultPerspective(const QString& id)
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

bool PerspectiveRegistry::ValidateLabel(const QString& label)
{
  return !label.trimmed().isEmpty();
}

IPerspectiveDescriptor::Pointer PerspectiveRegistry::ClonePerspective(const QString& id,
    const QString& label,
    IPerspectiveDescriptor::Pointer originalDescriptor)
{

  // Check for invalid labels
  if (label == "" || label.trimmed().isEmpty())
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

void PerspectiveRegistry::DeleteCustomDefinition(PerspectiveDescriptor::Pointer  /*desc*/)
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

bool PerspectiveRegistry::HasCustomDefinition(PerspectiveDescriptor::ConstPointer  /*desc*/) const
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
//          QString id = perspectiveList[i].getId();
//          if (event.getProperty().equals(id + PERSP))
//          { // found
//            // descriptor
//            // see if the perspective has been flagged for
//            // reverting or deleting
//            if (!perspToRemove.contains(id))
//            { // restore
//              store.setValue(id + PERSP, (QString) event
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
//        QString id = event.getProperty().substring(0,
//            event.getProperty().lastIndexOf(PERSP));
//        if (findPerspectiveWithId(id) == null)
//        {
//          // perspective does not already exist in registry, add
//          // it
//          PerspectiveDescriptor desc = new PerspectiveDescriptor(
//              null, null, null);
//          StringReader reader = new StringReader((QString) event
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
//      QString newList = perspBuffer.toString().trim();
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
  perspectives.removeAll(desc);
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
//  QString customPerspectives = store .getString(
//      IPreferenceConstants.PERSPECTIVES);
//  QString[] perspectivesList = StringConverter.asArray(customPerspectives);
//
//  for (int i = 0; i < perspectivesList.length; i++)
//  {
//    try
//    {
//      QString xmlString = store.getString(perspectivesList[i] + PERSP);
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
//      QString id = newPersp.getId();
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

void PerspectiveRegistry::UnableToLoadPerspective(const QString& status)
{
  QString msg = "Unable to load perspective";
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
//  QString str = PrefUtil.getAPIPreferenceStore().getString(
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
