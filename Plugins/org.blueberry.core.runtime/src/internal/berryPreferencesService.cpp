/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "berryPreferencesService.h"
#include "berryXMLPreferencesStorage.h"
#include "berryIPreferences.h"

#include "berryPlatform.h"

#include <QDir>
#include <QDateTime>


QString berry::PreferencesService::GetDefaultPreferencesDirPath()
{
  return Platform::GetUserPath().absolutePath() + "/.BlueBerryPrefs";
}

QString berry::PreferencesService::GetDefaultPreferencesFileName()
{
  return "prefs.xml";
}

berry::PreferencesService::PreferencesService(const QString& _PreferencesDir)
: m_PreferencesDir(_PreferencesDir)
{
  if(m_PreferencesDir.isEmpty())
    m_PreferencesDir = GetDefaultPreferencesDirPath();

  QDir prefDir(m_PreferencesDir);
  if(!prefDir.exists())
    QDir().mkpath(m_PreferencesDir);

  QString defaultName = GetDefaultPreferencesFileName();
  QStringList prefFiles = prefDir.entryList(QStringList(QString("*") + defaultName),
                                            QDir::Files | QDir::Readable | QDir::Writable);
  foreach(QString prefFile, prefFiles)
  {
    int pos = prefFile.lastIndexOf(defaultName);
    QString userName = prefFile.left(pos);
    // set the storage to 0 (will be loaded later)
    m_PreferencesStorages[userName] = AbstractPreferencesStorage::Pointer(nullptr);
  }
}

berry::PreferencesService::~PreferencesService()
{
  this->ShutDown();
}

berry::IPreferences::Pointer berry::PreferencesService::GetSystemPreferences()
{
  QMutexLocker scopedMutex(&m_Mutex);
  // sys prefs are indicated by an empty user QString
  return this->GetUserPreferences_unlocked("");
}

berry::IPreferences::Pointer berry::PreferencesService::GetUserPreferences(const QString& name)
{
  QMutexLocker scopedMutex(&m_Mutex);
  return this->GetUserPreferences_unlocked(name);
}

berry::IPreferences::Pointer berry::PreferencesService::GetUserPreferences_unlocked(const QString& name)
{
  IPreferences::Pointer userPrefs(nullptr);

  QHash<QString, AbstractPreferencesStorage::Pointer>::const_iterator it
      = m_PreferencesStorages.find(name);

  // does not exist or is not loaded yet
  if(it == m_PreferencesStorages.end() || it.value().IsNull())
  {
    QString path = m_PreferencesDir;

    if(name.isEmpty())
      path = path +  '/' + GetDefaultPreferencesFileName();
    else
      path = path + '/' + name + GetDefaultPreferencesFileName();

    XMLPreferencesStorage::Pointer storage(new XMLPreferencesStorage(path));
    m_PreferencesStorages[name] = storage;
  }

  userPrefs = m_PreferencesStorages[name]->GetRoot();

  return userPrefs;
}

QStringList berry::PreferencesService::GetUsers() const
{
  QMutexLocker scopedMutex(&m_Mutex);
  QStringList users;

  for (QHash<QString, AbstractPreferencesStorage::Pointer>::const_iterator it = m_PreferencesStorages.begin();
       it != m_PreferencesStorages.end(); ++it)
  {
    users.push_back(it.key());
  }

  return users;
}

void berry::PreferencesService::ImportPreferences(const QString& f, const QString& name)
{
  QHash<QString, AbstractPreferencesStorage::Pointer>::const_iterator it
      = m_PreferencesStorages.find(name);

  if(it == m_PreferencesStorages.end() || it.value() == 0)
  {
    this->GetUserPreferences(name);
  }

  //Poco::File defaultFile = it->second->GetFile();
  XMLPreferencesStorage::Pointer storage(new XMLPreferencesStorage(f));

  IPreferences::Pointer rootOfImportedPrefs = storage->GetRoot();
  IPreferences::Pointer rootOfOldPrefs = m_PreferencesStorages[name]->GetRoot();

  // make backup of old
  QString exportFilePath = QDateTime::currentDateTime().toString();
  exportFilePath = GetDefaultPreferencesDirPath() + '/' + exportFilePath + "prefs.xml";
  this->ExportPreferences(exportFilePath, name);

  if(rootOfImportedPrefs.IsNotNull())
  {
    this->ImportNode(rootOfImportedPrefs, rootOfOldPrefs);
  }

}

void berry::PreferencesService::ShutDown()
{
  // flush all preferences
  for (QHash<QString, AbstractPreferencesStorage::Pointer>::const_iterator it = m_PreferencesStorages.begin();
       it != m_PreferencesStorages.end(); ++it)
  {
    // the preferences storage may be 0 if the corresponding file was never loaded
    if(it.value() != 0)
      it.value()->GetRoot()->Flush();
  }
}

void berry::PreferencesService::ImportNode(const IPreferences::Pointer& nodeToImport,
                                           const IPreferences::Pointer& rootOfOldPrefs)
{
  //# overwrite properties
  IPreferences::Pointer oldNode
    = rootOfOldPrefs->Node(nodeToImport->AbsolutePath()); // get corresponding node in "old" tree

  QStringList keys = nodeToImport->Keys(); // get all keys for properties
  foreach(QString key, keys)
  {
    oldNode->Put(key, nodeToImport->Get(key, "")); // set property in old node to the value of the imported.
                                                   // properties not existing in imported are left untouched
  }

  // do it for all children
  QStringList childrenNames = nodeToImport->ChildrenNames();
  foreach (QString childName, childrenNames)
  {
    // with node->Node(<childName>) you get the child node with the name <childName>
    this->ImportNode(nodeToImport->Node(childName), rootOfOldPrefs);
  }
}

void berry::PreferencesService::ExportPreferences(const QString& f, const QString& name )
{
  QHash<QString, AbstractPreferencesStorage::Pointer>::const_iterator it
      = m_PreferencesStorages.find(name);

  if(it.value() == 0)
  {
    this->GetUserPreferences(name);
  }
  AbstractPreferencesStorage::Pointer storage = it.value();
  QString temp = storage->GetFile();
  storage->SetFile(f);
  storage->GetRoot()->Flush();
  storage->SetFile(temp);
}
