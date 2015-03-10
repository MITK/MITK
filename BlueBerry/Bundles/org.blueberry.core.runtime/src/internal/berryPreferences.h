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
#ifndef BERRYPREFERENCES_H_
#define BERRYPREFERENCES_H_

#include <org_blueberry_core_runtime_Export.h>

#include "berryIBerryPreferences.h"

#include <QHash>
#include <QList>

namespace berry
{
  class AbstractPreferencesStorage;
  ///
  /// Implementation of the OSGI Preferences Interface.
  /// Wraps a DOMNode.
  ///
  class org_blueberry_core_runtime_EXPORT Preferences: public IBerryPreferences
  {

  public:
    ///
    /// For use with berry::SmartPtr
    ///
    berryObjectMacro(berry::Preferences);
    ///
    /// Maps a string key to a string value
    ///
    typedef QHash<QString, QString> PropertyMap;
    ///
    /// The list of Child nodes
    ///
    typedef QList<Preferences::Pointer> ChildrenList;

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
      , const QString& _Name
      , Preferences* _Parent
      , AbstractPreferencesStorage* _Storage);

    ///
    /// Nothing to do here
    ///
    virtual ~Preferences();

    ///
    /// Prints out the absolute path of the preference node.
    ///
    QString ToString() const;
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
    bool Has(const QString& key) const;
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
    virtual QString AbsolutePath() const;

    ///
    /// \see IPreferences::ChildrenNames()
    ///
    virtual QStringList ChildrenNames() const;

    ///
    /// \see IPreferences::ChildrenNames()
    ///
    virtual AbstractPreferencesStorage* GetStorage() const;

    ///
    /// \see IPreferences::Clear()
    ///
    virtual void Clear();

    ///
    /// \see IPreferences::Flush()
    ///
    virtual void Flush();

    ///
    /// \see IPreferences::Get()
    ///
    virtual QString Get(const QString& key, const QString& def) const;

    ///
    /// \see IPreferences::GetBool()
    ///
    virtual bool GetBool(const QString& key, bool def) const;

    ///
    /// \see IPreferences::GetByteArray()
    ///
    virtual QByteArray GetByteArray(const QString& key, const QByteArray& def) const;

    ///
    /// \see IPreferences::GetDouble()
    ///
    virtual double GetDouble(const QString& key, double def) const;

    ///
    /// \see IPreferences::GetFloat()
    ///
    virtual float GetFloat(const QString& key, float def) const;

    ///
    /// \see IPreferences::GetInt()
    ///
    virtual int GetInt(const QString& key, int def) const;

    ///
    /// \see IPreferences::GetLong()
    ///
    virtual long GetLong(const QString& key, long def) const;

    ///
    /// \see IPreferences::Keys()
    ///
    QStringList Keys() const;

    ///
    /// \see IPreferences::Name()
    ///
    virtual QString Name() const;

    ///
    /// \see IPreferences::Node()
    ///
    virtual IPreferences::Pointer Node(const QString& pathName);

    ///
    /// \see IPreferences::NodeExists()
    ///
    virtual bool NodeExists(const QString& pathName) const;

    ///
    /// \see IPreferences::Parent()
    ///
    virtual IPreferences::Pointer Parent() const;

    ///
    /// \see IPreferences::Put()
    ///
    virtual void Put(const QString& key, const QString& value);

    ///
    /// \see IPreferences::PutByteArray()
    ///
    virtual void PutByteArray(const QString& key, const QByteArray& value);

    ///
    /// \see IPreferences::PutBool()
    ///
    virtual void PutBool(const QString& key, bool value);

    ///
    /// \see IPreferences::PutDouble()
    ///
    virtual void PutDouble(const QString& key, double value);

    ///
    /// \see IPreferences::Sync()
    ///
    virtual void PutFloat(const QString& key, float value);

    ///
    /// \see IPreferences::PutInt()
    ///
    virtual void PutInt(const QString& key, int value);

    ///
    /// \see IPreferences::PutLong()
    ///
    virtual void PutLong(const QString& key, long value);

    ///
    /// \see IPreferences::Remove()
    ///
    virtual void Remove(const QString& key);

    ///
    /// \see IPreferences::RemoveNode()
    ///
    virtual void RemoveNode();

    ///
    /// \see IPreferences::Sync()
    ///
    virtual void Sync();

    //# End of IPreferences methods

  protected:

    ///
    /// Checks if this node is about to be removed.
    /// \throws IllegalStateException
    ///
    void AssertValid_unlocked() const;
    ///
    /// Checks a path value for validity.
    /// \throws invalid_argument
    ///
    static void AssertPath_unlocked(const QString& pathName);
//    ///
//    /// Converts any value to a string (using stream operator "<<")
//    ///
//    template <class T>
//    static QString ToString(const T& obj, int precision = 12 )
//    {
//      std::ostringstream s;
//      std::locale C("C");
//      s.imbue(C);
//      s.precision(precision); s << obj; return s.str();
//    }

    bool Has_unlocked(const QString& key) const;

    Preferences::Pointer Node_unlocked(const QString& pathName);

    ///
    /// Sets the dirty flag recursively on all child nodes.
    ///
    void SetDirty(bool _Dirty);
    void SetDirty_unlocked(bool _Dirty);
    ///
    /// Sets the removed flag recursively on all child nodes.
    ///
    void SetRemoved(bool _Removed);
    void SetRemoved_unlocked(bool _Removed);

  protected:
    ///
    /// Holds all Key/Value Pairs.
    ///
    QHash<QString, QString> m_Properties;
    ///
    /// Holds all child nodes (explicit ownership).
    ///
    QList<Preferences::Pointer> m_Children;
    ///
    /// Saves the absolute path of this node (calculated in the constructor)
    ///
    const QString m_Path;
    ///
    /// Saves the name of this node (set when read from backend)
    ///
    const QString m_Name;
    ///
    /// Saves the parent of this node
    ///
    Preferences* const m_Parent;
    ///
    /// Saves the root of this tree
    ///
    Preferences* const m_Root;
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
    AbstractPreferencesStorage* const m_Storage;
    ///
    /// A mutex to avoid concurrency crashes. Mutable because we need to use Mutex::lock() in const functions
    ///
    mutable QMutex m_Mutex;
  };

}

#endif /* BERRYPREFERENCES_H_ */
