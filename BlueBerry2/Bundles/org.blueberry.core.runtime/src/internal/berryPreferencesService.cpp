#include "berryPreferencesService.h"
#include "berryXMLPreferencesStorage.h"

#include "Poco/ScopedLock.h"
#include "Poco/DirectoryIterator.h"
#include "berryPlatform.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"

using namespace std;

bool berry::PreferencesService::IsA( const std::type_info& type ) const
{
  std::string name(GetType().name());
  return name == type.name() || Service::IsA(type);
}

const std::type_info& berry::PreferencesService::GetType() const
{
  return typeid(berry::IPreferencesService);
}

std::string berry::PreferencesService::GetDefaultPreferencesDirPath()
{
  string _PreferencesDirPath;
  _PreferencesDirPath = Platform::GetUserPath().toString() + ".BlueBerryPrefs";
  return _PreferencesDirPath;
}


std::string berry::PreferencesService::GetDefaultPreferencesFileName()
{
  return "prefs.xml";
}

berry::PreferencesService::PreferencesService(string _PreferencesDir)
: m_PreferencesDir(_PreferencesDir)
{
  if(m_PreferencesDir.empty())
    m_PreferencesDir = GetDefaultPreferencesDirPath();

  Poco::File prefDir(m_PreferencesDir);
  if(!prefDir.exists())
    prefDir.createDirectory();

  Poco::DirectoryIterator dirIt(prefDir);

  Poco::File f;

  while(dirIt.path().getFileName() != "") 
  {
    f = dirIt.path();

    if(f.isFile())
    {
      // check if this file is a preferences file
      string::size_type pos = dirIt.name().rfind(GetDefaultPreferencesFileName());
      if(pos != string::npos)
      {
        string userName = dirIt.name().substr(0, pos);
        // set the storage to 0 (will be loaded later)
        m_PreferencesStorages[userName] = AbstractPreferencesStorage::Pointer(0);
      }
    }
    ++dirIt;
  }
}

berry::PreferencesService::~PreferencesService()
{
  this->ShutDown();
}

berry::IPreferences::Pointer berry::PreferencesService::GetSystemPreferences()
{
  Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
  // sys prefs are indicated by an empty user string 
  return this->GetUserPreferences("");
}

berry::IPreferences::Pointer berry::PreferencesService::GetUserPreferences( std::string name )
{
  Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
  IPreferences::Pointer userPrefs(0);
  
  map<string, AbstractPreferencesStorage::Pointer>::iterator it
    = m_PreferencesStorages.find(name); 

  // does not exist or is not loaded yet
  if(it == m_PreferencesStorages.end() || it->second.IsNull())
  {
    std::string path = m_PreferencesDir;

    if(name.empty())
      path = path +  Poco::Path::separator() + GetDefaultPreferencesFileName();
    // 
    else
      path = path +  Poco::Path::separator() + name + GetDefaultPreferencesFileName();

    XMLPreferencesStorage::Pointer storage(new XMLPreferencesStorage(path));
    m_PreferencesStorages[name] = storage;
  }

  userPrefs = m_PreferencesStorages[name]->GetRoot();

  return userPrefs;
}

std::vector<std::string> berry::PreferencesService::GetUsers() const
{
  Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
  vector<string> users;

  for (map<string, AbstractPreferencesStorage::Pointer>::const_iterator it = m_PreferencesStorages.begin()
    ; it != m_PreferencesStorages.end(); ++it)
  {
    users.push_back(it->first);
  }

  return users;
}

void berry::PreferencesService::ImportPreferences( Poco::File f, std::string name )
{
  map<string, AbstractPreferencesStorage::Pointer>::iterator it
    = m_PreferencesStorages.find(name); 

  if(it->second == 0)
  {
    this->GetUserPreferences(name);
  }

  //Poco::File defaultFile = it->second->GetFile();
  XMLPreferencesStorage::Pointer storage(new XMLPreferencesStorage(f));

  IPreferences::Pointer rootOfImportedPrefs = storage->GetRoot();
  IPreferences::Pointer rootOfOldPrefs = m_PreferencesStorages[name]->GetRoot();

  // make backup of old
  std::string exportFilePath = Poco::DateTimeFormatter::format(Poco::DateTime(), "%Y.%m.%d-%H%M%S");
  exportFilePath = GetDefaultPreferencesDirPath() + Poco::Path::separator() + exportFilePath + "prefs.xml";
  Poco::File exportFile(exportFilePath);
  this->ExportPreferences(exportFile, name);

  if(rootOfImportedPrefs.IsNotNull())
  {
    this->ImportNode(rootOfImportedPrefs, rootOfOldPrefs);
  }

}

void berry::PreferencesService::ShutDown()
{
  // flush all preferences
  for (map<string, AbstractPreferencesStorage::Pointer>::const_iterator it = m_PreferencesStorages.begin()
    ; it != m_PreferencesStorages.end(); ++it)
  {
    // the preferences storage may be 0 if the corresponding file was never loaded
    if(it->second != 0)
      it->second->GetRoot()->Flush();
  }
}

void berry::PreferencesService::ImportNode( IPreferences::Pointer nodeToImport
                                            , IPreferences::Pointer rootOfOldPrefs )
{
  //# overwrite properties
  IPreferences::Pointer oldNode 
    = rootOfOldPrefs->Node(nodeToImport->AbsolutePath()); // get corresponding node in "old" tree

  std::vector<std::string> keys = nodeToImport->Keys(); // get all keys for properties
  for (vector<string>::const_iterator it = keys.begin()
    ; it != keys.end(); ++it)
  {
    oldNode->Put((*it), nodeToImport->Get((*it), ""));// set property in old node to the value of the imported.
                                                // properties not existing in imported are left untouched
     
  }

  // do it for all children
  vector<string> childrenNames = nodeToImport->ChildrenNames();
  for (vector<string>::const_iterator it = childrenNames.begin()
    ; it != childrenNames.end(); ++it)
  {
    // with node->Node(<childName>) you get the child node with the name <childName>
    this->ImportNode(nodeToImport->Node((*it)), rootOfOldPrefs);
  }
}

void berry::PreferencesService::ExportPreferences( Poco::File f, std::string name )
{
  map<string, AbstractPreferencesStorage::Pointer>::iterator it
    = m_PreferencesStorages.find(name); 

  if(it->second == 0)
  {
    this->GetUserPreferences(name);
  }
  Poco::File temp = it->second->GetFile();
  it->second->SetFile(f);
  it->second->GetRoot()->Flush();
  it->second->SetFile(temp);

}
