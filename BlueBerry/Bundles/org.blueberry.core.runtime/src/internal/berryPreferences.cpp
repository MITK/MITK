#include "berryPreferences.h"
#include "berryAbstractPreferencesStorage.h"
#include "Poco/ScopedLock.h"
#include <algorithm>

using namespace std;

namespace berry 
{

  Preferences::Preferences(const PropertyMap& _Properties
    , const std::string& _Name
    , Preferences* _Parent
    , AbstractPreferencesStorage* _Storage) 
      : m_Properties(_Properties)
      , m_Name(_Name)
      , m_Parent(_Parent)
      , m_Removed(false)
      , m_Storage(_Storage)
  {
    // root node if parent is 0
    if (_Parent != 0)
    {
      m_Path = _Parent->AbsolutePath() + (_Parent->AbsolutePath() == "/" ? "": "/") + _Name;
      // save as child in parent
      _Parent->m_Children.push_back(Preferences::Pointer(this));
      m_Root = _Parent->m_Root;
    }
    else
    {
      // root path
      m_Path = "/";
      m_Root = this;
    }
    
  }

  bool Preferences::Has( string key ) const
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    return (m_Properties.find(key) != m_Properties.end());
  }

  bool Preferences::IsDirty() const
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);

    bool dirty = m_Dirty;
    for (ChildrenList::const_iterator it = m_Children.begin()
      ; it != m_Children.end(); ++it)
    {      
      // break condition: if one node is dirty the whole tree is dirty
      if(dirty)
        break;
      else
        dirty = (*it)->IsDirty();
    }

    return dirty;
  }

  string Preferences::ToString() const
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    ostringstream s;
    s << "Preferences[" << m_Path << "]";
    return s.str();
  }

  bool Preferences::Equals(const Preferences* rhs)  const
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    if(rhs == 0)
      return false;

    return (this->m_Path == rhs->m_Path);
  }

  Preferences::PropertyMap Preferences::GetProperties() const
  {
    return m_Properties;
  }

  Preferences::ChildrenList Preferences::GetChildren() const
  {
    return m_Children;
  }

  string Preferences::AbsolutePath()  const
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    return m_Path;
  }

  vector<string> Preferences::ChildrenNames() const throw(Poco::Exception, BackingStoreException)
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    vector<string> names;
    for (ChildrenList::const_iterator it = m_Children.begin()
      ; it != m_Children.end(); ++it)
    {
      names.push_back((*it)->Name());
    }
    return names;
  }


  AbstractPreferencesStorage* Preferences::GetStorage() const
  {
    return m_Storage;
  }

  void Preferences::Clear() throw(Poco::Exception, BackingStoreException)
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    m_Properties.clear();
    this->SetDirty(true);
  }

  void Preferences::Flush() throw(Poco::Exception, BackingStoreException)  
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    // ensure that this is the parent

    m_Storage->Flush(this);
    // if something is written, make the tree undirty
    this->SetDirty(false);
  }

  string Preferences::Get(string key, string def) const 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    return this->Has(key)? m_Properties.find(key)->second: def;
  }

  bool Preferences::GetBool(string key, bool def) const 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    return this->Has(key)? (m_Properties.find(key)->second == "true"? true: false): def;
  }

  string Preferences::GetByteArray(string key, string def) const 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    return this->Has(key)? Base64::decode(m_Properties.find(key)->second): def;
  }

  double Preferences::GetDouble(string key, double def) const 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    return this->Has(key)? atof(m_Properties.find(key)->second.c_str()): def;
  }

  float Preferences::GetFloat(string key, float def) const 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    return this->Has(key)? static_cast<float>(atof(m_Properties.find(key)->second.c_str())): def;
  }

  int Preferences::GetInt(string key, int def) const
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    return this->Has(key)? atoi(m_Properties.find(key)->second.c_str()): def;
  }

  long Preferences::GetLong(string key, long def) const 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    return this->Has(key)? atol(m_Properties.find(key)->second.c_str()): def;
  }

  vector<string> Preferences::Keys() const throw(Poco::Exception, BackingStoreException)
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();

    vector<string> keys;
    for (PropertyMap::const_iterator it = m_Properties.begin()
      ; it != m_Properties.end(); it++)
    {
      keys.push_back(it->first);
    }
    return keys;
  }

  string Preferences::Name() const 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    return m_Name;
  }

  IPreferences::Pointer Preferences::Node(string pathName)  
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    AssertPath(pathName);

    IPreferences::Pointer node;

    // self reference
    if(pathName == "")
      return IPreferences::Pointer(this);
    // absolute path
    else if(pathName[0] == '/')
    {
      pathName = pathName.substr(1);
      // call root with this relative path
      return m_Root->Node(pathName);
    }
    // relative path
    else
    {
      // check if pathName contains anymore names
      string name = pathName;

      // create new child nodes as long as there are names in the path
      string::size_type pos = pathName.find("/");
      // cut from the beginning
      if(pos != string::npos)
      {
        name = pathName.substr(0, pos);
        pathName = pathName.substr(pos+1);
      }

      // now check if node exists->if not: make new
      for (ChildrenList::iterator it = m_Children.begin()
        ; it != m_Children.end(); it++)
      {
        // node found
        if((*it)->Name() == name && (*it)->IsRemoved() == false)
        {
          node = IPreferences::Pointer((*it).GetPointer());
          break;
        }
   
      }

      // node not found create new one
      if(node.IsNull())
      {
        // the new node automatically pushes itself into the children array of *this*
        Preferences::Pointer newNode(new Preferences(PropertyMap(), name, this, m_Storage));
        node = newNode.GetPointer();
        // this branch is dirty now -> prefs must be rewritten persistently
        this->SetDirty(true);
      
      }

      // call Node() again if there are any names left on the path
      if(pos != string::npos)
        node = node->Node(pathName);
    }

    return node;
  }

  bool Preferences::NodeExists(string pathName) const throw(Poco::Exception, BackingStoreException)
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    AssertPath(pathName);

    bool nodeExists = false;

    // absolute path
    if(pathName[0] == '/')
    {
      pathName = pathName.substr(1);
      // call root with this relative path
      return m_Root->NodeExists(pathName);
    }
    // relative path
    else
    {
      // check if pathName contains anymore names
      string name = pathName;

      // create new child nodes as long as there are names in the path
      string::size_type pos = pathName.find_first_of("/");
      // cut from the beginning
      if(pos != string::npos)
      {
        name = pathName.substr(0, pos);
        pathName = pathName.substr(pos+1);
      }

      // now check if node exists->if not: make new
      for (ChildrenList::const_iterator it = m_Children.begin()
        ; it != m_Children.end(); it++)
      {
        // node found
        if((*it)->Name() == name)
        {
          // call recursively if more names on the path exist
          if(pos != string::npos)
            nodeExists = (*it)->NodeExists(pathName);
          else 
            nodeExists = true;
          break;
        }
      }
    }

    return nodeExists;
  }

  void Preferences::Put(string key, string value) 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();

    m_Properties[key] = value;
    this->SetDirty(true);
  }

  void Preferences::PutByteArray(string key, string value) 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    this->Put(key, Base64::encode(value));
  }

  void Preferences::PutBool(string key, bool value)  
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    this->Put(key, value ? "true" : "false");
  }

  void Preferences::PutDouble(string key, double value)
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    this->Put(key, Preferences::ToString(value));
  }

  void Preferences::PutFloat(string key, float value)
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    this->Put(key, Preferences::ToString(value));
  }

  void Preferences::PutInt(string key, int value)   
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    this->Put(key, Preferences::ToString(value));
  }

  void Preferences::PutLong(string key, long value)
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    this->Put(key, Preferences::ToString(value));
  }

  void Preferences::Remove(string key)  
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    PropertyMap::iterator it = m_Properties.find(key);
    if(it != m_Properties.end())
      m_Properties.erase(it);
  }

  void Preferences::RemoveNode() throw(Poco::Exception, BackingStoreException)
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    this->SetRemoved(true);
    m_Parent->m_Children.erase(std::find(m_Parent->m_Children.begin(), m_Parent->m_Children.end(),
      Preferences::Pointer(this)));
  }

  void Preferences::Sync() throw(Poco::Exception, BackingStoreException) 
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    this->Flush();
  }

  void Preferences::AssertValid() const
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    if(m_Removed) 
    {
      ostringstream s; s << "no node at '" << m_Path << "'";
      throw Poco::IllegalStateException(s.str());
    }
  }

  void Preferences::AssertPath(string pathName) 
  {
    if(pathName.find("//") != string::npos) 
    {
      ostringstream s; s << "Illegal // in m_Path m_Name '" << pathName << "'";
      throw invalid_argument(s.str());
    }
    string::size_type strLength = pathName.length();
    if(pathName.length() > 1 && pathName[strLength-1] == '/')
    {
      ostringstream s; s << "Trailing / in m_Path m_Name '" << pathName << "'";
      throw invalid_argument(s.str());
    }
  }

  IPreferences::Pointer Preferences::Parent() const
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    AssertValid();
    return IPreferences::Pointer(m_Parent);
  }

  void Preferences::SetDirty( bool _Dirty )
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    m_Dirty = _Dirty;
    if(_Dirty)
      this->OnChanged.Send(this);
