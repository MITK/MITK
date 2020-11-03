/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryPerspectiveRegistry.h"

#include "berryWorkbench.h"
#include "berryWorkbenchPage.h"
#include "berryWorkbenchPlugin.h"
#include "berryPreferenceConstants.h"
#include "berryPerspective.h"
#include "berryPerspectiveRegistryReader.h"
#include "berryPlatformUI.h"
#include "handlers/berryClosePerspectiveHandler.h"
#include "berryIPreferencesService.h"
#include "berryIBerryPreferences.h"
#include "berryIExtension.h"
#include "berryIExtensionTracker.h"

namespace berry
{

const QString PerspectiveRegistry::EXT = "_persp.xml";
const QString PerspectiveRegistry::ID_DEF_PERSP = "PerspectiveRegistry.DEFAULT_PERSP";
const QString PerspectiveRegistry::PERSP = "_persp";
const char PerspectiveRegistry::SPACE_DELIMITER = ' ';

class PerspectiveRegistry::PreferenceChangeListener
{

  PerspectiveRegistry* m_Registry;

public:

  PreferenceChangeListener(PerspectiveRegistry* registry)
    : m_Registry(registry)
  {}

  void PropertyChange(const IBerryPreferences::ChangeEvent& event)
  {
    /*
     * To ensure that no custom perspective definitions are
     * deleted when preferences are imported, merge old and new
     * values
     */
    if (event.GetProperty().endsWith(PERSP))
    {
      /* A Perspective is being changed, merge */
      this->MergePerspectives(event);
    }
    else if (event.GetProperty() == PreferenceConstants::PERSPECTIVES)
    {
      /* The list of perpsectives is being changed, merge */
      UpdatePreferenceList(event.GetSource());
    }
  }

  void MergePerspectives(const IBerryPreferences::ChangeEvent& event)
  {
    IBerryPreferences* store = event.GetSource();
    if (event.GetNewValue().isNull() ||
        event.GetNewValue().isEmpty())
    {
      /*
       * Perpsective is being removed; if the user has deleted or
       * reverted a custom perspective, let the change pass
       * through. Otherwise, restore the custom perspective entry
       */

      // Find the matching descriptor in the registry
      QList<IPerspectiveDescriptor::Pointer> perspectiveList = m_Registry->GetPerspectives();
      for (int i = 0; i < perspectiveList.size(); i++)
      {
        QString id = perspectiveList[i]->GetId();
        if (event.GetProperty() == id + PERSP)
        { // found
          // descriptor
          // see if the perspective has been flagged for
          // reverting or deleting
          if (!m_Registry->perspToRemove.contains(id))
          { // restore
            store->Put(id + PERSP, event.GetOldValue());
          }
          else
          { // remove element from the list
            m_Registry->perspToRemove.removeAll(id);
          }
        }
      }
    }
    else if ((event.GetOldValue().isNull() || event.GetOldValue().isEmpty()))
    {

      /*
       * New perspective is being added, update the
       * perspectiveRegistry to contain the new custom perspective
       */

      QString id = event.GetProperty().left(event.GetProperty().lastIndexOf(PERSP));
      if (m_Registry->FindPerspectiveWithId(id).IsNull())
      {
        // perspective does not already exist in registry, add
        // it
        PerspectiveDescriptor::Pointer desc(new PerspectiveDescriptor(
                                              QString::null, QString::null, PerspectiveDescriptor::Pointer()));
        std::stringstream reader;
        std::string xmlStr = event.GetNewValue().toStdString();
        reader.str(xmlStr);
        try
        {
          XMLMemento::Pointer memento = XMLMemento::CreateReadRoot(reader);
          desc->RestoreState(memento);
          m_Registry->AddPerspective(desc);
        }
        catch (const WorkbenchException& e)
        {
          //m_Registry->UnableToLoadPerspective(e.getStatus());
          m_Registry->UnableToLoadPerspective(e.what());
        }
      }
    }
    /* If necessary, add to the list of perspectives */
    this->UpdatePreferenceList(store);
  }

