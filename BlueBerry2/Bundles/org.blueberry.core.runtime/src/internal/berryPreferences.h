#ifndef BERRYPREFERENCES_H_
#define BERRYPREFERENCES_H_

#include "../berryRuntimeDll.h"
#include "berryIBerryPreferences.h"

#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include "Poco/Mutex.h"

namespace berry 
{
  class AbstractPreferencesStorage;
  ///
  /// Implementation of the OSGI Preferences Interface.
  /// Wraps a DOMNode.
  ///  
  class BERRY_RUNTIME Preferences: public IBerryPreferences 
  {

  public:
    ///
    /// For use with berry::SmartPtr
    ///
    berryObjectMacro(berry::Preferences)
    ///
    /// Maps a string key to a string value 
    ///    
    typedef std::map<std::string, std::string> PropertyMap;
    ///
    /// The list of Child nodes
    ///    
    typedef std::vector<Preferences::Pointer> ChildrenList;

    ///
    /// Constructs a new preference node.
    /// \param _Properties the key->value pairs of this preference node
    /// \param _Path the absolute path to this node, e.g. "/general/editors/font"
    /// \param _Name the name of this node, e.g. "font"
    /// \param _FileName the absolute path to the file in which this preferences tree will be saved to
    /// \param _Parent the parent node or 0 if this is the root
    /// \param _Root the root of this preference tree
    ///    
    Preferences(const PropertyMap& _Properties
      , const std::string& _Name
      , Preferences* _Parent
      , AbstractPreferencesStorage* _Storage);

    ///
    /// Nothing to do here
    ///    
    virtual ~Preferences();
    
    ///
    /// Prints out the absolute path of the preference node.
    ///    
    std::string ToString() const;
    ///
    /// Returns if this node and his silblings have to be rewritten persistently
    ///    
    bool IsDirty() const; 
    ///
    /// Returns if this node is removed
    ///    
    bool IsRemoved() const; 
    ///
    /// Returns if this node has property with a specific key
    ///    
    bool Has(std::string key) const;    
    ///
    /// Returns true if the absolute paths are the same
    ///    
    bool Equals(const Preferences* rhs) const;
    ///
    /// Returns all Properties as map.
    ///
    PropertyMap GetProperties() const;
    ///
    /// Returns a reference to the children list in order to add or remove nodes.
    /// *ATTENTION*: Should only be used
    /// when constructing the preferences tree from a persistent location. Normally, one would 
    /// only use the IPreferences methods
    ///
    ChildrenList GetChildren() const;

    //# Begin of IPreferences methods

    ///
    /// \see IPreferences::AbsolutePath()
    ///    
    virtual std::string AbsolutePath() const;

    ///
    /// \see IPreferences::ChildrenNames()
    ///    
    virtual std::vector<std::string> ChildrenNames() const throw(Poco::Exception, BackingStoreException);

    ///
    /// \see IPreferences::ChildrenNames()
    ///    
    virtual AbstractPreferencesStorage* GetStorage() const;

    ///
    /// \see IPreferences::Clear()
    ///    
    virtual void Clear() throw(Poco::Exception, BackingStoreException);

    ///
    /// \see IPreferences::Flush()
    ///    
    virtual void Flush() throw(Poco::Exception, BackingStoreException);

    ///
    /// \see IPreferences::Get()
    ///    
    virtual std::string Get(std::string key, std::string def) const;

    ///
    /// \see IPreferences::GetBool()
    ///    
    virtual bool GetBool(std::string key, bool def) const;

    ///
    /// \see IPreferences::GetByteArray()
    ///    
    virtual std::string GetByteArray(std::string key, std::string def) const;

    ///
    /// \see IPreferences::GetDouble()
    ///    
    virtual double GetDouble(std::string key, double def) const;

    ///
    /// \see IPreferences::GetFloat()
    ///    
    virtual float GetFloat(std::string key, float def) const;

    ///
    /// \see IPreferences::GetInt()
    ///    
    virtual int GetInt(std::string key, int def) const;

    ///
    /// \see IPreferences::GetLong()
    ///    
    virtual long GetLong(std::string key, long def) const;

    ///
    /// \see IPreferences::Keys()
    ///    
    std::vector<std::string> Keys() const throw(Poco::Exception, BackingStoreException);