/*
    for (ChildrenList::iterator it = m_Children.begin()
    ; it != m_Children.end(); ++it)
    {      
      (*it)->SetDirty(_Dirty);
    }
*/

  }

  void Preferences::SetRemoved( bool _Removed )
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    m_Removed = _Removed;

    for (ChildrenList::iterator it = m_Children.begin()
      ; it != m_Children.end(); ++it)
    {      
      (*it)->SetRemoved(_Removed);
    }

  }

/*
  Preferences::ChildrenList& Preferences::GetChildren() const
  {
    return m_Children;
  }*/


  bool Preferences::IsRemoved() const
  {
    Poco::ScopedLock<Poco::Mutex> scopedMutex(m_Mutex);
    return m_Removed;
  }

  Preferences::~Preferences()
  {
  }
} 

namespace Base64 
{
  string encode(const string &sString)
  {
    static const string base64Table(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
      );
    static const char fillChar = '=';
    string::size_type   length = sString.length();
    string              result;

    // Allocate memory for the converted string
    result.reserve(length * 8 / 6 + 1);

    for(string::size_type nPos = 0; nPos < length; nPos++) {
      char code;

      // Encode the first 6 bits
      code = (sString[nPos] >> 2) & 0x3f;
      result.append(1, base64Table[code]);

      // Encode the remaining 2 bits with the next 4 bits (if present)
      code = (sString[nPos] << 4) & 0x3f;
      if(++nPos < length)
        code |= (sString[nPos] >> 4) & 0x0f;
      result.append(1, base64Table[code]);

      if(nPos < length) {
        code = (sString[nPos] << 2) & 0x3f;
        if(++nPos < length)
          code |= (sString[nPos] >> 6) & 0x03;

        result.append(1, base64Table[code]);
      } else {
        ++nPos;
        result.append(1, fillChar);
      }

      if(nPos < length) {
        code = sString[nPos] & 0x3f;
        result.append(1, base64Table[code]);
      } else {
        result.append(1, fillChar);
      }
    }

    return result;
  }

