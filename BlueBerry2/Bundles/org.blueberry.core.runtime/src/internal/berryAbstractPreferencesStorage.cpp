#include "berryAbstractPreferencesStorage.h"

using namespace std;

namespace berry 
{

  AbstractPreferencesStorage::AbstractPreferencesStorage( const Poco::File& _File )
    : m_File(_File)
    , m_Root(0)
  {

  }

  AbstractPreferencesStorage::~AbstractPreferencesStorage()
  {

  }

  IPreferences::Pointer AbstractPreferencesStorage::GetRoot() const
  { 
    return m_Root;    
  }

  const Poco::File& AbstractPreferencesStorage::GetFile() const
  {
    return m_File;
  }

  void AbstractPreferencesStorage::SetFile( const Poco::File& f )
  {
    m_File = f;
  }
}