  void UpdatePreferenceList(IBerryPreferences* store)
  {
    QList<IPerspectiveDescriptor::Pointer> perspectiveList = m_Registry->GetPerspectives();
    QStringList perspBuffer;
    for (int i = 0; i < perspectiveList.size(); i++)
    {
      PerspectiveDescriptor::Pointer desc = perspectiveList[i].Cast<PerspectiveDescriptor>();
      if (m_Registry->HasCustomDefinition(desc))
      {
        perspBuffer.push_back(desc->GetId());
      }
    }
    store->Put(PreferenceConstants::PERSPECTIVES, perspBuffer.join(QString(SPACE_DELIMITER)));
  }
};

PerspectiveRegistry::PerspectiveRegistry()
  : preferenceListener(new PreferenceChangeListener(this))
{
  IExtensionTracker* tracker = PlatformUI::GetWorkbench()->GetExtensionTracker();
  tracker->RegisterHandler(this, QString("org.blueberry.ui.perspectives"));

  berry::IBerryPreferences::Pointer prefs =
      WorkbenchPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences().Cast<IBerryPreferences>();
  prefs->OnPropertyChanged +=
      berry::MessageDelegate1<PreferenceChangeListener, const IBerryPreferences::ChangeEvent&>(
        preferenceListener.data(), &PreferenceChangeListener::PropertyChange);
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
    return IPerspectiveDescriptor::Pointer(nullptr);
  }
  if (this->FindPerspectiveWithLabel(label) != 0)
  {
    return IPerspectiveDescriptor::Pointer(nullptr);
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

  return IPerspectiveDescriptor::Pointer(nullptr);
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
  return IPerspectiveDescriptor::Pointer(nullptr);
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

void PerspectiveRegistry::SaveCustomPersp(PerspectiveDescriptor::Pointer desc,
                                          XMLMemento* memento)
{
  IPreferencesService* prefs = WorkbenchPlugin::GetDefault()->GetPreferencesService();

  // Save it to the preference store.
  std::stringstream ss;
  memento->Save(ss);
  prefs->GetSystemPreferences()->Put(desc->GetId() + PERSP, QString::fromStdString(ss.str()));
}

IMemento::Pointer PerspectiveRegistry::GetCustomPersp(const QString&  id)
{
  std::stringstream ss;

  IPreferencesService* prefs = WorkbenchPlugin::GetDefault()->GetPreferencesService();
  std::string xmlString = prefs->GetSystemPreferences()->Get(id + PERSP, QString::null).toStdString();
  if (!xmlString.empty())
  { // defined in store
    ss.str(xmlString);
  }
  XMLMemento::Pointer memento = XMLMemento::CreateReadRoot(ss);
  return memento;
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
//  PlatformUI::GetWorkbench()->GetExtensionTracker()->UnregisterHandler(this);
//  WorkbenchPlugin::GetDefault()->GetPreferencesService()->GetSystemPreferences()->RemovePropertyChangeListener(
//      preferenceListener);
}

void PerspectiveRegistry::DeleteCustomDefinition(PerspectiveDescriptor::Pointer  desc)
{
  // remove the entry from the preference store.
  IPreferencesService* store = WorkbenchPlugin::GetDefault()->GetPreferencesService();

  store->GetSystemPreferences()->Remove(desc->GetId() + PERSP);
}

bool PerspectiveRegistry::HasCustomDefinition(PerspectiveDescriptor::ConstPointer desc) const
{
  IPreferencesService* store = WorkbenchPlugin::GetDefault()->GetPreferencesService();
  return store->GetSystemPreferences()->Keys().contains(desc->GetId() + PERSP);
}

void PerspectiveRegistry::Add(PerspectiveDescriptor::Pointer desc)
{
  perspectives.push_back(desc);
  IConfigurationElement::Pointer element = desc->GetConfigElement();
  if (element.IsNotNull())
  {
    PlatformUI::GetWorkbench()->GetExtensionTracker()->RegisterObject(
          element->GetDeclaringExtension(), desc, IExtensionTracker::REF_WEAK);
  }
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
  QScopedPointer<std::istream> reader;

  /* Get the entries from the Preference store */
  IPreferencesService* store = WorkbenchPlugin::GetDefault()->GetPreferencesService();

  IPreferences::Pointer prefs = store->GetSystemPreferences();

  /* Get the space-delimited list of custom perspective ids */
  QString customPerspectives = prefs->Get(PreferenceConstants::PERSPECTIVES, QString::null);
  QStringList perspectivesList = customPerspectives.split(' ', QString::SkipEmptyParts);

  for (int i = 0; i < perspectivesList.size(); i++)
  {
    try
    {
      std::string xmlString = prefs->Get(perspectivesList[i] + PERSP, QString::null).toStdString();
      if (!xmlString.empty())
      {
        reader.reset(new std::stringstream(xmlString));
        //reader->exceptions(std::ios_base::failbit);
      }
      else
      {
        throw WorkbenchException(QString("Description of '%1' perspective could not be found.").arg(perspectivesList[i]));
      }

      // Restore the layout state.
      XMLMemento::Pointer memento = XMLMemento::CreateReadRoot(*reader);
      PerspectiveDescriptor::Pointer newPersp(new PerspectiveDescriptor(
                                                QString::null, QString::null,
                                                PerspectiveDescriptor::Pointer(nullptr)));
      newPersp->RestoreState(memento);
      QString id = newPersp->GetId();
      IPerspectiveDescriptor::Pointer oldPersp = FindPerspectiveWithId(id);
      if (oldPersp.IsNull())
      {
        Add(newPersp);
      }
    }
    catch (const std::ios_base::failure&)
    {
      UnableToLoadPerspective(QString::null);
    }
    catch (const WorkbenchException& e)
    {
      UnableToLoadPerspective(e.message());
    }
  }

//  // Get the entries from files, if any
//  // if -data @noDefault specified the state location may not be
//  // initialized
//  IPath path = WorkbenchPlugin.getDefault().getDataLocation();
//  if (path == null)
//  {
//    return;
//  }

//  File folder = path.toFile();

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

//          // save to the preference store
//          saveCustomPersp(newPersp, memento);

//          // delete the file
//          file.delete();

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
  PerspectiveRegistryReader reader(this);
  reader.ReadPerspectives(Platform::GetExtensionRegistry());
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

void PerspectiveRegistry::RemoveExtension(const IExtension::Pointer& /*source*/,
                                          const QList<Object::Pointer>& objects)
{
  for (int i = 0; i < objects.size(); i++)
  {
    if (PerspectiveDescriptor::Pointer desc = objects[i].Cast<PerspectiveDescriptor>())
    {
      // close the perspective in all windows
      QList<IWorkbenchWindow::Pointer> windows = PlatformUI::GetWorkbench()->GetWorkbenchWindows();
      for (int w = 0; w < windows.size(); ++w)
      {
        IWorkbenchWindow::Pointer window = windows[w];
        QList<IWorkbenchPage::Pointer> pages = window->GetPages();
        for (int p = 0; p < pages.size(); ++p)
        {
          WorkbenchPage::Pointer page = pages[p].Cast<WorkbenchPage>();
          ClosePerspectiveHandler::ClosePerspective(page, page->FindPerspective(desc));
        }
      }

      // ((Workbench)PlatformUI.getWorkbench()).getPerspectiveHistory().removeItem(desc);

      this->InternalDeletePerspective(desc);
    }
  }
}

void PerspectiveRegistry::AddExtension(IExtensionTracker* /*tracker*/,
                                       const IExtension::Pointer& addedExtension)
{
  QList<IConfigurationElement::Pointer> addedElements = addedExtension->GetConfigurationElements();
    for (int i = 0; i < addedElements.size(); i++)
    {
      PerspectiveRegistryReader reader(this);
      reader.ReadElement(addedElements[i]);
    }
  }

}