  string decode(const string &sString)
  {
    static const string::size_type np = string::npos;

    static const string::size_type DecodeTable[] = 
    {
      // 0   1   2   3   4   5   6   7   8   9 
      np, np, np, np, np, np, np, np, np, np,  //   0 -   9
      np, np, np, np, np, np, np, np, np, np,  //  10 -  19
      np, np, np, np, np, np, np, np, np, np,  //  20 -  29
      np, np, np, np, np, np, np, np, np, np,  //  30 -  39
      np, np, np, 62, np, np, np, 63, 52, 53,  //  40 -  49
      54, 55, 56, 57, 58, 59, 60, 61, np, np,  //  50 -  59
      np, np, np, np, np,  0,  1,  2,  3,  4,  //  60 -  69
      5,  6,  7,  8,  9, 10, 11, 12, 13, 14,  //  70 -  79
      15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  //  80 -  89
      25, np, np, np, np, np, np, 26, 27, 28,  //  90 -  99
      29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  // 100 - 109
      39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  // 110 - 119
      49, 50, 51, np, np, np, np, np, np, np,  // 120 - 129
      np, np, np, np, np, np, np, np, np, np,  // 130 - 139
      np, np, np, np, np, np, np, np, np, np,  // 140 - 149
      np, np, np, np, np, np, np, np, np, np,  // 150 - 159
      np, np, np, np, np, np, np, np, np, np,  // 160 - 169
      np, np, np, np, np, np, np, np, np, np,  // 170 - 179
      np, np, np, np, np, np, np, np, np, np,  // 180 - 189
      np, np, np, np, np, np, np, np, np, np,  // 190 - 199
      np, np, np, np, np, np, np, np, np, np,  // 200 - 209
      np, np, np, np, np, np, np, np, np, np,  // 210 - 219
      np, np, np, np, np, np, np, np, np, np,  // 220 - 229
      np, np, np, np, np, np, np, np, np, np,  // 230 - 239
      np, np, np, np, np, np, np, np, np, np,  // 240 - 249
      np, np, np, np, np, np                   // 250 - 256
    };
    static const char fillChar = '=';

    string::size_type length = sString.length();
    string            result;

    result.reserve(length);

    for(string::size_type nPos = 0; nPos < length; nPos++) {
      unsigned char c, c1;

      c = (char) DecodeTable[(unsigned char)sString[nPos]];
      nPos++;
      c1 = (char) DecodeTable[(unsigned char)sString[nPos]];
      c = (c << 2) | ((c1 >> 4) & 0x3);
      result.append(1, c);

      if(++nPos < length) {
        c = sString[nPos];
        if(fillChar == c)
          break;

        c = (char) DecodeTable[(unsigned char)sString[nPos]];
        c1 = ((c1 << 4) & 0xf0) | ((c >> 2) & 0xf);
        result.append(1, c1);
      }

      if(++nPos < length) {
        c1 = sString[nPos];
        if(fillChar == c1)
          break;

        c1 = (char) DecodeTable[(unsigned char)sString[nPos]];
        c = ((c << 6) & 0xc0) | c1;
        result.append(1, c);
      }
    }

    return result;
  }

}

ostream& operator<<( ostream& os,const berry::Preferences& m )
{
  os << m.ToString();
  return os;
}

ostream& operator<<( ostream& os,const berry::Preferences* m )
{
  os << m->ToString();
  return os;
}