    ///
    /// \see IPreferences::Name()
    ///    
    virtual std::string Name() const;

    ///
    /// \see IPreferences::Node()
    ///    
    virtual IPreferences::Pointer Node(std::string pathName);

    ///
    /// \see IPreferences::NodeExists()
    ///    
    virtual bool NodeExists(std::string pathName) const throw(Poco::Exception, BackingStoreException);

    ///
    /// \see IPreferences::Parent()
    ///    
    virtual IPreferences::Pointer Parent() const;

    ///
    /// \see IPreferences::Put()
    ///    
    virtual void Put(std::string key, std::string value);

    ///
    /// \see IPreferences::PutByteArray()
    ///    
    virtual void PutByteArray(std::string key, std::string value);

    ///
    /// \see IPreferences::PutBool()
    ///    
    virtual void PutBool(std::string key, bool value);

    ///
    /// \see IPreferences::PutDouble()
    ///    
    virtual void PutDouble(std::string key, double value);

    ///
    /// \see IPreferences::Sync()
    ///    
    virtual void PutFloat(std::string key, float value);

    ///
    /// \see IPreferences::PutInt()
    ///    
    virtual void PutInt(std::string key, int value);

    ///
    /// \see IPreferences::PutLong()
    ///    
    virtual void PutLong(std::string key, long value);

    ///
    /// \see IPreferences::Remove()
    ///    
    virtual void Remove(std::string key);

    ///
    /// \see IPreferences::RemoveNode()
    ///    
    virtual void RemoveNode() throw(Poco::Exception, BackingStoreException);

    ///
    /// \see IPreferences::Sync()
    ///    
    virtual void Sync() throw(Poco::Exception, BackingStoreException);

    //# End of IPreferences methods

  protected:

    ///
    /// Checks if this node is about to be removed.
    /// \throws IllegalStateException
    ///
    void AssertValid() const;
    ///
    /// Checks a path value for validity.
    /// \throws invalid_argument
    ///
    static void AssertPath(std::string pathName);
    ///
    /// Converts any value to a string (using stream operator "<<")
    ///
    template <class T>
    static std::string ToString(const T& obj, int precision = 12 )
    {
      std::ostringstream s; s.precision(precision); s << obj; return s.str();
    }
    ///
    /// Sets the dirty flag recursively on all child nodes.
    ///
    void SetDirty(bool _Dirty);
    ///
    /// Sets the removed flag recursively on all child nodes.
    ///
    void SetRemoved(bool _Removed);

  protected:
    ///
    /// Holds all Key/Value Pairs.
    ///    
    std::map<std::string, std::string> m_Properties;
    ///
    /// Holds all child nodes (explicit ownership). 
    ///    
    std::vector<Preferences::Pointer> m_Children;
    ///
    /// Saves the absolute path of this node (calculated in the constructor)
    ///    
    std::string m_Path;
    ///
    /// Saves the name of this node (set when read from backend)
    ///    
    std::string m_Name;
    ///
    /// Saves the parent of this node 
    ///    
    Preferences* m_Parent;
    ///
    /// Saves the root of this tree
    ///    
    Preferences* m_Root;
    ///
    /// Saves if something changed on this branch.
    /// Meaning that you would have to rewrite it.
    ///    
    bool m_Dirty;
    ///
    /// Saves if this Node is removed (will not be saved to the backend).
    ///    
    bool m_Removed;
    ///
    /// A storage to call the flush method.
    ///
    AbstractPreferencesStorage* m_Storage;
    ///
    /// A mutex to avoid concurrency crashes. Mutable because we need to use Mutex::lock() in const functions
    ///
    mutable Poco::Mutex m_Mutex;
  };

}

namespace Base64 
{
  /// 
  /// Encode string to base64 (needed for writing byte arrays)
  ///
  std::string encode(const std::string &sString);
  ///
  /// Decode base64 to string (needed for reading byte arrays)
  ///
  std::string decode(const std::string &sString);
};

///
/// Uses Preferences::ToString to print node information 
///
std::ostream& operator<<(std::ostream& os,const berry::Preferences& m);

///
/// Uses Preferences::ToString to print node information 
///
std::ostream& operator<<(std::ostream& os,const berry::Preferences* m);

#endif /* BERRYPREFERENCES_H_ */
