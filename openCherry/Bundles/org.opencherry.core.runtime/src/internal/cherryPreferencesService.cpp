#include "cherryPreferencesService.h"
#include "cherryXMLPreferencesStorage.h"

#include "Poco/ScopedLock.h"
#include "Poco/DirectoryIterator.h"
#include "cherryPlatform.h"

using namespace std;

bool cherry::PreferencesService::IsA( const std::type_info& type ) const
{
  std::string name(GetType().name());
  return name == type.name() || Service::IsA(type);
}

const std::type_info& cherry::PreferencesService::GetType() const
{
  return typeid(cherry::IPreferencesService);
}

std::string cherry::PreferencesService::GetDefaultPreferencesDirPath()
{
  string _PreferencesDirPath;
  _PreferencesDirPath = Platform::GetUserPath().getFileName() + ".BlueBerryPrefs";
  return _PreferencesDirPath;
}


std::string cherry::PreferencesService::GetDefaultPreferencesFileName()
{
  return "prefs.xml";
}

cherry::PreferencesService::PreferencesService(string _PreferencesDir)
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

cherry::PreferencesService::~PreferencesService()
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

cherry::IPreferences::Pointer cherry::PreferencesService::GetSystemPreferences()
{
  Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
  // sys prefs are indicated by an empty user string 
  return this->GetUserPreferences("");
}

cherry::IPreferences::Pointer cherry::PreferencesService::GetUserPreferences( std::string name )
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

std::vector<std::string> cherry::PreferencesService::GetUsers() const
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

void cherry::PreferencesService::ImportPreferences( Poco::File f, std::string name )
{
  map<string, AbstractPreferencesStorage::Pointer>::iterator it
    = m_PreferencesStorages.find(name); 

  if(it->second == 0)
  {
    this->GetUserPreferences(name);
  }

  Poco::File defaultFile = it->second->GetFile();
  XMLPreferencesStorage::Pointer storage(new XMLPreferencesStorage(f));
  m_PreferencesStorages[name] = storage;
  storage->SetFile(defaultFile);
}

void cherry::PreferencesService::ExportPreferences( Poco::File f, std::string name )
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