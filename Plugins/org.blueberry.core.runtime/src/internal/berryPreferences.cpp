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
#include "berryPreferences.h"
#include "berryAbstractPreferencesStorage.h"

#include <algorithm>

#include <QStringList>

namespace berry
{

  Preferences::Preferences(const PropertyMap& _Properties
    , const QString& _Name
    , Preferences* _Parent
    , AbstractPreferencesStorage* _Storage)
      : m_Properties(_Properties)
      , m_Path(_Parent ? _Parent->AbsolutePath() + (_Parent->AbsolutePath() == "/" ? "": "/") + _Name : "/")
      , m_Name(_Name)
      , m_Parent(_Parent)
      , m_Root(_Parent ? _Parent->m_Root : this)
      , m_Removed(false)
      , m_Storage(_Storage)
      , m_BlockSignal(false)
  {
    // root node if parent is 0
    if (_Parent != nullptr)
    {
      // save as child in parent
      _Parent->m_Children.push_back(Preferences::Pointer(this));
    }
  }

  bool Preferences::Has(const QString& key ) const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    return this->Has_unlocked(key);
  }

  bool Preferences::Has_unlocked(const QString& key ) const
  {
    return (m_Properties.find(key) != m_Properties.end());
  }

  bool Preferences::IsDirty() const
  {
    QMutexLocker scopedMutex(&m_Mutex);

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

  QString Preferences::ToString() const
  {
    return QString("Preferences[") + m_Path + "]";
  }

  bool Preferences::Equals(const Preferences* rhs)  const
  {
    if(rhs == nullptr)
      return false;

    return (this->m_Path == rhs->m_Path);
  }

  Preferences::PropertyMap Preferences::GetProperties() const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    return m_Properties;
  }

  Preferences::ChildrenList Preferences::GetChildren() const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    return m_Children;
  }

  QString Preferences::AbsolutePath() const
  {
    return m_Path;
  }

  QStringList Preferences::ChildrenNames() const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    QStringList names;
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

  void Preferences::Clear()
  {
    {
      QMutexLocker scopedMutex(&m_Mutex);
      AssertValid_unlocked();
      m_Properties.clear();
    }
    this->SetDirty(true);
  }

  void Preferences::Flush()
  {
    {
      QMutexLocker scopedMutex(&m_Mutex);
      AssertValid_unlocked();
    }

    m_Storage->Flush(this);

    // if something is written, make the tree undirty
    // there is a race condition here: after flushing, another thread
    // could modify this object before we can set dirty to false,
    // but we cannot hold a lock before flushing because the operation
    // will call other methods on this object, which would lead
    // to a recursive lock.
    this->SetDirty(false);
  }

  QString Preferences::Get(const QString& key, const QString& def) const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    return this->Has_unlocked(key) ? m_Properties[key] : def;
  }

  bool Preferences::GetBool(const QString& key, bool def) const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    return this->Has_unlocked(key) ? (m_Properties[key] == "true" ? true: false) : def;
  }

  QByteArray Preferences::GetByteArray(const QString& key, const QByteArray& def) const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    return this->Has_unlocked(key) ? QByteArray::fromBase64(m_Properties[key].toLatin1()) : def;
  }

  double Preferences::GetDouble(const QString& key, double def) const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    return this->Has_unlocked(key) ? m_Properties[key].toDouble() : def;
  }

  float Preferences::GetFloat(const QString& key, float def) const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    return this->Has_unlocked(key) ? m_Properties[key].toFloat() : def;
  }

  int Preferences::GetInt(const QString& key, int def) const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    return this->Has_unlocked(key) ? m_Properties[key].toInt() : def;
  }

  long Preferences::GetLong(const QString& key, long def) const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    return this->Has_unlocked(key) ? m_Properties[key].toLong() : def;
  }

  QStringList Preferences::Keys() const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();

    return m_Properties.keys();
  }

  QString Preferences::Name() const
  {
    return m_Name;
  }

  IPreferences::Pointer Preferences::Node(const QString& path)
  {
    QMutexLocker scopedMutex(&m_Mutex);
    return this->Node_unlocked(path);
  }

  Preferences::Pointer Preferences::Node_unlocked(const QString& path)
  {
    QString pathName = path;

    AssertValid_unlocked();
    AssertPath_unlocked(pathName);

    Preferences::Pointer node;

    // self reference
    if(pathName == "")
      return Preferences::Pointer(this);
    // absolute path
    else if(pathName[0] == '/')
    {
      pathName = pathName.mid(1);
      // call root with this relative path
      if (this == m_Root)
        return m_Root->Node_unlocked(pathName);
      else
        return m_Root->Node(pathName).Cast<Preferences>();
    }
    // relative path
    else
    {
      // check if pathName contains anymore names
      QString name = pathName;

      // create new child nodes as long as there are names in the path
      int pos = pathName.indexOf('/');
      // cut from the beginning
      if(pos != -1)
      {
        name = pathName.left(pos);
        pathName = pathName.mid(pos+1);
      }

      // now check if node exists->if not: make new
      for (ChildrenList::iterator it = m_Children.begin()
        ; it != m_Children.end(); it++)
      {
        // node found
        if((*it)->Name() == name && (*it)->IsRemoved() == false)
        {
          node = *it;
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
        this->SetDirty_unlocked(true);
      }

      // call Node() again if there are any names left on the path
      if(pos != -1)
      {
        if (this == node.GetPointer())
          node = node->Node_unlocked(pathName);
        else
          node = node->Node(pathName).Cast<Preferences>();
      }
    }

    return node;
  }

  bool Preferences::NodeExists(const QString& path) const
  {
    QString pathName = path;

    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    AssertPath_unlocked(pathName);

    bool nodeExists = false;

    // absolute path
    if(pathName[0] == '/')
    {
      pathName = pathName.mid(1);
      // call root with this relative path
      return m_Root->NodeExists(pathName);
    }
    // relative path
    else
    {
      // check if pathName contains anymore names
      QString name = pathName;

      // create new child nodes as long as there are names in the path
      int pos = pathName.indexOf("/");
      // cut from the beginning
      if(pos != -1)
      {
        name = pathName.left(pos);
        pathName = pathName.mid(pos+1);
      }

      // now check if node exists->if not: make new
      for (ChildrenList::const_iterator it = m_Children.begin()
        ; it != m_Children.end(); it++)
      {
        // node found
        if((*it)->Name() == name)
        {
          // call recursively if more names on the path exist
          if(pos != -1)
            nodeExists = (*it)->NodeExists(pathName);
          else
            nodeExists = true;
          break;
        }
      }
    }

    return nodeExists;
  }

  void Preferences::Put(const QString& key, const QString& value)
  {
    QString oldValue;
    {
      QMutexLocker scopedMutex(&m_Mutex);
      AssertValid_unlocked();
      oldValue = m_Properties[key];
      m_Properties[key] = value;
    }
    if (oldValue != value)
    {
      this->SetDirty(true);
      if (!m_BlockSignal)
      {
        this->OnPropertyChanged(ChangeEvent(this, key, oldValue, value));
      }
    }
  }

  void Preferences::PutByteArray(const QString& key, const QByteArray& value)
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    this->Put(key, value.toBase64().data());
  }

  void Preferences::PutBool(const QString& key, bool value)
  {
    this->Put(key, value ? "true" : "false");
  }

  void Preferences::PutDouble(const QString& key, double value)
  {
    this->Put(key, QString::number(value));
  }

  void Preferences::PutFloat(const QString& key, float value)
  {
    this->Put(key, QString::number(value));
  }

  void Preferences::PutInt(const QString& key, int value)
  {
    this->Put(key, QString::number(value));
  }

  void Preferences::PutLong(const QString& key, long value)
  {
    this->Put(key, QString::number(value));
  }

  void Preferences::Remove(const QString& key)
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    PropertyMap::iterator it = m_Properties.find(key);
    if(it != m_Properties.end())
      m_Properties.erase(it);
  }

  void Preferences::RemoveNode()
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    this->SetRemoved_unlocked(true);
    m_Parent->m_Children.erase(std::find(m_Parent->m_Children.begin(), m_Parent->m_Children.end(),
      Preferences::Pointer(this)));
  }

  void Preferences::Sync()
  {
    this->Flush();
  }

  void Preferences::BlockSignals(bool block)
  {
    m_BlockSignal = block;
  }

  void Preferences::AssertValid_unlocked() const
  {
    if(m_Removed)
    {
      throw ctkIllegalStateException(QString("no node at '") + m_Path + "'");
    }
  }

  void Preferences::AssertPath_unlocked(const QString& pathName)
  {
    if(pathName.indexOf("//") != -1)
    {
      throw ctkInvalidArgumentException(QString("Illegal // in m_Path m_Name '") + pathName + "'");
    }
    int strLength = pathName.size();
    if(strLength > 1 && pathName[strLength-1] == '/')
    {
      throw ctkInvalidArgumentException(QString("Trailing / in m_Path m_Name '") + pathName + "'");
    }
  }

  IPreferences::Pointer Preferences::Parent() const
  {
    QMutexLocker scopedMutex(&m_Mutex);
    AssertValid_unlocked();
    return IPreferences::Pointer(m_Parent);
  }

  void Preferences::SetDirty( bool _Dirty )
  {
    {
      QMutexLocker scopedMutex(&m_Mutex);
      m_Dirty = _Dirty;
    }
    if(_Dirty)
    {
      if (!m_BlockSignal)
      {
        this->OnChanged.Send(this);
      }
    }
  }

  void Preferences::SetDirty_unlocked( bool _Dirty )
  {
    m_Dirty = _Dirty;
    if(_Dirty)
      if (!m_BlockSignal)
      {
        this->OnChanged.Send(this);
      }
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
    QMutexLocker scopedMutex(&m_Mutex);
    this->SetRemoved_unlocked(_Removed);
  }

  void Preferences::SetRemoved_unlocked( bool _Removed )
  {
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
    QMutexLocker scopedMutex(&m_Mutex);
    return m_Removed;
  }

  Preferences::~Preferences()
  {
  }
}
