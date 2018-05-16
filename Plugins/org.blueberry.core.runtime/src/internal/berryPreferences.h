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

  /**
   * Implementation of the OSGI Preferences Interface.
   * Wraps a DOMNode.
   */
  class org_blueberry_core_runtime_EXPORT Preferences: public IBerryPreferences
  {

  public:

    berryObjectMacro(berry::Preferences)

    /**
     * Maps a string key to a string value
     */
    typedef QHash<QString, QString> PropertyMap;

    /**
     * The list of Child nodes
     */
    typedef QList<Preferences::Pointer> ChildrenList;

    /**
     * Constructs a new preference node.
     * \param _Properties the key->value pairs of this preference node
     * \param _Path the absolute path to this node, e.g. "/general/editors/font"
     * \param _Name the name of this node, e.g. "font"
     * \param _FileName the absolute path to the file in which this preferences tree will be saved to
     * \param _Parent the parent node or 0 if this is the root
     * \param _Root the root of this preference tree
     */
    Preferences(const PropertyMap& _Properties,
                const QString& _Name,
                Preferences* _Parent,
                AbstractPreferencesStorage* _Storage);

    ~Preferences() override;

    /**
     * Prints out the absolute path of the preference node.
     */
    QString ToString() const override;

    /**
     * Returns if this node and his silblings have to be rewritten persistently
     */
    bool IsDirty() const;

    /**
     * Returns if this node is removed
     */
    bool IsRemoved() const;

    /**
     * Returns if this node has property with a specific key
     */
    bool Has(const QString& key) const;

    /**
     * Returns true if the absolute paths are the same
     */
    bool Equals(const Preferences* rhs) const;

    /**
     * Returns all Properties as map.
     */
    PropertyMap GetProperties() const;

    /**
     * Returns a reference to the children list in order to add or remove nodes.
     * *ATTENTION*: Should only be used
     * when constructing the preferences tree from a persistent location. Normally, one would
     * only use the IPreferences methods
     */
    ChildrenList GetChildren() const;

    //# Begin of IPreferences methods

    /**
     * \see IPreferences::AbsolutePath()
     */
    QString AbsolutePath() const override;

    /**
     * \see IPreferences::ChildrenNames()
     */
    QStringList ChildrenNames() const override;

    /**
     * \see IPreferences::ChildrenNames()
     */
    virtual AbstractPreferencesStorage* GetStorage() const;

    /**
     * \see IPreferences::Clear()
     */
    void Clear() override;

    /**
     * \see IPreferences::Flush()
     */
    void Flush() override;

    /**
     * \see IPreferences::Get()
     */
    QString Get(const QString& key, const QString& def) const override;

    /**
     * \see IPreferences::GetBool()
     */
    bool GetBool(const QString& key, bool def) const override;

    /**
     * \see IPreferences::GetByteArray()
     */
    QByteArray GetByteArray(const QString& key, const QByteArray& def) const override;

    /**
     * \see IPreferences::GetDouble()
     */
    double GetDouble(const QString& key, double def) const override;

    /**
     * \see IPreferences::GetFloat()
     */
    float GetFloat(const QString& key, float def) const override;

    /**
     * \see IPreferences::GetInt()
     */
    int GetInt(const QString& key, int def) const override;

    /**
     * \see IPreferences::GetLong()
     */
    long GetLong(const QString& key, long def) const override;

    /**
     * \see IPreferences::Keys()
     */
    QStringList Keys() const override;

    /**
     * \see IPreferences::Name()
     */
    QString Name() const override;

    /**
     * \see IPreferences::Node()
     */
    IPreferences::Pointer Node(const QString& pathName) override;

    /**
     * \see IPreferences::NodeExists()
     */
    bool NodeExists(const QString& pathName) const override;

    /**
     * \see IPreferences::Parent()
     */
    IPreferences::Pointer Parent() const override;

    /**
     * \see IPreferences::Put()
     */
    void Put(const QString& key, const QString& value) override;

    /**
     * \see IPreferences::PutByteArray()
     */
    void PutByteArray(const QString& key, const QByteArray& value) override;

    /**
     * \see IPreferences::PutBool()
     */
    void PutBool(const QString& key, bool value) override;

    /**
     * \see IPreferences::PutDouble()
     */
    void PutDouble(const QString& key, double value) override;

    /**
     * \see IPreferences::Sync()
     */
    void PutFloat(const QString& key, float value) override;

    /**
     * \see IPreferences::PutInt()
     */
    void PutInt(const QString& key, int value) override;

    /**
     * \see IPreferences::PutLong()
     */
    void PutLong(const QString& key, long value) override;

    /**
     * \see IPreferences::Remove()
     */
    void Remove(const QString& key) override;

    /**
     * \see IPreferences::RemoveNode()
     */
    void RemoveNode() override;

    /**
     * \see IPreferences::Sync()
     */
    void Sync() override;

    /**
    * \see IPreferences::BlockSignals()
    */
    void BlockSignals(bool block) override;

    //# End of IPreferences methods

  protected:

    /**
     * Checks if this node is about to be removed.
     * \throws IllegalStateException
     */
    void AssertValid_unlocked() const;

    /**
     * Checks a path value for validity.
     * \throws invalid_argument
     */
    static void AssertPath_unlocked(const QString& pathName);

//    /**
//     * Converts any value to a string (using stream operator "<<")
//     */
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

    /**
     * Sets the dirty flag recursively on all child nodes.
     */
    void SetDirty(bool _Dirty);
    void SetDirty_unlocked(bool _Dirty);

    /**
     * Sets the removed flag recursively on all child nodes.
     */
    void SetRemoved(bool _Removed);
    void SetRemoved_unlocked(bool _Removed);

  protected:

    /**
     * Holds all Key/Value Pairs.
     */
    QHash<QString, QString> m_Properties;

    /**
     * Holds all child nodes (explicit ownership).
     */
    QList<Preferences::Pointer> m_Children;

    /**
     * Saves the absolute path of this node (calculated in the constructor)
     */
    const QString m_Path;

    /**
     * Saves the name of this node (set when read from backend)
     */
    const QString m_Name;

    /**
     * Saves the parent of this node
     */
    Preferences* const m_Parent;

    /**
     * Saves the root of this tree
     */
    Preferences* const m_Root;

    /**
     * Saves if something changed on this branch.
     * Meaning that you would have to rewrite it.
     */
    bool m_Dirty;

    /**
     * Saves if this Node is removed (will not be saved to the backend).
     */
    bool m_Removed;

    /**
     * A storage to call the flush method.
     */
    AbstractPreferencesStorage* const m_Storage;

    /**
     * A mutex to avoid concurrency crashes. Mutable because we need to use Mutex::lock() in const functions
     */
    mutable QMutex m_Mutex;

    /**
    * A flag to block berry messages in order to prevent callback functions to be called.
    */
    bool m_BlockSignal;
  };

}

#endif /* BERRYPREFERENCES_H_ */
